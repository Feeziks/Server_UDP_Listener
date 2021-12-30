#ifndef __network_h__
#define __network_h__

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include <map>
#include "logger.h"
#include "networksecrets.h"
#include <cstring>
#include <iostream>

const uint8_t headerLen = 2; // Length of headers in bytes

const uint8_t login_header = (uint8_t)0x01FF;
const uint8_t register_header = (uint8_t)0x02FF;

enum packet_t
{
  login_t,
  register_packet_t, //register register_t are both used elsewhere
  END_PACKET_TYPE
};

const std::map<packet_t, uint8_t> packetTypeToHeader
  {
    {login_t, login_header},
    {register_packet_t, register_header}
  };
const std::map<uint8_t, packet_t> headerToPacketType
  {
    {login_header, login_t},
    {register_header, register_packet_t}
  };

packet_t GetPacketType(uint8_t *packet, logger *log = NULL);

bool ParseLoginPacket(uint8_t *packet, int packetLen, std::string *email, 
        std::string *pass, logger *log = NULL );

bool ParseRegisterPacket(uint8_t *packet, int packetLen, std::string *email, logger *log = NULL );


bool CreateAndBindSocket(int &sockfd, logger *log = NULL);
void CloseSocket(int sockfd);

#endif