#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <sqlite3.h>

#include "persistence.h"
#include "drmaa2-mock.h"
#include "drmaa2-debug.h"



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
session_name TEXT,\
template_id INTEGER,\
pid INTEGER,\
\
exit_status INTEGER,\
terminating_signal INTEGER,\
submission_time INTEGER,\
dispatch_time INTEGER,\
finish_time INTEGER\
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
session_name TEXT,\
remoteCommand TEXT,\
args TEXT\
);\
\
CREATE TABLE reservation_templates(\
session_name TEXT,\
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

// sqlite3 callback function signature
typedef int (*sqlite3_callback)(void*, int, char**, char**);

// sqlite3 helper functions

sqlite3 *open_db(char *name)
{
	sqlite3 *db;
    int rc;
    rc = sqlite3_open(name, &db);
    sqlite3_busy_timeout(db, 30000);
    if( rc )
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    return db;
}

int evaluate_result_code(int rc, char *zErrMsg)
{
    if( rc!=SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return rc;
}

int drmaa2_setup_db(char *name)
{
    char *zErrMsg = 0;
    int rc;
    sqlite3 *db = open_db(name);
    rc = sqlite3_exec(db, setup, NULL, 0, &zErrMsg);
    evaluate_result_code(rc, zErrMsg);
    sqlite3_close(db);
}

int drmaa2_reset_db(char *name)
{
    char *zErrMsg = 0;
    int rc;
    sqlite3 *db = open_db(name);
    rc = sqlite3_exec(db, reset, NULL, 0, &zErrMsg);
    evaluate_result_code(rc, zErrMsg);
    sqlite3_close(db);
}


int drmaa2_db_query(char *stmt, sqlite3_callback callback, void *args)
{
    sqlite3 *db = open_db(DRMAA_DBFILE);
    char *zErrMsg = 0;

    DEBUG_PRINT("%s\n", stmt);
    int rc = sqlite3_exec(db, stmt, callback, args, &zErrMsg);

    evaluate_result_code(rc, zErrMsg);   
    sqlite3_close(db);
    return rc;
}

int drmaa2_db_query_rowid(char *stmt)
{
    sqlite3 *db = open_db(DRMAA_DBFILE);
    char *zErrMsg = 0;

    DEBUG_PRINT("%s\n", stmt);
    int rc = sqlite3_exec(db, stmt, NULL, NULL, &zErrMsg);
    sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);

    rc = evaluate_result_code(rc, zErrMsg);   
    sqlite3_close(db);
    if (rc != SQLITE_OK) return 0;
    return rowid;
}


//start of external used functions

int save_jsession(const char *contact, const char *session_name)
{
    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE; INSERT INTO job_sessions VALUES(%Q, %Q); COMMIT;", contact, session_name);
    int rc = drmaa2_db_query(stmt, NULL, NULL);
    sqlite3_free(stmt);
    return rc;
}

int delete_jsession(const char *session_name)
{
    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE;\
        DELETE FROM job_sessions WHERE name = %Q;\
        DELETE FROM job_templates WHERE session_name = %Q;\
        DELETE FROM jobs WHERE session_name = %Q;\
        COMMIT;", session_name, session_name, session_name);
    int rc = drmaa2_db_query(stmt, NULL, NULL);
    sqlite3_free(stmt);
    return rc;
}



static int get_jsession_callback(void **ptr, int argc, char **argv, char **azColName)
{
	assert(argc == 2);
	drmaa2_jsession js = (drmaa2_jsession)malloc(sizeof(drmaa2_jsession_s));
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "name"))
        	js->name = strdup(argv[i]);
        else
        	js->contact = argv[i] ? strdup(argv[i]) : NULL;
    }
    DEBUG_PRINT("\n");
    *ptr = js;
    return 0;
}

drmaa2_jsession get_jsession(const char *session_name)
{
	drmaa2_jsession js = NULL;
    char *stmt = sqlite3_mprintf("SELECT * FROM job_sessions WHERE name = %Q", session_name);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)get_jsession_callback, &js);
    sqlite3_free(stmt);
    if (js == NULL) printf("no such jobsession\n");
    return js;
}


static int drmaa2_entry_exists_callback(void *ptr, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    DEBUG_PRINT("%s = %s\n", azColName[0], argv[0] ? argv[0] : "NULL");
    *((int *)ptr) = atoi(argv[0]);
    return 0;
}

int drmaa2_session_is_valid(int session_type, const char *session_name)
{
    char *session_type_name = (session_type == 0) ? "job_sessions": "reservation_sessions";
    char *stmt = sqlite3_mprintf("SELECT EXISTS(SELECT 1 FROM %s WHERE name = %Q LIMIT 1);", session_type_name, session_name);
    int exists = 0;
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)drmaa2_entry_exists_callback, &exists);
    sqlite3_free(stmt);
    return exists;
}

int drmaa2_jsession_is_valid(const char *session_name)
{
    return drmaa2_session_is_valid(0, session_name);
}

int drmaa2_rsession_is_valid(const char *session_name)
{
    return drmaa2_session_is_valid(1, session_name);
}


long long save_job(const char *session_name, long long template_id)
{
    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE; INSERT INTO jobs \
        VALUES(%Q, %lld, %Q, %Q, %Q, datetime('now'), %Q, %Q); COMMIT;", session_name, template_id, NULL, NULL, NULL, NULL, NULL);
    sqlite3_int64 row_id = drmaa2_db_query_rowid(stmt);
    sqlite3_free(stmt);
    return row_id;
}


long long save_jtemplate(drmaa2_jtemplate jt, const char *session_name)
{
    char *stmt = sqlite3_mprintf("INSERT INTO job_templates VALUES(%Q, %Q, %Q)", session_name, jt->remoteCommand, jt->args);
    sqlite3_int64 row_id = drmaa2_db_query_rowid(stmt);
    sqlite3_free(stmt);
    return row_id;
}


static int get_jsession_names_callback(void *session_names, int argc, char **argv, char **azColName)
{
	assert(argc == 1);
    DEBUG_PRINT("%s = %s\n", azColName[0], argv[0] ? argv[0] : "NULL");

    drmaa2_string_list names = (drmaa2_string_list)session_names;
    drmaa2_list_add(names, strdup(argv[0]));
    DEBUG_PRINT("\n");
    return 0;
}

drmaa2_string_list get_jsession_names(drmaa2_string_list session_names)
{
    char stmt[] = "SELECT name FROM job_sessions";
    int rc = drmaa2_db_query(stmt, get_jsession_names_callback, session_names);
    if (rc != SQLITE_OK) return NULL;
    return session_names;
}


static int get_sjobs_callback(void *ptr, int argc, char **argv, char **azColName)
{
	assert(argc == 2);
	drmaa2_j j = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "session_name"))
        	j->session_name = strdup(argv[i]);
        else
        	j->id = strdup(argv[i]);
    }
    DEBUG_PRINT("\n");
    drmaa2_list_add((drmaa2_j_list)ptr, j);
    return 0;
}

drmaa2_j_list get_session_jobs(drmaa2_j_list jobs, const char *session_name)
{
    char *stmt = sqlite3_mprintf("SELECT session_name, rowid FROM jobs WHERE session_name = %Q;", session_name);
    int rc = drmaa2_db_query(stmt, get_sjobs_callback, jobs);
    sqlite3_free(stmt);
    if (rc != SQLITE_OK) return NULL;
    return jobs;
}



static int get_jobs_callback(void *ptr, int argc, char **argv, char **azColName)
{
    assert(argc == 2);
    drmaa2_j j = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "session_name"))
            j->session_name = strdup(argv[i]);
        else
            j->id = strdup(argv[i]);
    }
    DEBUG_PRINT("\n");
    drmaa2_list_add((drmaa2_j_list)ptr, j);
    return 0;
}

drmaa2_j_list get_jobs(drmaa2_j_list jobs, drmaa2_jinfo filter)
{
    //TODO: build full filter statement
    char *sqlfilter = NULL;
    if (filter){
        if (filter->exitStatus != -1)
        {
            asprintf(&sqlfilter, "WHERE exit_status = %d", filter->exitStatus);
        }
    }
    char *stmt = sqlite3_mprintf("SELECT session_name, rowid FROM jobs %s", sqlfilter);
    int rc = drmaa2_db_query(stmt, get_jobs_callback, jobs);
    sqlite3_free(stmt);
    free(sqlfilter);
    if (rc != SQLITE_OK) return NULL;
    return jobs;
}



int save_rsession(const char *contact, const char *session_name)
{
    char *stmt = sqlite3_mprintf("INSERT INTO reservation_sessions VALUES(%Q, %Q)", contact, session_name);
    int rc = drmaa2_db_query(stmt, NULL, 0);
    sqlite3_free(stmt);
    return rc;
}


int delete_rsession(const char *session_name)
{
    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE;\
    DELETE FROM reservation_sessions WHERE name = %Q;\
    DELETE FROM reservation_templates WHERE session_name = %Q;\
    DELETE FROM reservations WHERE session_name = %Q;\
    COMMIT;", session_name, session_name, session_name);
    int rc = drmaa2_db_query(stmt, NULL, 0);
    sqlite3_free(stmt);
    return rc;
}


static int get_rsession_callback(void **ptr, int argc, char **argv, char **azColName)
{
    assert(argc == 2);
    drmaa2_rsession rs = (drmaa2_rsession)malloc(sizeof(drmaa2_rsession_s));
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "name"))
            rs->name = strdup(argv[i]);
        else
            rs->contact = argv[i] ? strdup(argv[i]) : NULL;
    }
    DEBUG_PRINT("\n");
    *ptr = rs;
    return 0;
}

drmaa2_rsession get_rsession(const char *session_name)
{
    drmaa2_rsession rs = NULL;
    char *stmt = sqlite3_mprintf("SELECT * FROM reservation_sessions WHERE name = %Q", session_name);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)get_rsession_callback, &rs);
    sqlite3_free(stmt);
    if (rs == NULL) printf("no such reservation session\n");
    return rs;
}


long long save_reservation(const char *session_name, long long template_id)
{
    char *stmt = sqlite3_mprintf("INSERT INTO reservations VALUES(%Q, %lld, %Q, %Q, %Q, %Q, %Q)", session_name, template_id, NULL, NULL, NULL, NULL, NULL);
    sqlite3_int64 rowid = drmaa2_db_query_rowid(stmt);
    sqlite3_free(stmt);
    return rowid;
}


long long save_rtemplate(drmaa2_rtemplate rt, const char *session_name)
{
    char *stmt = sqlite3_mprintf("INSERT INTO reservation_templates VALUES(%Q, %Q)", session_name, rt->candidateMachines);
    sqlite3_int64 row_id = drmaa2_db_query_rowid(stmt);
    sqlite3_free(stmt);
    return row_id;
}


static int drmaa2_get_reservation_callback(drmaa2_string *session_name, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    assert(!strcmp(azColName[0], "session_name"));
    *session_name = strdup(argv[0]);
    return 0;
}

drmaa2_r drmaa2_get_reservation(const drmaa2_string reservationId)
{
    long long rowid = atoll(reservationId);
    char *stmt = sqlite3_mprintf("SELECT session_name FROM reservations WHERE rowid = %lld", rowid);
    drmaa2_string session_name = NULL;
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)drmaa2_get_reservation_callback, &session_name);
    sqlite3_free(stmt);
    if (rc != SQLITE_OK || !session_name) return NULL;
    else
    {
        drmaa2_r r = (drmaa2_r)malloc(sizeof(drmaa2_r_s));
        r->session_name = strdup(session_name);
        r->id = strdup(reservationId);
        return r;
    }
}


static int get_rsession_names_callback(void *session_names, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    DEBUG_PRINT("%s = %s\n", azColName[0], argv[0] ? argv[0] : "NULL");

    drmaa2_string_list names = (drmaa2_string_list)session_names;
    drmaa2_list_add(names, strdup(argv[0]));
    DEBUG_PRINT("\n");
    return 0;
}

drmaa2_string_list get_rsession_names(drmaa2_string_list session_names)
{
    char stmt[] = "SELECT name FROM reservation_sessions";
    int rc = drmaa2_db_query(stmt, get_rsession_names_callback, session_names);
    if (rc != SQLITE_OK) return NULL;
    return session_names;
}


static int drmaa2_get_sreservations_callback(void *ptr, int argc, char **argv, char **azColName)
{
    assert(argc == 2);
    drmaa2_r r = (drmaa2_r)malloc(sizeof(drmaa2_r_s));
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "session_name"))
            r->session_name = strdup(argv[i]);
        else
            r->id = strdup(argv[i]);
    }
    DEBUG_PRINT("\n");
    drmaa2_list_add((drmaa2_r_list)ptr, r);
    return 0;
}

drmaa2_r_list drmaa2_get_session_reservations(drmaa2_r_list reservations, const char *session_name)
{
    char *stmt = sqlite3_mprintf("SELECT session_name, rowid FROM reservations WHERE session_name = %Q;", session_name);
    int rc = drmaa2_db_query(stmt, drmaa2_get_sreservations_callback, reservations);
    sqlite3_free(stmt);
    if (rc != SQLITE_OK) return NULL;
    return reservations;
}


static int get_reservations_callback(void *ptr, int argc, char **argv, char **azColName)
{
    assert(argc == 2);
    drmaa2_r r = (drmaa2_r)malloc(sizeof(drmaa2_r_s));
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "session_name"))
            r->session_name = strdup(argv[i]);
        else
            r->id = strdup(argv[i]);
    }
    DEBUG_PRINT("\n");
    drmaa2_list_add((drmaa2_r_list)ptr, r);
    return 0;
}

drmaa2_r_list get_reservations(drmaa2_r_list reservations)
{
    char *stmt = "SELECT session_name, rowid FROM reservations";
    int rc = drmaa2_db_query(stmt, get_reservations_callback, reservations);
    if (rc != SQLITE_OK) return NULL;
    return reservations;
}



static int get_status_callback(int *ptr, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    if (argv[0] != NULL)
    {
        *ptr = atoi(argv[0]);
    }
    // else: status value stays -1
    return 0;
}

int drmaa2_get_job_status(drmaa2_j j)
{
    int status = -1;
    long long rowid = atoll(j->id);
    char *stmt = sqlite3_mprintf("SELECT exit_status FROM jobs WHERE rowid = %lld", rowid);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)get_status_callback, &status);
    sqlite3_free(stmt);
    return status;
}


static int get_jobinfo_callback(drmaa2_jinfo ji, int argc, char **argv, char **azColName)
{
    assert(argc == 5);
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "exit_status") && argv[i] != NULL)
            ji->exitStatus = atoi(argv[i]);
        else if (!strcmp(azColName[i], "terminating_signal") && argv[i] != NULL)
            ji->terminatingSignal = strdup(argv[i]);
        else if ((argv[i] != NULL) && 
    (!strcmp(azColName[i], "submission_time") || !strcmp(azColName[i], "dispatch_time") || !strcmp(azColName[i], "finish_time")) )
        {
            struct tm tm;
            time_t epoch;
            // convert time string to unix time stamp
            if ( strptime(argv[i], "%Y-%m-%d %H:%M:%S", &tm) != 0 )
                epoch = mktime(&tm);
            else
            {
                printf("time conversion error\n");
                exit(1);
            }

            if (!strcmp(azColName[i], "submission_time"))
                ji->submissionTime = epoch;
            if (!strcmp(azColName[i], "dispatch_time"))
                ji->dispatchTime = epoch;
            if (!strcmp(azColName[i], "finish_time"))
                ji->finishTime = epoch;
        }
    }
    DEBUG_PRINT("\n");
    return 0;
}

drmaa2_jinfo get_job_info(drmaa2_jinfo ji)
{
    long long rowid = atoll(ji->jobId);
    char *stmt = sqlite3_mprintf("SELECT exit_status, terminating_signal, submission_time, dispatch_time, finish_time\
        FROM jobs WHERE rowid = %lld", rowid);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)get_jobinfo_callback, ji);
    sqlite3_free(stmt);
    if (rc != SQLITE_OK) return NULL;
    return ji;
}





//queries for wrapper

static int cmd_callback(char **ptr, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    char *command = strdup(argv[0]);
    *ptr = command;
    return 0;
}

char *get_command(long long row_id)
{
    char *stmt = sqlite3_mprintf("SELECT remoteCommand FROM jobs, job_templates \
        WHERE jobs.rowid = %lld AND job_templates.rowid = jobs.template_id", row_id);
    char *command = NULL;
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)cmd_callback, &command);
    sqlite3_free(stmt);
    return command;
}


int drmaa2_save_pid(long long row_id, pid_t pid)
{
    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE; UPDATE jobs\
        SET pid = %d, dispatch_time = datetime('now') WHERE rowid = %lld; COMMIT;", pid, row_id);
    int rc = drmaa2_db_query(stmt, NULL, NULL);
    sqlite3_free(stmt);
    return rc;
}


int drmaa2_save_exit_status(long long row_id, int status)
{
    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE; UPDATE jobs \
        SET exit_status = %d, finish_time = datetime('now') WHERE rowid = %lld; COMMIT;", status, row_id);
    int rc = -1;
    while (rc != SQLITE_OK) //must ensure that status is really written as drmaa2-mock wait relies on it
    {
        rc = drmaa2_db_query(stmt, NULL, NULL);
        if (rc == SQLITE_OK)
            break;
        sleep(1);
    }    
    sqlite3_free(stmt);
    return rc;
}

