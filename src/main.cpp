#include <iostream>
#include <string>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include "secrets.h"

#define BUFF_SIZE 2048

int main(int argc, char **argv)
{
  secrets my_secrets;

  int socketfd;
  int opt = 1;
  char buffer[BUFF_SIZE];
  const char *hello = "Hello from the raspberry pi!";

  socketfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(socketfd < 0)
  {
    // Check what error occured
    std::cout << "Error number: " << errno << " occured when attempting to create the socket\n";
    exit(1);
  }

  if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    std::cout << "Error number: " << errno << " occured when attempting to set socket options\n";
    exit(EXIT_FAILURE);
  }

  std::cout << hello <<"\n";
}
