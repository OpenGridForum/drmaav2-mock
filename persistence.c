#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sqlite3.h>
#include "persistence.h"


sqlite3 *open_db(char *name)
{
	sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    rc = sqlite3_open(name, &db);
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
    return ;
}



int save_jsession(char *db_name, const char *contact, const char *session_name)
{
    sqlite3 *db = open_db(db_name);
    char *zErrMsg = 0;

    char *stmt = sqlite3_mprintf("INSERT INTO job_sessions VALUES(%Q, %Q)", contact, session_name);
    printf("%s\n", stmt);
    int rc = sqlite3_exec(db, stmt, NULL, 0, &zErrMsg);
    sqlite3_free(stmt);

    evaluate_result_code(rc, zErrMsg);   
    sqlite3_close(db);
    return 1;
}

int delete_jsession(char *db_name, const char *session_name)
{
    sqlite3 *db = open_db(db_name);
    char *zErrMsg = 0;
    
    char *stmt = sqlite3_mprintf("DELETE FROM job_sessions WHERE name = %Q", session_name);
    printf("%s\n", stmt);
    int rc = sqlite3_exec(db, stmt, NULL, 0, &zErrMsg);
    sqlite3_free(stmt);

	evaluate_result_code(rc, zErrMsg); 
    
    sqlite3_close(db);
    return 1;
    //TODO: check, whether session with given name exists
}



static int get_jsession_callback(void **ptr, int argc, char **argv, char **azColName)
{
	assert(argc == 2);
	drmaa2_jsession js = (drmaa2_jsession)malloc(sizeof(drmaa2_jsession_s));
    int i;
    for(i=0; i<argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "name"))
        	js->name = strdup(argv[i]);
        else
        	js->contact = argv[i] ? strdup(argv[i]) : NULL;
    }
    printf("\n");
    *ptr = js;
    return 0;
}

drmaa2_jsession get_jsession(char *db_name, const char *session_name)
{
	drmaa2_jsession js = NULL;

    sqlite3 *db = open_db(db_name);
    char *zErrMsg = 0;

    char *stmt = sqlite3_mprintf("SELECT * FROM job_sessions WHERE name = %Q", session_name);
    printf("%s\n", stmt);
    int rc = sqlite3_exec(db, stmt, get_jsession_callback, &js, &zErrMsg);
    sqlite3_free(stmt);

	evaluate_result_code(rc, zErrMsg);

    if (js == NULL)
    {
    	printf("no such jobsession\n");
    }

    sqlite3_close(db);
    return js;
}


long long save_job(char *db_name, const char *session_name, long long template_id)
{
    sqlite3 *db = open_db(db_name);
    char *zErrMsg = 0;

    char *stmt = sqlite3_mprintf("INSERT INTO jobs VALUES(%Q, %Q, %lld, %Q)", NULL, session_name, template_id, NULL);
    printf("%s\n", stmt);
    int rc = sqlite3_exec(db, stmt, NULL, 0, &zErrMsg);
    sqlite3_int64 row_id = sqlite3_last_insert_rowid(db);
    sqlite3_free(stmt);

    evaluate_result_code(rc, zErrMsg);
    sqlite3_close(db);

    return row_id;
}


long long save_jtemplate(char *db_name, drmaa2_jtemplate jt)
{
	sqlite3 *db = open_db(db_name);
    char *zErrMsg = 0;

    char *stmt = sqlite3_mprintf("INSERT INTO job_templates VALUES(%Q, %Q)", jt->remoteCommand, jt->args);
    printf("%s\n", stmt);
    int rc = sqlite3_exec(db, stmt, NULL, 0, &zErrMsg);
    sqlite3_int64 row_id = sqlite3_last_insert_rowid(db);
    sqlite3_free(stmt);

    evaluate_result_code(rc, zErrMsg);
    sqlite3_close(db);

    return row_id;
}


static int get_jsession_names_callback(void *session_names, int argc, char **argv, char **azColName)
{
	assert(argc == 1);
    printf("%s = %s\n", azColName[0], argv[0] ? argv[0] : "NULL");

    drmaa2_string_list names = (drmaa2_string_list)session_names;
    drmaa2_list_add(names, strdup(argv[0]));
    printf("\n");
    return 0;
}


drmaa2_string_list get_jsession_names(char *db_name, drmaa2_string_list session_names)
{
	sqlite3 *db = open_db(db_name);
    char *zErrMsg = 0;

    char stmt[] = "SELECT name FROM job_sessions";
    printf("%s\n", stmt);
    int rc = sqlite3_exec(db, stmt, get_jsession_names_callback, session_names, &zErrMsg);

    evaluate_result_code(rc, zErrMsg);
    sqlite3_close(db);

    return session_names;
}


static int get_jobs_callback(void *ptr, int argc, char **argv, char **azColName)
{
	assert(argc == 2);
	drmaa2_j j = (drmaa2_j)malloc(sizeof(drmaa2_j_s));
    int i;
    for(i=0; i<argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (!strcmp(azColName[i], "session_name"))
        	j->session_name = strdup(argv[i]);
        else
        	j->id = strdup(argv[i]);
    }
    printf("\n");
    drmaa2_list_add((drmaa2_j_list)ptr, j);
    return 0;
}

drmaa2_j_list get_jobs(char *db_name, const char *session_name, drmaa2_j_list jobs)
{
	sqlite3 *db = open_db(db_name);
    char *zErrMsg = 0;

    char *stmt = sqlite3_mprintf("SELECT session_name, row_id FROM jobs WHERE session_name = %Q", session_name);
    printf("%s\n", stmt);
    int rc = sqlite3_exec(db, stmt, get_jobs_callback, jobs, &zErrMsg);
    sqlite3_free(stmt);

    evaluate_result_code(rc, zErrMsg);
    sqlite3_close(db);
    
    return jobs;
}

