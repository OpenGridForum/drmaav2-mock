#ifndef DRMAA2_PERSISTENCE_H
#define DRMAA2_PERSISTENCE_H

#include "drmaa2.h"
#include <unistd.h>
#include "config.h"

#define DB_NAME DRMAA_DBFILE

int save_jsession(char *db_name, const char *contact, const char *session);

int delete_jsession(char *db_name, const char *session_name);

drmaa2_jsession get_jsession(char *db_name, const char *session_name);

int drmaa2_jsession_is_valid(const char *session_name);
int drmaa2_rsession_is_valid(const char *session_name);

long long save_job(char *db_name, const char *session_name, long long template_id);

long long save_jtemplate(char *db_name, drmaa2_jtemplate jt, const char *session_name);

drmaa2_string_list get_jsession_names(char *db_name, drmaa2_string_list session_names);

drmaa2_j_list get_session_jobs(char *db_name, drmaa2_j_list jobs, const char *session_name);

drmaa2_j_list get_jobs(char *db_name, drmaa2_j_list jobs, drmaa2_jinfo filter);


int save_rsession(char *db_name, const char *contact, const char *session);

int delete_rsession(char *db_name, const char *session_name);

drmaa2_rsession get_rsession(char *db_name, const char *session_name);

long long save_reservation(char *db_name, const char *session_name, long long template_id);

long long save_rtemplate(char *db_name, drmaa2_rtemplate rt);

drmaa2_string_list get_rsession_names(char *db_name, drmaa2_string_list session_names);

drmaa2_r_list get_reservations(char *db_name, drmaa2_r_list reservations);


int drmaa2_get_job_status(char *db_name, drmaa2_j j);

drmaa2_jinfo get_job_info(char *db_name, drmaa2_jinfo ji);

int drmaa2_setup_db(char *name);

int drmaa2_reset_db(char *name);



//queries for wrapper

char *get_command(char *db_name, long long row_id);

int drmaa2_save_pid(char *db_name, long long row_id, pid_t pid);

int drmaa2_save_exit_status(char *db_name, long long row_id, int status);


#endif 
