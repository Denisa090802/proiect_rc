#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <set>
#include <queue>
#include "db_controller.h"
#define PORT 8452

std::string compress_peers(std::set<std::string> &peers)
{
  std::string s = "peers:";
  for (auto itr = peers.begin(); itr != peers.end(); itr++)
  {
    s += (*itr);
    s += ",";
  }
  return s;
}

bool check_if_client_is_active(std::string ip, std::set<std::string> &connected_peers)
{
  char *addr;
  struct sockaddr_in address;
  short int sock = -1;
  fd_set fdset;
  struct timeval tv;

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = inet_addr(ip.c_str());
  address.sin_port = htons(PORT);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  fcntl(sock, F_SETFL, O_NONBLOCK);

  connect(sock, (struct sockaddr *)&address, sizeof(address));

  FD_ZERO(&fdset);
  FD_SET(sock, &fdset);
  tv.tv_sec = 3;
  tv.tv_usec = 0;

  if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1)
  {
    int so_error;
    socklen_t len = sizeof so_error;
    getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
    if (so_error == 0)
    {
      connected_peers.insert(ip);
      char msg_buff[100];
      strcpy(msg_buff, compress_peers(connected_peers).c_str());
      fflush(stdout);
      write(sock, msg_buff, 100);
      fflush(stdout);
      close(sock);
      return true;
    }
  }
  close(sock);
  return false;
}

std::string getIpAddress()
{
  char ss[100];
  struct ifaddrs *ifAddrStruct = NULL;
  struct ifaddrs *ifa = NULL;
  void *tmpAddrPtr = NULL;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
  { // itereaza toate adresele din ifAddsStruct cu ajutorul var ifa
    if (ifa->ifa_addr->sa_family == AF_INET)
    {                                                                // verific daca adresa este de tip TCP
      tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr; // iau adresa ip
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN); // copiaza adresa din buffer in temp
      if (strcmp("eth0", ifa->ifa_name) == 0)                         // iau prima adresa de tip ethernet
      {
        strcpy(ss, addressBuffer);
      }
    }
  }
  if (ifAddrStruct != NULL)
    freeifaddrs(ifAddrStruct); // golim lista de ip uri
  return ss;
}

void setup_db()
{
  test_db_connection();
  initialize_database();
}

int broadcast_insert(char ip[50], char msg[100])
{
  if (ip == getIpAddress().c_str())
    return 0;
  std::cout << "Sending msg to " << ip << "'" << msg << "'" << endl;
  int sd;
  struct sockaddr_in server;

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(ip);
  /* portul de conectare */
  server.sin_port = htons(PORT);

  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  /* trimiterea mesajului la server */
  if (write(sd, msg, 100) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    return errno;
  }

  /* citirea raspunsului dat de server
     (apel blocant pina cind serverul raspunde); Atentie si la cum se face read- vezi cursul! */
  if (read(sd, msg, 100) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
    return errno;
  }
  /* afisam mesajul primit */
  printf("[peer-%s]Mesajul primit este: %s\n",ip, msg);

  /* inchidem conexiunea, am terminat */
  close(sd);
}

int listen_server(std::set<std::string> &connected_peers)
{
  int fd1 = open("broadcast.buffer",O_WRONLY);

  struct sockaddr_in server; // structura folosita de server
  struct sockaddr_in from;
  char msg[100];           // mesajul primit de la client
  char msgrasp[100] = " "; // mesaj de raspuns pentru client
  int sd;                  // descriptorul de socket

  /* crearea unui socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }

  /* pregatirea structurilor de date */
  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 5) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }

  /* servim in mod iterativ clientii... */
  while (1)
  {
    int client;
    socklen_t length = sizeof(from);

    // printf ("[server]Asteptam la portul %d...\n",PORT);
    // fflush (stdout);

    /* acceptam un client (stare blocanta pana la realizarea conexiunii) */
    client = accept(sd, (struct sockaddr *)&from, &length);

    /* eroare la acceptarea conexiunii de la un client */
    if (client < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }

    /* s-a realizat conexiunea, se astepta mesajul */
    bzero(msg, 100);
    // printf ("[server]Asteptam mesajul...\n");
    // fflush (stdout);

    /* citirea mesajului */
    if (read(client, msg, 100) <= 0)
    {
      perror("[server]Eroare la read() de la client.\n");
      close(client); /* inchidem conexiunea cu clientul */
      continue;      /* continuam sa ascultam */
    }

    // printf ("[server]Mesajul a fost receptionat...%s\n", msg);

    if (strncmp(msg, "peers:", 6) == 0)
    {
      char temp[1000];
      strcpy(temp, msg + 6);
      char *ip = strtok(temp, ",");
      while (ip)
      {
        if (strlen(ip) > 0)
        {
          std::string str = ip;
          printf("CONNECTED WITH: %s\n", str.c_str());
          fflush(stdout);
          connected_peers.insert(str);
        }
        ip = strtok(NULL, ",");
      }
    }
    else if (strncmp(msg, "insert:", 7) == 0 || strncmp(msg, "#insert:", 8) == 0)
    {
      bool disable_broadcast = msg[0] == '#';
      if (disable_broadcast)
      {
        strcpy(msg, msg + 1);
      }
      printf("INSERT MODE > \n");
      char nume[100], address[100];
      int id, year, month, day;
      char temp[1000];
      strcpy(temp, msg + 7);
      char *p = strtok(temp, ",");
      int contor = 1;
      id = atoi(p);
      while (p)
      {
        if (contor == 2)
        {
          strcpy(nume, p);
        }
        else if (contor == 3)
        {
          year = atoi(p);
        }
        else if (contor == 4)
        {
          month = atoi(p);
        }
        else if (contor == 5)
        {
          day = atoi(p);
        }
        else if (contor == 6)
        {
          strcpy(address, p);
        }

        p = strtok(NULL, ",");
        contor++;
      }
      if (check_id_db(id))
      {
        char msg2[1024];
        strcpy(msg2, "Elementul ");
        strcat(msg2, to_string(id).c_str());
        strcat(msg2, " a fost deja adaugat.");
        strcpy(msg, msg2);
      }
      else
      {
        insert_db(id, nume, year, month, day, address);
        if (disable_broadcast == false)
        {
          for (auto itr = connected_peers.begin(); itr != connected_peers.end(); itr++)
          {
            if((*itr) == getIpAddress()) continue;
            char msg2[1024];
            strcpy(msg2, "#");
            strcat(msg2, msg);
            strcat(msg2, "|");
            strcat(msg2, (*itr).c_str());

            if (write(fd1, msg2, sizeof(msg2)) < 0)
              perror("[copil]Err...write");
          }
        }

        strcpy(msg, "Elementul a fost inserat.");
      }
    }
    else if (strncmp(msg, "update:", 7) == 0 || strncmp(msg, "#update:", 7) == 0)
    {
      bool disable_broadcast = msg[0] == '#';
      if (disable_broadcast)
      {
        strcpy(msg, msg + 1);
      }
      printf("update MODE > \n");
      char nume[100], address[100];
      int id, year, month, day;
      char temp[1000];
      strcpy(temp, msg + 7);
      char *p = strtok(temp, ",");
      int contor = 1;
      id = atoi(p);
      while (p)
      {
        if (contor == 2)
        {
          strcpy(nume, p);
        }
        else if (contor == 3)
        {
          year = atoi(p);
        }
        else if (contor == 4)
        {
          month = atoi(p);
        }
        else if (contor == 5)
        {
          day = atoi(p);
        }
        else if (contor == 6)
        {
          strcpy(address, p);
        }

        p = strtok(NULL, ",");
        contor++;
      }
      if (!check_id_db(id))
      {
        char msg2[1024];
        strcpy(msg2, "Elementul ");
        strcat(msg2, to_string(id).c_str());
        strcat(msg2, " nu exista.");
        strcpy(msg, msg2);
      }
      else
      {
        printf("'%s' = '%s'\n", msg + 7, read_from_db(id).c_str());
        if (strcmp(msg + 7, read_from_db(id).c_str()) == 0)
        {
          strcpy(msg, "Elementul era deja actualizat.");
        }
        else
        {
          update_db(id, nume, year, month, day, address);
          if (disable_broadcast == false)
          {
            for (auto itr = connected_peers.begin(); itr != connected_peers.end(); itr++)
            {
              if((*itr) == getIpAddress()) continue;
              char msg2[1024];
              strcpy(msg2, "#");
              strcat(msg2, msg);
              strcat(msg2, "|");
              strcat(msg2, (*itr).c_str());

              if (write(fd1, msg2, sizeof(msg2)) < 0)
                perror("[copil]Err...write");
            }
          }

          strcpy(msg, "Elementul a fost actualizat.");
        }
      }
    }
    else if (strncmp(msg, "delete:", 7) == 0 || strncmp(msg, "#delete:", 7) == 0)
    {
      bool disable_broadcast = msg[0] == '#';
      if (disable_broadcast)
      {
        strcpy(msg, msg + 1);
      }
      int id = atoi(msg + 7);
      if (check_id_db(id))
      {
        delete_from_db(id);

        if (disable_broadcast == false)
        {
          for (auto itr = connected_peers.begin(); itr != connected_peers.end(); itr++)
          {
            if((*itr) == getIpAddress()) continue;
            char msg2[1024];
            strcpy(msg2, "#");
            strcat(msg2, msg);
            strcat(msg2, "|");
            strcat(msg2, (*itr).c_str());

            if (write(fd1, msg2, sizeof(msg2)) < 0)
              perror("[copil]Err...write");
          }
        }
      }
      strcpy(msg, "Elementul a fost sters.");
    }
    else if (strncmp(msg, "read:", 5) == 0)
    {
      int id = atoi(msg + 5);
      strcpy(msg, read_from_db(id).c_str());
    }
    else if (strncmp(msg, "read_all:", 9) == 0)
    {
      strcpy(msg, read_from_db().c_str());
    }
    else
    {
      std::cout << "Mesaj nerecunoscut: " << msg << std::endl;
    }

    /*pregatim mesajul de raspuns */
    bzero(msgrasp, 100);
    strcat(msgrasp, msg);

    // printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

    /* returnam mesajul clientului */
    if (write(client, msgrasp, 100) <= 0)
    {
      printf("[server]Eroare la catre client. '%s'\n", msgrasp);
      continue; /* continuam sa ascultam */
    }
    /* am terminat cu acest client, inchidem conexiunea */
    close(client);
  } /* while */
}
