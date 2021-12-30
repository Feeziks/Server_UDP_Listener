#ifndef __logger_h__
#define __logger_h__

#include <string>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iostream>
#include <pthread.h>

#define FILE_NAME_TIME_FORMAT "%m_%d_%Y_%HH_%MM_%SS"
#define LOGGING_TIME_FORMAT "%HH_%MM_%SS"

enum class logVerbosity
{
  low,
  med,
  high
};

enum class logLevel
{
  comment,
  warning,
  error,
  exception
};

class logger
{
public:
  // Constructor options
  logger(logVerbosity);

  // Destructors
  ~logger();

  //Public Methods
  void SetMutex(pthread_mutex_t m);
  void Log(logLevel, std::string);
  void CloseLog();

private:
  //Private members
  const logVerbosity m_verbosity;
  std::ofstream m_fstream;
  pthread_mutex_t m_mutex;
  bool m_mutexSet;

  // Private methods
  std::string GetTimestamp();
  std::string GetFilenameTimestamp();
};

#endif
