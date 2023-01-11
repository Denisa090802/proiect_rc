/*
External client. Used to interact with the distributed hash table application
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <string>

using namespace std;

/* portul de conectare la server*/
int port = 8452;

string send_msg(string ip, string msg)
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(ip.c_str());
  server.sin_port = htons(port);

  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
  }

  if (write(sd, msg.c_str(), 100) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
  }

  char msg_rec[100];
  if (read(sd, msg_rec, 100) < 0)
  {
    perror("[client]Eroare la read() de la server.\n");
  }

  close(sd);
  string response = msg_rec;
  return response;
}

int main(int argc, char *argv[])
{
  char msg[100];
  char ip[100];
  bzero(ip, 100);
  printf("Introduceti adresa ip: ");
  fflush(stdout);
  read(0, ip, 100);

  while (true)
  {
    bzero(msg, 100);
    printf("> ");
    fflush(stdout);
    read(0, msg, 100);

    bool is_read_all = false;
    if (strncmp(msg, "read_all:", 9) == 0)
    {
      is_read_all = true;
    }

    string result = send_msg(ip, msg);
    if (!is_read_all)
    {
      printf("/> %s\n", result.c_str());
    }
    else
    {
      char buff[1000];
      strcpy(buff, result.c_str());
      char *p = strtok(buff, ",");
      while (p)
      {
        string msgq = "read:" + to_string(atoi(p));

        string r = send_msg(ip, msgq);
        printf("%s\n", r.c_str());

        p = strtok(NULL, ",");
      }
    }
  }
}