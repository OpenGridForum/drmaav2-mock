#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include <sqlite3.h>

#include "drmaa2-persistence.h"
#include "drmaa2-specific.h"
#include "drmaa2-debug.h"
#include "drmaa2-list.h"



char setup[] = "\
CREATE TABLE IF NOT EXISTS job_sessions(\
id INTEGER,\
contact TEXT,\
name TEXT UNIQUE NOT NULL\
);\
\
CREATE TABLE IF NOT EXISTS reservation_sessions(\
contact TEXT,\
name TEXT UNIQUE NOT NULL\
);\
\
CREATE TABLE IF NOT EXISTS jobs(\
session_id INTEGER,\
session_name TEXT,\
template_id INTEGER,\
pid INTEGER,\
\
exit_status INTEGER,\
terminating_signal INTEGER,\
annotation TEXT,\
job_state INTEGER,\
job_sub_state TEXT,\
allocated_machines TEXT,\
submission_machine TEXT,\
job_owner TEXT,\
slots INTEGER,\
queue_name TEXT,\
wallclock_time INTEGER,\
time_amount INTEGER,\
submission_time INTEGER,\
dispatch_time INTEGER,\
finish_time INTEGER\
);\
\
CREATE TABLE IF NOT EXISTS reservations(\
session_name TEXT,\
template_id INTEGER,\
\
reservation_name TEXT,\
reserved_start_time NUMERIC,\
reserved_end_time NUMERIC,\
users_ACL TEXT,\
reserved_slots INTEGER,\
reserved_machines TEXT\
);\
\
CREATE TABLE IF NOT EXISTS job_arrays(\
session_name TEXT,\
template_id INTEGER,\
\
jobs TEXT\
);\
\
CREATE TABLE IF NOT EXISTS job_templates(\
session_name TEXT,\
remote_command TEXT,\
args TEXT,\
submit_as_hold INTEGER,\
rerunnable INTEGER,\
job_environment TEXT,\
working_directory TEXT,\
job_category TEXT,\
email TEXT,\
email_on_started TEXT,\
email_on_terminated TEXT,\
job_name TEXT,\
input_path TEXT,\
output_path TEXT,\
error_path TEXT,\
join_files INTEGER,\
reservation_id TEXT,\
queue_name TEXT,\
min_slots INTEGER,\
max_slots INTEGER,\
priority INTEGER,\
candidate_machines TEXT,\
min_phys_memory INTEGER,\
machine_os INTEGER,\
machine_arch INTEGER,\
start_time NUMERIC,\
deadline_time NUMERIC,\
stage_in_files TEXT,\
stage_out_files TEXT,\
resource_limits TEXT,\
accounting_id TEXT\
);\
\
CREATE TABLE IF NOT EXISTS reservation_templates(\
session_name TEXT,\
reservation_name TEXT,\
start_time NUMERIC,\
end_time NUMERIC,\
duration NUMERIC,\
min_slots INTEGER,\
max_slots INTEGER,\
job_category TEXT,\
users_ACL TEXT,\
candidate_machines TEXT,\
min_phys_memory INTEGER,\
machine_os INTEGER,\
machine_arch INTEGER\
);\
";

char reset[] = "\
DELETE FROM job_sessions;\
DELETE FROM reservation_sessions;\
DELETE FROM jobs;\
DELETE FROM reservations;\
DELETE FROM job_arrays;\
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


int drmaa2_reset_db(char *name)
{
    char *zErrMsg = 0;
    int rc;
    sqlite3 *db = open_db(name);
    rc = sqlite3_exec(db, setup, NULL, 0, &zErrMsg);
    evaluate_result_code(rc, zErrMsg);
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
    if (rc != SQLITE_OK)
        printf("%s\n", stmt);   
    sqlite3_close(db);
    return rc;
}

int drmaa2_db_query_rowid(char *stmt)
{
    sqlite3 *db = open_db(DRMAA_DBFILE);
    char *zErrMsg = 0;

    DEBUG_PRINT("%s\n", stmt);
    int rc = sqlite3_exec(db, stmt, NULL, NULL, &zErrMsg);
    sqlite3_int64 row_id = sqlite3_last_insert_rowid(db);

    rc = evaluate_result_code(rc, zErrMsg);   
    sqlite3_close(db);
    if (rc != SQLITE_OK) return 0;
    return row_id;
}


//start of external used functions

int save_jsession(drmaa2_jsession *jsRef)
{
    drmaa2_jsession js = *jsRef;
    js->id = drmaa2_random_int(); 
    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE; INSERT INTO job_sessions VALUES(%lld, %Q, %Q); COMMIT;", js->id, js->contact, js->name);
    int rc = drmaa2_db_query(stmt, NULL, NULL);
    sqlite3_free(stmt);
    return rc;
}

int delete_jsession(const char *session_name)
{
    char *stmt = sqlite3_mprintf("DELETE FROM job_sessions WHERE name = %Q", session_name);
//    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE;\
//        DELETE FROM job_sessions WHERE name = %Q;\
//        DELETE FROM job_templates WHERE session_name = %Q;\
//        DELETE FROM jobs WHERE session_name = %Q;\
//        COMMIT;", session_name, session_name, session_name);
    int rc = drmaa2_db_query(stmt, NULL, NULL);
    sqlite3_free(stmt);
    return rc;
}



static int get_jsession_callback(void **ptr, int argc, char **argv, char **azColName)
{
	assert(argc == 3);
	drmaa2_jsession js = (drmaa2_jsession)malloc(sizeof(drmaa2_jsession_s));
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "name"))
        	js->name = strdup(argv[i]);
        else if (!strcmp(azColName[i], "contact"))
        	js->contact = argv[i] ? strdup(argv[i]) : NULL;
        else
            js->id = atoi(argv[i]);
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


long long save_job(drmaa2_jsession js, long long template_id)
{
    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE; INSERT INTO jobs \
        VALUES(%lld, %Q, %lld, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, datetime('now'), %Q, %Q); COMMIT;",
         js->id, js->name, template_id, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    sqlite3_int64 row_id = drmaa2_db_query_rowid(stmt);
    sqlite3_free(stmt);
    return row_id;
}


long long save_jtemplate(drmaa2_jtemplate jt, const char *session_name)
{
    char *stmt = sqlite3_mprintf("INSERT INTO job_templates VALUES(%Q, %Q, %Q, %d, %d, %Q, %Q, %Q, %Q, %d, %d, %Q, %Q, %Q,\
                                %Q, %d, %Q, %Q, %lld, %lld, %lld, %Q, %lld, %d, %d, %d, %d, %Q, %Q, %Q, %Q)",
                    session_name, jt->remoteCommand, jt->args, jt->submitAsHold, jt->rerunnable, NULL, jt->workingDirectory,
                    jt->jobCategory, NULL, jt->emailOnStarted, jt->emailOnTerminated, jt->jobName, jt->inputPath,
                    jt->outputPath, jt->errorPath, jt->joinFiles, jt->reservationId, jt->queueName, jt->minSlots,
                    jt->maxSlots, jt->priority, NULL, jt->minPhysMemory, jt->machineOS, jt->machineArch, jt->startTime,
                    jt->deadlineTime, NULL, NULL, NULL, jt->accountingId);
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


int has_to_be_escaped(const char *key) {
    if (strcmp(key, "session_name") == 0)
        return 1;
    else if (strcmp(key, "job_sub_state") == 0)
        return 1;
    else if (strcmp(key, "submission_time") == 0)
        return 1;
    else if (strcmp(key, "dispatch_time") == 0)
        return 1;
    else if (strcmp(key, "finish_time") == 0)
        return 1;
    else
        return 0;
}


char *comperator(const char *key) {
    if (strcmp(key, "submission_time") == 0)
        return ">=";
    else if (strcmp(key, "dispatch_time") == 0)
        return ">=";
    else if (strcmp(key, "finish_time") == 0)
        return ">=";
    else
        return "=";
}



char *build_query_string(char* start, drmaa2_dict filter) {
    if (filter == DRMAA2_UNSET_DICT)
        return strdup(start);

    size_t req_space = strlen(start) + 2; // 2 - 1 whitespace and terminating '\0'
    drmaa2_string_list keys = drmaa2_dict_list(filter);

    size_t i;
    for (i = 0; i < drmaa2_list_size(keys); i++) {
        const char *key = drmaa2_list_get(keys, i);
        req_space += strlen(key) + strlen(drmaa2_dict_get(filter, key)) + 4; // 4 - 2 whitespace and "="/">="
        if (has_to_be_escaped(key)) {
            req_space += 2;
        }
    }

    if (drmaa2_list_size(keys) > 0)
    {
        req_space += 7; // " WHERE "
        req_space += (drmaa2_list_size(keys) - 1) * 5;
    }

    char *query_string = (char *)malloc(sizeof(char *) * req_space);
    
    char *ins = query_string;
    strcpy(ins, start);
    ins[strlen(start)] = ' ';
    ins += strlen(start) + 1;

    char *cur_filter;

    for (i = 0; i < drmaa2_list_size(keys); i++) {
        if (i != 0) {
            strcpy(ins, " AND ");
            ins += 5;
        } else {
            strcpy(ins, " WHERE ");
            ins += 7;
        }

        const char *key = drmaa2_list_get(keys, i);
        const char *value = drmaa2_dict_get(filter, key);
        if (has_to_be_escaped(key)) {
            asprintf(&cur_filter, "%s %s \"%s\"", key, comperator(key), value);
        }
        else {
            asprintf(&cur_filter, "%s %s %s", key, comperator(key), value);
        }

        strcpy(ins, cur_filter);
        ins += strlen(cur_filter);
    }
    query_string[req_space - 1] = '\0';
    drmaa2_list_free(&keys);

    return query_string;
}


drmaa2_j_list get_jobs(drmaa2_j_list jobs, drmaa2_jsession js, drmaa2_jinfo filter)
{
    drmaa2_dict sql_filter = drmaa2_dict_create(NULL);
    char *session_id_c = NULL; // only used if js != NULL

    char *exit_status_c         = NULL;
    char *job_state_c           = NULL;
    char *slots_c               = NULL;
    //char *wallclock_time_c      = NULL;       wallclock time  is ignored right now
    //char *cpu_time_c            = NULL;       cpu time        is ignored right now
    char submission_time_c[32];
    char dispatch_time_c[32];
    char finish_time_c[32];

    if (js != NULL) {
        asprintf(&session_id_c, "%d", js->id);
        drmaa2_dict_set(sql_filter, "session_name", js->name);
        drmaa2_dict_set(sql_filter, "session_id", session_id_c);
    }


    if (filter != NULL) {
        if (filter->jobId != DRMAA2_UNSET_STRING)
            drmaa2_dict_set(sql_filter, "rowid", filter->jobId);
        if (filter->exitStatus != DRMAA2_UNSET_NUM) {
            asprintf(&exit_status_c, "%d", filter->exitStatus);
            drmaa2_dict_set(sql_filter, "exit_status", exit_status_c);
        }
        if (filter->terminatingSignal != DRMAA2_UNSET_STRING)
            drmaa2_dict_set(sql_filter, "terminating_signal", filter->terminatingSignal);
        // annotation is ignored (according to the spec)
        if (filter->jobState != DRMAA2_UNSET_ENUM) {
            asprintf(&job_state_c, "%d", filter->jobState);
            drmaa2_dict_set(sql_filter, "job_state", job_state_c);
        }
        if (filter->jobSubState != DRMAA2_UNSET_STRING)
            drmaa2_dict_set(sql_filter, "job_sub_state", filter->jobSubState);
        // allocatedMachines is ignored in the mock implementation
        if (filter->submissionMachine != DRMAA2_UNSET_STRING)
            drmaa2_dict_set(sql_filter, "submission_machine", filter->submissionMachine);
        if (filter->jobOwner != DRMAA2_UNSET_STRING)
            drmaa2_dict_set(sql_filter, "job_owner", filter->jobOwner);
        if (filter->slots != DRMAA2_UNSET_NUM) {
            asprintf(&slots_c, "%lld", filter->slots);
            drmaa2_dict_set(sql_filter, "slots", slots_c);
        }
        if (filter->queueName != DRMAA2_UNSET_STRING)
            drmaa2_dict_set(sql_filter, "queue_name", filter->queueName);

        if (filter->submissionTime != DRMAA2_UNSET_TIME) {
            strftime(submission_time_c, 32, "%Y-%m-%d %H:%M:%S", gmtime(&filter->submissionTime));
            drmaa2_dict_set(sql_filter, "submission_time", submission_time_c);
        }
        if (filter->dispatchTime != DRMAA2_UNSET_TIME) {
            strftime(dispatch_time_c, 32, "%Y-%m-%d %H:%M:%S", gmtime(&filter->dispatchTime));
            drmaa2_dict_set(sql_filter, "dispatch_time", dispatch_time_c);
        }
        if (filter->finishTime != DRMAA2_UNSET_TIME) {
            strftime(finish_time_c, 32, "%Y-%m-%d %H:%M:%S", gmtime(&filter->finishTime));
            drmaa2_dict_set(sql_filter, "finish_time", finish_time_c);  
        }
    }
        
    drmaa2_string query = build_query_string("SELECT session_name, rowid FROM jobs", sql_filter);
    int rc = drmaa2_db_query(query, get_jobs_callback, jobs);
    drmaa2_string_free(&query);

    drmaa2_string_free(&session_id_c);

    drmaa2_string_free(&job_state_c);
    drmaa2_string_free(&slots_c);
    //drmaa2_string_free(&wallclock_time_c);
    //drmaa2_string_free(&cpu_time_c);

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
    char *stmt = sqlite3_mprintf("DELETE FROM reservation_sessions WHERE name = %Q", session_name);
//    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE;\
//    DELETE FROM reservation_sessions WHERE name = %Q;\
//    DELETE FROM reservation_templates WHERE session_name = %Q;\
//    DELETE FROM reservations WHERE session_name = %Q;\
//    COMMIT;", session_name, session_name, session_name);
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


long long save_reservation(const char *session_name, long long template_id, const char *name)
{
    char *stmt = sqlite3_mprintf("INSERT INTO reservations VALUES(%Q, %lld, %Q, %Q, %Q, %Q, %Q, %Q)", session_name, template_id, NULL, NULL, NULL, NULL, NULL, NULL);
    sqlite3_int64 row_id = drmaa2_db_query_rowid(stmt);
    sqlite3_free(stmt);
    stmt = sqlite3_mprintf("BEGIN EXCLUSIVE; UPDATE reservations\
        SET reservation_name = \'%s%d\' WHERE rowid = %lld; COMMIT;", name, row_id, row_id);
    int rc = drmaa2_db_query(stmt, NULL, NULL);
    sqlite3_free(stmt);

    return row_id;
}


long long save_rtemplate(drmaa2_rtemplate rt, const char *session_name)
{
    //TODO: save string lists
    char *stmt = sqlite3_mprintf("INSERT INTO reservation_templates VALUES(%Q, %Q, %d, %d, %d, %lld, %lld, %Q, %Q, %Q, %lld, %d, %d)", 
                    session_name, rt->reservationName, rt->startTime, rt->endTime, rt->duration, rt->minSlots, rt->maxSlots,
                    rt->jobCategory, NULL, NULL, rt->minPhysMemory, rt->machineOS, rt->machineArch);
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
    long long row_id = atoll(reservationId);
    char *stmt = sqlite3_mprintf("SELECT session_name FROM reservations WHERE rowid = %lld", row_id);
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


static int drmaa2_get_rtemplate_callback(drmaa2_rtemplate rt, int argc, char **argv, char **azColName)
{
    //TODO: save string lists
    assert(argc == 12);
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "reservation_name") && argv[i] != NULL)
            rt->reservationName = strdup(argv[i]);
        else if (!strcmp(azColName[i], "min_slots") && argv[i] != NULL)
            rt->minSlots = atoi(argv[i]);
        else if (!strcmp(azColName[i], "max_slots") && argv[i] != NULL)
            rt->maxSlots = atoi(argv[i]);
        else if (!strcmp(azColName[i], "job_category") && argv[i] != NULL)
            rt->jobCategory = strdup(argv[i]);
        else if (!strcmp(azColName[i], "users_ACL") && argv[i] != NULL)
            rt->usersACL = DRMAA2_UNSET_LIST;
        else if (!strcmp(azColName[i], "candidate_machines") && argv[i] != NULL)
            rt->candidateMachines = DRMAA2_UNSET_LIST;
        else if (!strcmp(azColName[i], "min_phys_memory") && argv[i] != NULL)
            rt->minPhysMemory = atoi(argv[i]);
        else if (!strcmp(azColName[i], "machine_os") && argv[i] != NULL)
            rt->machineOS = atoi(argv[i]);
        else if (!strcmp(azColName[i], "machine_arch") && argv[i] != NULL)
            rt->machineArch = atoi(argv[i]);
        else if ((argv[i] != NULL) && 
    (!strcmp(azColName[i], "start_time") || !strcmp(azColName[i], "end_time") || !strcmp(azColName[i], "duration")) )
        {
            struct tm tm;
            time_t epoch;
            // convert time string to unix time stamp
            if (atoi(argv[i]) == DRMAA2_UNSET_TIME) {
                epoch = DRMAA2_UNSET_TIME;
            }
            else if ( strptime(argv[i], "%Y-%m-%d %H:%M:%S", &tm) != 0 ) {
                epoch = mktime(&tm);
            }
            else {
                printf("time conversion error\n");
                exit(1);
            }

            if (!strcmp(azColName[i], "start_time"))
                rt->startTime = epoch;
            if (!strcmp(azColName[i], "reserved_end_time"))
                rt->endTime = epoch;
            if (!strcmp(azColName[i], "duration"))
                rt->duration = epoch;
        }
    }
    DEBUG_PRINT("\n");
    return 0;
}

drmaa2_rtemplate drmaa2_get_rtemplate(drmaa2_rtemplate rt, const char *reservationId)
{
    long long row_id = atoll(reservationId);
    char *stmt = sqlite3_mprintf("SELECT reservation_name, start_time, end_time, duration, min_slots, max_slots,\
                job_category, users_ACL, candidate_machines, min_phys_memory, machine_os, machine_arch\
        FROM reservation_templates WHERE rowid = %lld", row_id);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)drmaa2_get_rtemplate_callback, rt);
    sqlite3_free(stmt);
    if (rc != SQLITE_OK) return NULL;
    return rt;
}


static int drmaa2_get_rinfo_callback(drmaa2_rinfo ri, int argc, char **argv, char **azColName)
{
    assert(argc == 6);
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "reservation_name") && argv[i] != NULL)
            ri->reservationName = strdup(argv[i]);
        else if (!strcmp(azColName[i], "users_ACL") && argv[i] != NULL)
            ri->usersACL = DRMAA2_UNSET_LIST;
        else if (!strcmp(azColName[i], "reserved_slots") && argv[i] != NULL)
            ri->reservedSlots = atoi(argv[i]);
        else if ((argv[i] != NULL) && 
    (!strcmp(azColName[i], "reserved_start_time") || !strcmp(azColName[i], "reserved_end_time") ) )
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

            if (!strcmp(azColName[i], "reserved_start_time"))
                ri->reservedStartTime = epoch;
            if (!strcmp(azColName[i], "reserved_end_time"))
                ri->reservedEndTime = epoch;
        }
    }
    DEBUG_PRINT("\n");
    return 0;
}

drmaa2_rinfo drmaa2_get_rinfo(drmaa2_rinfo ri)
{
    long long row_id = atoll(ri->reservationId);
    char *stmt = sqlite3_mprintf("SELECT reservation_name, reserved_start_time, reserved_end_time, users_ACL, reserved_slots, reserved_machines\
        FROM reservations WHERE rowid = %lld", row_id);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)drmaa2_get_rinfo_callback, ri);
    sqlite3_free(stmt);
    if (rc != SQLITE_OK) return NULL;
    return ri;
}



static int get_status_callback(int *status, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    if (argv[0] != NULL)
    {
        *status = atoi(argv[0]);
    }
    // status stays -1
    return 0;
}

int drmaa2_get_job_status(drmaa2_j j)
{
    int status = -1;
    long long row_id = atoll(j->id);
    char *stmt = sqlite3_mprintf("SELECT exit_status FROM jobs WHERE rowid = %lld", row_id);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)get_status_callback, &status);
    sqlite3_free(stmt);
    return status;
}


static int drmaa2_get_job_template_callback(drmaa2_jtemplate jt, int argc, char **argv, char **azColName)
{
    //TODO: save string lists
    assert(argc == 30);
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "remote_command") && argv[i] != NULL)
            jt->remoteCommand = strdup(argv[i]);
        else if (!strcmp(azColName[i], "submit_as_hold") && argv[i] != NULL)
            jt->submitAsHold = atoi(argv[i]);
        else if (!strcmp(azColName[i], "rerunnable") && argv[i] != NULL)
            jt->rerunnable = atoi(argv[i]);
        else if (!strcmp(azColName[i], "job_environment") && argv[i] != NULL)
            jt->jobEnvironment = DRMAA2_UNSET_DICT;
        else if (!strcmp(azColName[i], "working_directory") && argv[i] != NULL)
            jt->workingDirectory = strdup(argv[i]);
        else if (!strcmp(azColName[i], "job_category") && argv[i] != NULL)
            jt->jobCategory = strdup(argv[i]);
        else if (!strcmp(azColName[i], "email") && argv[i] != NULL)
            jt->email = DRMAA2_UNSET_LIST;
        else if (!strcmp(azColName[i], "email_on_started") && argv[i] != NULL)
            jt->emailOnStarted = atoi(argv[i]);
        else if (!strcmp(azColName[i], "email_on_terminated") && argv[i] != NULL)
            jt->emailOnTerminated = atoi(argv[i]);
        else if (!strcmp(azColName[i], "job_name") && argv[i] != NULL)
            jt->jobName = strdup(argv[i]);
        else if (!strcmp(azColName[i], "input_path") && argv[i] != NULL)
            jt->inputPath = strdup(argv[i]);
        else if (!strcmp(azColName[i], "output_path") && argv[i] != NULL)
            jt->outputPath = strdup(argv[i]);
        else if (!strcmp(azColName[i], "error_path") && argv[i] != NULL)
            jt->errorPath = strdup(argv[i]);
        else if (!strcmp(azColName[i], "join_files") && argv[i] != NULL)
            jt->joinFiles = atoi(argv[i]);
        else if (!strcmp(azColName[i], "reservation_id") && argv[i] != NULL)
            jt->reservationId = strdup(argv[i]);
        else if (!strcmp(azColName[i], "queue_name") && argv[i] != NULL)
            jt->queueName = strdup(argv[i]);
        else if (!strcmp(azColName[i], "min_slots") && argv[i] != NULL)
            jt->minSlots = atoi(argv[i]);
        else if (!strcmp(azColName[i], "max_slots") && argv[i] != NULL)
            jt->maxSlots = atoi(argv[i]);
        else if (!strcmp(azColName[i], "priority") && argv[i] != NULL)
            jt->priority = atoi(argv[i]);
        else if (!strcmp(azColName[i], "candidate_machines") && argv[i] != NULL)
            jt->candidateMachines = DRMAA2_UNSET_LIST;
        else if (!strcmp(azColName[i], "min_phys_memory") && argv[i] != NULL)
            jt->minPhysMemory = atoi(argv[i]);
        else if (!strcmp(azColName[i], "machine_os") && argv[i] != NULL)
            jt->machineOS = atoi(argv[i]);
        else if (!strcmp(azColName[i], "machine_arch") && argv[i] != NULL)
            jt->machineArch = atoi(argv[i]);
        else if (!strcmp(azColName[i], "stage_in_files") && argv[i] != NULL)
            jt->jobEnvironment = DRMAA2_UNSET_DICT;
        else if (!strcmp(azColName[i], "stage_out_files") && argv[i] != NULL)
            jt->jobEnvironment = DRMAA2_UNSET_DICT;
        else if (!strcmp(azColName[i], "resource_limits") && argv[i] != NULL)
            jt->resourceLimits = DRMAA2_UNSET_DICT;
        else if (!strcmp(azColName[i], "accounting_id") && argv[i] != NULL)
            jt->accountingId = strdup(argv[i]);

        else if ((argv[i] != NULL) && 
    (!strcmp(azColName[i], "start_time") || !strcmp(azColName[i], "deadline_time") ) )
        {
            struct tm tm;
            time_t epoch;
            // convert time string to unix time stamp
            if (atoi(argv[i]) == DRMAA2_UNSET_TIME) {
                epoch = DRMAA2_UNSET_TIME;
            }
            else if ( strptime(argv[i], "%Y-%m-%d %H:%M:%S", &tm) != 0 ) {
                epoch = mktime(&tm);
            }
            else {
                printf("time conversion error\n");
                exit(1);
            }

            if (!strcmp(azColName[i], "start_time"))
                jt->startTime = epoch;
            if (!strcmp(azColName[i], "deadline_time"))
                jt->deadlineTime = epoch;
        }
    }
    DEBUG_PRINT("\n");
    return 0;
}

drmaa2_jtemplate drmaa2_get_job_template(drmaa2_jtemplate jt, const char *jobId)
{
    long long row_id = atoll(jobId);
    char *stmt = sqlite3_mprintf("SELECT remote_command, args, submit_as_hold, rerunnable, job_environment, \
        working_directory, job_category, email, email_on_started, email_on_terminated, job_name, input_path, \
        output_path, error_path, join_files, reservation_id, queue_name, min_slots, max_slots, priority, \
        candidate_machines, min_phys_memory, machine_os, machine_arch, start_time, deadline_time, \
        stage_in_files, stage_out_files, resource_limits, accounting_id \
        FROM job_templates WHERE rowid = %lld", row_id);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)drmaa2_get_job_template_callback, jt);
    sqlite3_free(stmt);
    if (rc != SQLITE_OK) return NULL;
    return jt; 
}


static int get_jobinfo_callback(drmaa2_jinfo ji, int argc, char **argv, char **azColName)
{
    assert(argc == 19);
    int i;
    for(i=0; i<argc; i++)
    {
        DEBUG_PRINT("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "exit_status") && argv[i] != NULL)
            ji->exitStatus = atoi(argv[i]);
        else if (!strcmp(azColName[i], "terminating_signal") && argv[i] != NULL)
            ji->terminatingSignal = strdup(argv[i]);
        else if (!strcmp(azColName[i], "annotation") && argv[i] != NULL)
            ji->annotation = strdup(argv[i]);
        else if (!strcmp(azColName[i], "job_state") && argv[i] != NULL)
            ji->jobState = atoi(argv[i]);
        else if (!strcmp(azColName[i], "job_sub_state") && argv[i] != NULL)
            ji->jobSubState = strdup(argv[i]);
        else if (!strcmp(azColName[i], "allocated_machines") && argv[i] != NULL)
            ji->allocatedMachines = string_split(argv[i], '|');
        else if (!strcmp(azColName[i], "submission_machine") && argv[i] != NULL)
            ji->submissionMachine = strdup(argv[i]);
        else if (!strcmp(azColName[i], "job_owner") && argv[i] != NULL)
            ji->jobOwner = strdup(argv[i]);
        else if (!strcmp(azColName[i], "slots") && argv[i] != NULL)
            ji->slots = atoll(argv[i]);
        else if (!strcmp(azColName[i], "queue_name") && argv[i] != NULL)
            ji->queueName = strdup(argv[i]);
        else if (!strcmp(azColName[i], "wallclock_time") && argv[i] != NULL)
            ji->wallclockTime = atoll(argv[i]);
        else if (!strcmp(azColName[i], "cpu_time") && argv[i] != NULL)
            ji->cpuTime = atoll(argv[i]);
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
    long long row_id = atoll(ji->jobId);
    char *stmt = sqlite3_mprintf("SELECT * FROM jobs WHERE rowid = %lld", row_id);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)get_jobinfo_callback, ji);
    sqlite3_free(stmt);
    if (rc != SQLITE_OK) return NULL;
    return ji;
}


static int get_pid_callback(pid_t *pid, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    if (argv[0] == NULL)
    {
        printf("PID is not yet set.\n");
    }
    *pid = atoi(argv[0]);
    return 0;
}

pid_t get_job_pid(drmaa2_j j)
{
    pid_t pid;
    long long row_id = atoll(j->id);
    char *stmt = sqlite3_mprintf("SELECT pid FROM jobs WHERE rowid = %lld", row_id);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)get_pid_callback, &pid);
    sqlite3_free(stmt);
    return pid;
}


static int get_state_callback(int *state, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    *state = atoi(argv[0]);
    return 0;
}

int get_state(drmaa2_j j)
{
    int state;
    long long row_id = atoll(j->id);
    char *stmt = sqlite3_mprintf("SELECT job_state FROM jobs WHERE rowid = %lld", row_id);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)get_pid_callback, &state);
    sqlite3_free(stmt);
    return state;
}


int save_state(drmaa2_j j, drmaa2_jstate state)
{
    long long row_id = atoll(j->id);
    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE; UPDATE jobs\
        SET job_state = %d WHERE rowid = %lld; COMMIT;", state, row_id);
    int rc = drmaa2_db_query(stmt, NULL, NULL);
    sqlite3_free(stmt);
    return rc;
}


long long save_jarray(const char *session_name, long long template_id, drmaa2_string_list sl)
{
    drmaa2_string string = string_join(sl, '|');
    char *stmt = sqlite3_mprintf("BEGIN EXCLUSIVE; INSERT INTO job_arrays VALUES (%Q, %lld, %Q); COMMIT;",
        session_name, template_id, string);
    sqlite3_int64 row_id = drmaa2_db_query_rowid(stmt);
    sqlite3_free(stmt);
    drmaa2_string_free(&string);
    return row_id;
}


static int jarray_exists_callback(int *exists, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    DEBUG_PRINT("%s = %s\n", azColName[0], argv[0] ? argv[0] : "NULL");
    *exists = atoi(argv[0]);
    return 0;
}

int jarray_exists(const char *session_name, const char *jobarrayId)
{
    long long row_id = atoll(jobarrayId);
    char *stmt = sqlite3_mprintf("SELECT EXISTS(SELECT 1 FROM job_arrays \
        WHERE session_name = %Q AND rowid = %lld LIMIT 1);", session_name, row_id);
    int exists = 0;
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)jarray_exists_callback, &exists);
    sqlite3_free(stmt);
    return exists;
}

static int get_jobs_of_jarray_callback(char **jobs, int argc, char **argv, char **azColName)
{
    assert(argc == 1);
    DEBUG_PRINT("%s = %s\n", azColName[0], argv[0] ? argv[0] : "NULL");
    *jobs = strdup(argv[0]);
    return 0;
}

drmaa2_j_list get_jobs_of_jarray(drmaa2_jarray ja)
{
    long long row_id = atoll(ja->id);
    char *stmt = sqlite3_mprintf("SELECT jobs FROM job_arrays \
        WHERE rowid = %lld;", row_id);
    char *jobs = NULL;
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)get_jobs_of_jarray_callback, &jobs);
    sqlite3_free(stmt);
    drmaa2_string_list sl = string_split(jobs, '|');
    drmaa2_j_list jl = (drmaa2_j_list)drmaa2_list_create(DRMAA2_JOBLIST, (drmaa2_list_entryfree)drmaa2_j_free);
    drmaa2_j j;
    size_t i;
    for (i = 0; i < drmaa2_list_size(sl); i++)
    {
        j = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
        j->id = strdup(drmaa2_list_get(sl, i));
        j->session_name = strdup(ja->session_name);
        drmaa2_list_add(jl, j);
    }
    return jl;
}

drmaa2_jtemplate drmaa2_get_jobarray_template(drmaa2_jtemplate jt, const char *jobarrayId)
{
    long long row_id = atoll(jobarrayId);
    printf("id: %lld\n", row_id);
    char *stmt = sqlite3_mprintf("SELECT remote_command, args, submit_as_hold, rerunnable, job_environment, \
        working_directory, job_category, email, email_on_started, email_on_terminated, job_name, input_path, \
        output_path, error_path, join_files, reservation_id, queue_name, min_slots, max_slots, priority, \
        candidate_machines, min_phys_memory, machine_os, machine_arch, start_time, deadline_time, \
        stage_in_files, stage_out_files, resource_limits, accounting_id \
        FROM job_templates, job_arrays WHERE job_arrays.rowid = %lld AND job_templates.rowid = job_arrays.template_id", row_id);
    int rc = drmaa2_db_query(stmt, (sqlite3_callback)drmaa2_get_job_template_callback, jt);
    sqlite3_free(stmt);
    if (rc != SQLITE_OK)
        return NULL;
    return jt; 
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
    char *stmt = sqlite3_mprintf("SELECT remote_command FROM jobs, job_templates \
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

