#include "network.h"

/// Gets the type of packet
/// Does this by checking the header and comparing against known constants
packet_t GetPacketType(const uint8_t *packet, logger *log /*= NULL*/)
{
  // Initialize return variable
  packet_t ret = END_PACKET_TYPE;
  //Get the header from the packet
  uint16_t header = ((uint16_t)packet[0] << 8) | (packet[1]);
  //Find the header within the headerToPacketType map
  if(headerToPacketType.count(header) > 0)
  {
    // Attempt to find the header
    try
    {
      ret = headerToPacketType.at(header);
    }
    catch (const std::exception & e)
    {
      if(log != NULL)
      {
        char headerString[7];
        snprintf(headerString, 7, "0x%04X", header);
        std::string exceptionString = e.what();
        std::string logString = "Recieved a packet with invalid header value: ";
        log->Log(logLevel::exception, std::string(logString + headerString + 
          "\texception: " + exceptionString + "\n"));
      }
    }
  }
  // Return the packet type
  return ret;
}

bool ParseLoginPacket(const uint8_t *packet, const int packetLen, std::string &email, 
      std::string &pass, logger *log /*= NULL*/)
{
  // Initialize return values
  bool ret = false;
  email = "";
  pass = "";
  size_t startpos = -1;
  size_t endpos = -1;

  // Create a string from the characters in packet
  std::string packetString((char *)packet, packetLen);  
  // Find where the email token begins and ends and take the substring
  startpos = packetString.find(loginPacketTokens[0]);
  endpos = packetString.find(loginPacketTokens[1]);
  if((startpos != std::string::npos) && (endpos != std::string::npos))
  {
    ret = true;
    email = packetString.substr(startpos + loginPacketTokens[0].size(),
      endpos - startpos - loginPacketTokens[1].size() - 1);
  }
  
  // Repeat for password
  if(ret)
  {
    startpos = endpos;
    endpos = packetString.size();
    pass = packetString.substr(startpos + loginPacketTokens[1].size(), endpos - 1);
  }

  std::cout << email << "\t" << pass << "\n";

  return ret;
}

bool ParseRegisterPacket(const uint8_t *packet, const int packetLen, std::string &email,
      std::string &pass, std::string &username, logger *log /*= NULL*/)
{
  // Initialize return value
  bool ret = false;

  return ret;
}

bool CreateAndBindSocket(int &sockfd, logger *log /* = NULL */)
{
  // Initialize return variable
  bool ret = false;

  // Initialize variables
  int status = -1; // Used to tell if errors have occured
  struct addrinfo hints; // Hints for get address info
  struct addrinfo *servinfo; // Server info
  struct addrinfo *iterator; // iterator for servinfo linked list  
  networksecrets my_secrets; // Secrets

  bool loggingEnabled = false;
  if(NULL != log)
  {
    loggingEnabled = true;
  }

  // Set hints values
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET; // use IPV4
  hints.ai_socktype = SOCK_DGRAM; // UDP
  hints.ai_flags = AI_PASSIVE; //Fill in this machines IP addres
  hints.ai_protocol = 0; // Any protocol
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  // Get the address info
  status = getaddrinfo(NULL, my_secrets.portString, &hints, &servinfo);
  if(status != 0)
  {
    if(loggingEnabled)
    {
      log->Log(logLevel::error, "Unable to getaddrinfo\n");
      std::string s = std::string(gai_strerror(status));
      s = s + "\n";
      log->Log(logLevel::error, s);
    }
    ret = false;
  }
  else
  {
    ret = true;
  }

  // If we got the address info without error
  if(ret)
  {
    //Create a socket at the available address, Attempt to bind to it
    for(iterator = servinfo; iterator != NULL; iterator = iterator->ai_next)
    {
      status = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);
      if(-1 == status)
      {
        // Socket wasnt made, try again
        perror("Socket");
        continue;
      }
      // Set the socket file descriptor to the output
      sockfd = status;
      // Try to bind if the socket call worked
      status = bind(sockfd, iterator->ai_addr, iterator->ai_addrlen);
      if(-1 == status)
      {
        perror("Bind");
        // Close the socket and try again with the next address
        close(sockfd);
        sockfd = -1;
        continue;
      }

      // Socket was created and bound correctly
      ret = true;
      break;
    }
  }

  if(NULL == iterator)
  {
    if(loggingEnabled)
    {
      log->Log(logLevel::error, "Unable to create and bind to a socket\n");
    }
    ret = false; // This should already be set to false but might as well ensure it here
  }

  freeaddrinfo(servinfo);

  return ret;
}

void CloseSocket(int sockfd)
{
  close(sockfd);
}
