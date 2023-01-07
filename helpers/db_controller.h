#include <stdio.h>
#include <sqlite3.h>
#include <string>
#include <iostream>
using namespace std;

const char db_name[100] = "internal.db";

void test_db_connection()
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open(db_name, &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_close(db);
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void initialize_database()
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char *sql;

    /* Open database */
    rc = sqlite3_open(db_name, &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    /* Create SQL statement */
    sql = "CREATE TABLE EVENTS("
          "ID INT PRIMARY KEY     NOT NULL,"
          "NAME           CHAR(101)    NOT NULL,"
          "YEAR           INT     NOT NULL,"
          "MONTH           INT     NOT NULL,"
          "DAY          INT     NOT NULL,"
          "ADDRESS        CHAR(101)"
          ");";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    // if (rc != SQLITE_OK)
    // {
    //     fprintf(stderr, "SQL error: %s\n", zErrMsg);
    //     sqlite3_free(zErrMsg);
    // }
    sqlite3_close(db);
}

void insert_db(int id, std::string name, int year, int month, int day, std::string address)
{
    address.erase(std::remove(address.begin(), address.end(), '\n'),
            address.end());
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    string sql;

    /* Open database */
    rc = sqlite3_open(db_name, &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    /* Create SQL statement */
    sql = "insert into events (id,name, year, month, day, address) values (" + to_string(id) + ",'" + name + "'," + to_string(year) + ", " + to_string(month) + ", " + to_string(day) + ", '" + address + "')";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Successfully inserted\n");
    }
    sqlite3_close(db);
}

void update_db(int id, std::string name, int year, int month, int day, std::string address)
{
    address.erase(std::remove(address.begin(), address.end(), '\n'),
            address.end());
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    string sql;

    /* Open database */
    rc = sqlite3_open(db_name, &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    sql = "update events set name = '"+name+"', year = '"+to_string(year)+"', month = '"+to_string(month)+"', day = "+to_string(day)+", address = '"+address+"' where id = " + to_string(id);

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Successfully inserted\n");
    }
    sqlite3_close(db);
}

void delete_from_db(int id)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    string sql;

    /* Open database */
    rc = sqlite3_open(db_name, &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    /* Create SQL statement */
    sql = "delete from events where id = " + to_string(id);

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Successfully inserted\n");
    }
    sqlite3_close(db);
}

std::string read_from_db(int id)
{
     sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    /* Open database */
    rc = sqlite3_open(db_name, &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }
    
  sqlite3_stmt *stmt;
  std::string sql = "SELECT * FROM events where id="+to_string(id)+";";
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    std::string output="";
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int id = sqlite3_column_int(stmt, 0);
    const unsigned char *name = sqlite3_column_text(stmt, 1);
    int year = sqlite3_column_int(stmt, 2);
    int month = sqlite3_column_int(stmt, 3);
    int day = sqlite3_column_int(stmt, 4);
    const unsigned char *address = sqlite3_column_text(stmt, 5);
    std::string _name = (char *)name;
    std::string _address = (char*) address;
    output += ( _name + "," + to_string(day) + "," + to_string(month) + "," + to_string(year) + "," + _address + ";");
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return output;
}

bool check_id_db(int id)
{
    // Open a connection to the database
    sqlite3 *db;
    int rc = sqlite3_open(db_name, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }

    // Execute a SELECT statement
    std::string sql = "SELECT count(*) FROM events where id=" + to_string(id) + ";";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }

    // Iterate through the results
    int count;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        // Access the values using the column index
        count = sqlite3_column_int(stmt, 0);
    }

    // Finalize the statement to release resources
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Error finalizing statement: ", sqlite3_errmsg(db));
    }

    // Close the database connection
    sqlite3_close(db);
    if (count == 0)
        return false;
    else
        return true;
}