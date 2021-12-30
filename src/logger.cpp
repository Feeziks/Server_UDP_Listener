#include "logger.h"

// **************************************************
//              CONSTRUCTORS
// **************************************************

logger::logger(logVerbosity v)
  : m_verbosity(v)
{
  std::string filePath = GetFilenameTimestamp();
  m_fstream.open(filePath);
  Log(logLevel::comment, "Log file created succesfully!\n");
  m_mutexSet = false;
}

// **************************************************
//              DESTRUCTORS
// **************************************************
logger::~logger()
{
  //Close the fstream
  m_fstream.close();
}

// **************************************************
//              PUBLIC METHODS
// **************************************************
void logger::Log(logLevel level, std::string logString)
{
  // Check if we need to lock on the mutex
  if(m_mutexSet)
  {
    pthread_mutex_lock(&m_mutex);
  }
  std::string timeStamp = GetTimestamp();
  // Check if our verbosity level lets this log through
  switch(level)
  {
    case logLevel::comment:
      // Comments only logged when verbosity set to high
      if(logVerbosity::high == m_verbosity)
      {
        m_fstream << timeStamp << "\tComment: " << logString;
      }
      break;
    case logLevel::warning:
      // Warnings logged when verbosity set to high or medium
      if((logVerbosity::high == m_verbosity) || (logVerbosity::med == m_verbosity))
      {
        m_fstream << timeStamp << "\tWarning: " << logString;
      }
      break;
    case logLevel::error: //Fall through on purpose
    case logLevel::exception:
      // Always log errors and exceptions
      m_fstream << timeStamp << "\tError: " << logString;
      break;
    default:
      // Log that a bad logger called happened
      m_fstream << timeStamp << "\tInvalid call to log occured!\n";
      m_fstream << "Attempted to log: " + logString;
      break;
  }
  // Check if we need to unlock on the mutex
  if(m_mutexSet)
  {
    pthread_mutex_unlock(&m_mutex);
  }
}

void logger::CloseLog()
{
  m_fstream.close();
}

void logger::SetMutex(pthread_mutex_t m)
{
  m_mutexSet = true;
  m_mutex = m;
}

///////// Private Methods
std::string logger::GetFilenameTimestamp()
{
  //Get the current time and create a string from it
  std::time_t t = std::time(0);
  struct tm *timeInfo;
  char buffer[80];
  timeInfo = localtime(&t);
  strftime(buffer, 80, FILE_NAME_TIME_FORMAT, timeInfo);

  //Creat the file path string
  std::string filePath = std::string("../logs/") + std::string(buffer);
  return filePath;
}

std::string logger::GetTimestamp()
{
  //Get the current time and create a string from it
  std::time_t t = std::time(0);
  struct tm *timeInfo;
  char buffer[80];
  timeInfo = localtime(&t);
  strftime(buffer, 80, LOGGING_TIME_FORMAT, timeInfo);
  //Return the time stamp as std::string
  return std::string(buffer);
}
