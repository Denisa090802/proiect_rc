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


/* portul de conectare la server*/
int port = 8452;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char msg[100];		// mesajul trimis
  char ip[100];
  bzero (ip, 100);
  printf ("Introduceti adresa ip: ");
  fflush (stdout);
  read (0, ip, 100);

  while(true)
  {
      // /* exista toate argumentele in linia de comanda? */
      // if (argc != 2)
      //   {
      //     printf ("Sintaxa: %s <adresa_server>\n", argv[0]);
      //     return -1;
      //   }

      /* cream socketul */
      if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
        {
          perror ("Eroare la socket().\n");
          return errno;
        }

      /* umplem structura folosita pentru realizarea conexiunii cu serverul */
      /* familia socket-ului */
      server.sin_family = AF_INET;
      /* adresa IP a serverului */
      server.sin_addr.s_addr = inet_addr(ip);
      /* portul de conectare */
      server.sin_port = htons (port);
      
      /* ne conectam la server */
      if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
        {
          perror ("[client]Eroare la connect().\n");
          return errno;
        }

      /* citirea mesajului */
      bzero (msg, 100);
      printf ("> ");
      fflush (stdout);
      read (0, msg, 100);
      
      /* trimiterea mesajului la server */
      if (write (sd, msg, 100) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }

      /* citirea raspunsului dat de server 
        (apel blocant pina cind serverul raspunde); Atentie si la cum se face read- vezi cursul! */
      if (read (sd, msg, 100) < 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
      /* afisam mesajul primit */
      printf ("/> %s\n", msg);

      /* inchidem conexiunea, am terminat */
      close (sd);
  }
}