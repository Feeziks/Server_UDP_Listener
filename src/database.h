#ifndef __database_h__
#define __database_h__

#include "logger.h"
#include "databasesecrets.h"
#include "argon2.h"

#include <vector>
#include <string>
#include <string.h> // used for strcmp
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

  bool TryLogin(std::string email, std::string pass, logger *log = NULL);
private:
  // Private Member variables
  sqlite3 *m_db;
  sqlite3_stmt *m_currStatement;
  databasesecrets my_dbsecrets; // Database secrets

  // Private Methods
  bool SanitizeUserInputStrings(std::vector<std::string> *inputs, std::vector<std::string> &outputs);
};


#endif
