#include "exceptions.h"

QString SQLErrorException::getErrorCodeDescription(int errorCode)
{
  QString errorDesc = "SQL-Error %1: ";
  errorDesc = errorDesc.arg(0, 0, errorCode);
  switch (errorCode) {
  case  1 :  /* Generic error */
    errorDesc += "Generic error";
    break;
  case  2 :  /* Internal logic error in SQLite */
    errorDesc += "Internal logic error in SQLite";
    break;
  case  3 :  /* Access permission denied */
    errorDesc += "Access permission denied";
    break;
  case  4 :  /* Callback routine requested an abort */
    errorDesc += "Callback routine requested an abort";
    break;
  case  5 :  /* The database file is locked */
    errorDesc += "The database file is locked";
    break;
  case  6 :  /* A table in the database is locked */
    errorDesc += "A table in the database is locked";
    break;
  case  7 :  /* A malloc() failed */
    errorDesc += "A malloc() failed";
    break;
  case  8 :  /* Attempt to write a readonly database */
    errorDesc += "Attempt to write a readonly database";
    break;
  case  9 :  /* Operation terminated by sqlite3_interrupt()*/
    errorDesc += "Operation terminated by sqlite3_interrupt()";
    break;
  case 10 :  /* Some kind of disk I/O error occurred */
    errorDesc += "Some kind of disk I/O error occurred";
    break;
  case 11 :  /* The database disk image is malformed */
    errorDesc += "The database disk image is malformed";
    break;
  case 12 :  /* Unknown opcode in sqlite3_file_control() */
    errorDesc += "Unknown opcode in sqlite3_file_control()";
    break;
  case 13 :  /* Insertion failed because database is full */
    errorDesc += "Insertion failed because database is full";
    break;
  case 14 :  /* Unable to open the database file */
    errorDesc += "Unable to open the database file";
    break;
  case 15 :  /* Database lock protocol error */
    errorDesc += "Database lock protocol error";
    break;
  case 17 :  /* The database schema changed */
    errorDesc += "The database schema changed";
    break;
  case 18 :  /* String or BLOB exceeds size limit */
    errorDesc += "String or BLOB exceeds size limit";
    break;
  case 19 :  /* Abort due to constraint violation */
    errorDesc += "Abort due to constraint violation";
    break;
  case 20 :  /* Data type mismatch */
    errorDesc += "Data type mismatch";
    break;
  case 21 :  /* Library used incorrectly */
    errorDesc += "Library used incorrectly";
    break;
  case 22 :  /* Uses OS features not supported on host */
    errorDesc += "Uses OS features not supported on host";
    break;
  case 23 :  /* Authorization denied */
    errorDesc += "Authorization denied";
    break;
  case 25 :  /* 2nd parameter to sqlite3_bind out of range */
    errorDesc += "2nd parameter to sqlite3_bind out of range";
    break;
  case 26 :  /* File opened that is not a database file */
    errorDesc += "File opened that is not a database file";
    break;
  case 27 :  /* Notifications from sqlite3_log() */
    errorDesc += "Notifications from sqlite3_log()";
    break;
  case 28 :  /* Warnings from sqlite3_log() */
    errorDesc += "Warnings from sqlite3_log()";
    break;
  case 100:  /* sqlite3_step() has another row ready */
    errorDesc += "sqlite3_step() has another row ready";
    break;
  case 101:  /* sqlite3_step() has finished executing */
    errorDesc += "sqlite3_step() has finished executing";
    break;
  default:
    errorDesc += "Unknown Error";
  }
  return errorDesc;
}
