#include "database.h"

// *********************************************
//              CONSTRUCTOR
// *********************************************
DatabaseMgr::DatabaseMgr()
{
  m_db = NULL;
  m_currStatement = NULL;
}

// *********************************************
//              DESTRUCTOR
// *********************************************
DatabaseMgr::~DatabaseMgr()
{

}

// *********************************************
//              PUBLIC METHODS
// *********************************************

bool DatabaseMgr::OpenDatabaseFile(std::string filePath, logger *log /*= NULL */)
{
  // Initialize return variable
  bool ret = false;

  // Initialize variables
  int status = -1; // Status for sqlite3 calls
  bool loggingEnabled = false; // Wether we should log or not
  if(log != NULL)
  {
    loggingEnabled = true;
  }

  // Check if we have a file already opened
  if(m_db != NULL)
  {
    if(loggingEnabled)
    {
      log->Log(logLevel::warning, "A databse file was already open. Closing it and opening a new one\n");
    }
    CloseDatabaseFile();
  }

  // Open the databse file
  status = sqlite3_open(filePath.c_str(), &m_db);
  if(status)
  {
    if(loggingEnabled)
    {
      log->Log(logLevel::error, "Failed to open requested datbase file\n");
    }
    ret = false;
    CloseDatabaseFile();
  }
  else
  {
    // Return true - database file was opened 
    ret = true;
  }

  return ret;
}

void DatabaseMgr::CloseDatabaseFile()
{
    sqlite3_close(m_db);
}
