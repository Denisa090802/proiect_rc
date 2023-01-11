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
#include <sys/stat.h>
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

int child;
char msg[1024];

int main()
{
  mkfifo("broadcast.buffer", 0666);
  setup_db();
  int pid = fork();
  if (pid != 0)
  {
    if (fork())
    {
      cout << "IP adress: " << getIpAddress() << endl;
      listen_server(connected_peers);
    }
    else
    {
      int fd1 = open("broadcast.buffer",O_RDONLY);
      while (true)
      {
        if (read(fd1, msg, 1024) < 0)
        {
          printf("Error reading from stocketpair\n");
        }
        else
        {
          fflush(stdout);
          char msg2[1024];
          strcpy(msg2, msg);

          char *p = strtok(msg2, "|");
          strcpy(msg, p);
          char ip[50];
          p = strtok(NULL, "|");
          strcpy(ip, p);

          printf("{%s} -> {%s}\n", ip, msg);
          fflush(stdout);

          broadcast_insert(ip, msg);
          // if (write(pipes[1], "MNAAAAA", sizeof("MNAAAAA")) < 0) perror("[parinte]Err...write");
        }
      }
    }
  }
  else
  {
    while (true)
    {
      get_clients();
      sleep(5);
    }
  }
}