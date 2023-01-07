/* servTCPIt.c - Exemplu de server TCP iterativ
   Asteapta un nume de la clienti; intoarce clientului sirul
   "Hello nume".

   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "helpers/network_controller.h"
#include <iostream>
#include <set>
#include <queue>
using namespace std;

set<string> connected_peers;

void get_clients()
{
  string ip = getIpAddress();
  int poz = ip.find_last_of('.');
  while (ip.size() > poz)
  {
    ip.erase(poz);
  }
  ip += ".";
  for (int i = 2; i <= 24; i++)
  {
    string ip1 = ip;
    ip1 += to_string(i);
    check_if_client_is_active(ip1, connected_peers);
  }
}

int sockp[2], child; 
char msg[1024];

int main()
{
  setup_db();

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
  { 
    perror("Err... socketpair"); 
    exit(1); 
  }

  int pid = fork();
  if (pid != 0)
  {
    if (fork())
    {
      while (true)
      {
        get_clients();
        sleep(10);
      }
    }
    else
    {
        close(sockp[0]); 
        while(true)
        {
            if (read(sockp[1], msg, 1024) < 0) { printf("Error reading from stocketpair\n"); }
            else {
              fflush (stdout);
              char msg2[1024];
              strcpy(msg2, msg);

              char *p = strtok(msg2, "|");
              strcpy(msg, p);
              char ip[50];
              p = strtok(NULL, "|");
              strcpy(ip, p);

              printf("{%s} -> {%s}\n", ip, msg);
              fflush (stdout);

              sleep(1);

              broadcast_insert(ip, msg);
              // if (write(sockp[1], "MNAAAAA", sizeof("MNAAAAA")) < 0) perror("[parinte]Err...write"); 
            } 
            sleep(1);
        }
        close(sockp[1]); 
    }
  }
  else
  {
    cout << "IP adress: " << getIpAddress() << endl;
    listen_server(connected_peers, sockp);
  }
}