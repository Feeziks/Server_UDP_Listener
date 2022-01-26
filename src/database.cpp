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

bool DatabaseMgr::TryLogin(std::string email, std::string pass, logger *log /* = NULL */)
{
  // Initialize return variable
  bool ret = false;
  // Init internal variables
  int sts = -1;
  std::string sqlStmt;
  std::string storedPass;
  bool loggingEnabled = false; // Wether we should log or not
  if(log != NULL)
  {
    loggingEnabled = true;
  }

  // Create a list of strings for the user input, and the sanitized output
  std::vector<std::string> userInputs;
  userInputs.push_back(email);
  userInputs.push_back(pass);
  std::vector<std::string> sanitizedInputs;

  // Sanitize the user inputs
  ret = SanitizeUserInputStrings(&userInputs, sanitizedInputs);
  if(!ret) { return false;}
  // Check a db file is open
  if(NULL == m_db) { return false;}
  // Check the registered users table in the DB for this user
  sqlStmt = "SELECT email, password FROM "  + my_dbsecrets.regUsersTableName + 
    " WHERE email = " + sanitizedInputs[0];
  std::cout << sqlStmt << "\n";
  sts = sqlite3_prepare_v2(m_db, sqlStmt.c_str(), -1, &m_currStatement, NULL);
  if(SQLITE_OK != sts)
  {
    if(loggingEnabled)
    {
      const char *errString = sqlite3_errmsg(m_db);
      std::string logString(errString);
      logString.insert(0, "Failed to prepare select statement in TryLogin: ", 0, 48);
      logString.append("\n");
      log->Log(logLevel::error, logString);
    }
    ret = false;
    return ret;
  }
  // Check the reults
  while(sqlite3_step(m_currStatement) != SQLITE_DONE)
  {
    // Get the password value
    storedPass = std::string(reinterpret_cast<const char*>(sqlite3_column_text(m_currStatement, 1)));
    std::cout << storedPass << "\n";
  }
  // Check that we got something
  if(storedPass.empty()) {ret = false; return ret;}
  // Verify the recieved password
  if(ARGON2_OK != argon2id_verify(storedPass.c_str(), 
    reinterpret_cast<void *>(pass[0]), pass.size())) {ret = false; return ret;}
  // We got a hit - run the hashing on the password and comapre the results
  // Need to pull the salt from the stored password
  // Then use it in the hash to check for equality
  std::cout << "We ade it here\n";
  // Return if the user exists or not
  return ret;
}

// *********************************************
//              PRIVATE METHODS
// *********************************************
bool DatabaseMgr::SanitizeUserInputStrings(std::vector<std::string> *input, std::vector<std::string> &output)
{
  // Initialize return variable
  bool ret = false;

  // Initialize return list
  output.clear();
  // Sanitize the inputs
  for(size_t i  = 0; i < input->size(); i++)
  {
    char *tmp = sqlite3_mprintf("%Q", (*input)[i].c_str());
    if(0 == strcmp(tmp, "NULL"))
    {
      ret = false;
      sqlite3_free(tmp);
      break;
    }
    else
    {
      ret = true;
      output.push_back(tmp);
      sqlite3_free(tmp);
    }
  }
  // Return
  return ret;
}
