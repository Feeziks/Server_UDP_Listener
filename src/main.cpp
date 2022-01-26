#include <iostream>
#include <unistd.h>

#include <string>
#include <queue>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <time.h>

#include "network.h"
#include "logger.h"
#include "database.h"
#include "databasesecrets.h"

// Forward decleration of functions
void *ReaderThreadFunction(void *args);
void SigintHandler(int sig);

// Global variables
int sockfd = -1; // Socket file descriptor - set up happens in main reading in reader thread
std::queue<packet_buffer> packet_queue; // Packet for ReaderThread to write incoming messages into
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for queue data
logger myLog(logVerbosity::high); // logger for us to store mistakes
pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for logger
volatile sig_atomic_t running; // Should the threads keep running?

int main(int argc, char **argv)
{
  // Initialize variables
  running = 1;
  bool ret = false; // Used to check for errors in function calls
  bool pass = true; // Used to check for errors in while loop
  int sts = -1; // Status for function calls
  struct timespec time1, time2; // For use in nano sleep
  timeval respondTime; // Checking time between recv a packet and responding to it
  long elapsedSeconds, elapsedMicroseconds; // Elapsed time
  double totalElapsed; // Total elapsed time from recieve to respond for an individual packet
  
  packet_buffer thisPacket; // Packet that we read from the queue
  DatabaseMgr dbMgr; // Database manager
  databasesecrets my_dbsecrets; // Database secrets
  pthread_t reader_thread; // Thread that will put the udp packets into our queue
  myLog.Log(logLevel::comment, "Beginning listener, creating and binding socket\n");
  myLog.SetMutex(logMutex);

  // Register our sigint handler to watch for ctrl-c
  signal(SIGINT, SigintHandler);
  // Create socket for UDP connections
  ret = CreateAndBindSocket(sockfd, &myLog);
  if(!ret)
  {
    myLog.Log(logLevel::error, "Unable to create socket\nExiting program\n");
    running = 0;
    exit(EXIT_FAILURE);
  }

  myLog.Log(logLevel::comment, "Socket created and bound\n");

  sts = pthread_create(&reader_thread, NULL, ReaderThreadFunction, NULL);
  if(sts != 0)
  {
    myLog.Log(logLevel::error, "Pthread create failed. Exiting program\n");
    running = 0;
    exit(EXIT_FAILURE);
  }

  myLog.Log(logLevel::comment, "Opening database file and listening to socket\n");
  ret = dbMgr.OpenDatabaseFile(my_dbsecrets.filePath, &myLog);
  if(!ret)
  {
    myLog.Log(logLevel::error, "Unable to open database file\nExiting program\n");
    running = 0;
    exit(EXIT_FAILURE);
  }

  // Set sleep to last 25ms (1/40 second)
  time1.tv_sec = 0;
  time2.tv_nsec = 25000000L;

  do
  {
    // Check for data in the queue
    if(!packet_queue.empty())
    {
      // Get the mutex for the queue then read the first item
      pthread_mutex_lock(&queueMutex);
      thisPacket = packet_queue.front(); // Read the value
      packet_queue.pop(); // Remove it from the queue
      pthread_mutex_unlock(&queueMutex);
      // TODO: decrypt recieved data
      // Parse it
      switch(GetPacketType(thisPacket.data, &myLog))
      {
        case login_t:
        {
          std::string emailString = "";
          std::string passString = "";
          pass = ParseLoginPacket(thisPacket.data, thisPacket.dataLen, 
            emailString, passString, &myLog);
          if(pass)
          {
            pass = dbMgr.TryLogin(emailString, passString);
          }
          break;
        }
        case register_packet_t:
          break;
        default:
          myLog.Log(logLevel::warning, "Packet with malformed header recieved\n");
          break;
      }
      // Get the time we finished with this packet
      gettimeofday(&respondTime, 0);
      elapsedSeconds = thisPacket.recvTime.tv_sec - respondTime.tv_sec;
      elapsedMicroseconds = thisPacket.recvTime.tv_usec = respondTime.tv_usec;
      totalElapsed = elapsedSeconds + elapsedMicroseconds;
      char s[100];
      snprintf(s, 100, "%.02f seconds elapsed from packet recieve to response\n", totalElapsed);
      myLog.Log(logLevel::comment, std::string(s));
      //TODO: Create a moving average of response time. Probably just do some window averageing
      // over some number of packets
    }
    else
    {
      // The queue is empty sleep 
      nanosleep(&time1, &time2);
    }

    // Exit if appropriate
    //ret = false;
  }while(ret && (running == 1));

  running = 0;
  pthread_join(reader_thread, NULL);

  myLog.Log(logLevel::comment, "Closing socket and exiting program\n");
  CloseSocket(sockfd);
  myLog.CloseLog();
  return 0;
}

void *ReaderThreadFunction(void *ptr)
{
  // Listen to the UDP port with recv_from
  // Place the data into the queue
  
  // Variable initialization
  int bytesrcv = -1; // Bytes recieved
  socklen_t addrLen; // Address length from sender
  packet_buffer tmpBuffer; // Temporary holding for the recieved data & sender before queue

  // Prepare the socket to use polling so there is a timeout eventually
  // Allows us to respond to signals and interrupts
  struct pollfd pfd; // poll file descripter used to tell poll which files to check
  int sts = -1; // Status from poll call 
  int timeout = 500; // timeout every half a second
  pfd.fd = sockfd; // poll on socket file
  pfd.events = POLLIN; // poll for incoming data

  while(running)
  {
    // Poll for changes on sockfd
    sts = poll(&pfd, 1, timeout);
    switch(sts)
    {
      case -1:
        // An error occured in poll
        myLog.Log(logLevel::warning, "error occured when polling sockfd for incoming data\n");
        break;
      case 0:
        // Timeout occured - do nothing
        break;
      default:
        // Data is ready to be read from the socket
        // Recieve data on the opened socket
        bytesrcv = recvfrom(sockfd, tmpBuffer.data, UDP_BUFFER_LEN - 1, 
          0, (struct sockaddr *)&tmpBuffer.sender, &addrLen);
        // Make sure that we recieved an actual number of bytes
        if(bytesrcv <= 0)
        {
          myLog.Log(logLevel::warning, "recvfrom error occured, not enqueueing this packet\n");
        }
        else
        {
          // Add a null terminator to the end of the buffer
          tmpBuffer.data[bytesrcv] = '\0';
          tmpBuffer.dataLen = bytesrcv;
          // Get the recieved time for the packet
          gettimeofday(&tmpBuffer.recvTime, 0);
          // Put the data into the queue
          pthread_mutex_lock(&queueMutex);
          packet_queue.push(tmpBuffer);
          pthread_mutex_unlock(&queueMutex);
          // Clear the tmpBuffer data for next time
          memset(&tmpBuffer.data, 0, UDP_BUFFER_LEN);
          memset(&tmpBuffer.sender, 0, sizeof(struct sockaddr_storage));
          tmpBuffer.dataLen = 0;
        }
    }
    // Keep looping until asked to stop
  }

  return 0; // Required to remove no retrun warning. DO NOT DE-REFERENCE
}

void SigintHandler(int sig)
{
  running = 0;
}
