#ifndef __database_h__
#define __database_h__

#include "logger.h"
#include "databasesecrets.h"

#include <string>
#include <iostream>
#include <sqlite3.h>

class DatabaseMgr
{
public:
  // Constructor
  DatabaseMgr();

  // Destructor
  ~DatabaseMgr();

  // Public Member variables

  // Public Methods
  bool OpenDatabaseFile(std::string filePath, logger *log = NULL);
  void CloseDatabaseFile();
private:
  // Private Member variables
  sqlite3 *m_db;
  sqlite3_stmt *m_currStatement;
  // Private Methods
};


#endif
