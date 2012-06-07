#include <stdio.h>
#include <sqlite3.h>
 
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    for(i=0; i<argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

char setup[] = "\
CREATE TABLE job_sessions(\
contact TEXT,\
name TEXT UNIQUE NOT NULL\
);\
\
CREATE TABLE reservation_sessions(\
contact TEXT,\
name TEXT UNIQUE NOT NULL\
);\
\
CREATE TABLE jobs(\
pid INTEGER,\
session_name TEXT,\
template_id INTEGER,\
\
exit_status INTEGER\
);\
\
CREATE TABLE reservations(\
session_name TEXT,\
template_id INTEGER,\
\
reserved_start_time NUMERIC,\
reserved_end_time NUMERIC,\
users_ACL TEXT,\
reserved_slots INTEGER,\
reservedMachines TEXT\
);\
\
CREATE TABLE job_templates(\
remoteCommand TEXT,\
args TEXT\
);\
\
CREATE TABLE reservation_templates(\
candidate_machines TEXT\
);\
";

char reset[] = "\
DELETE FROM job_sessions;\
DELETE FROM reservation_sessions;\
DELETE FROM jobs;\
DELETE FROM reservations;\
DELETE FROM job_templates;\
DELETE FROM reservation_templates;\
";



int main(int argc, char **argv)
{
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;

    if( argc!=3 || (strcmp(argv[2], "SETUP") && strcmp(argv[2], "RESET")))
    {
        fprintf(stderr, "Usage: %s DATABASE (SETUP | RESET)\n", argv[0]);
        return(1);
    }
    rc = sqlite3_open(argv[1], &db);
    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return(1);
    }

    if (!strcmp(argv[2], "SETUP"))
    {
        rc = sqlite3_exec(db, setup, NULL, 0, &zErrMsg);
    }
    else
    {
        rc = sqlite3_exec(db, reset, NULL, 0, &zErrMsg);
    }
    if( rc!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
    return 0;
}