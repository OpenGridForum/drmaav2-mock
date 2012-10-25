#ifndef DRMAA2_PERSISTENCE_H
#define DRMAA2_PERSISTENCE_H

#include "drmaa2.h"
#include <unistd.h>
#include "config.h"

#define DB_NAME DRMAA_DBFILE


// job session methods
int save_jsession(const char *contact, const char *session);
int delete_jsession(const char *session_name);
drmaa2_jsession get_jsession(const char *session_name);

int drmaa2_jsession_is_valid(const char *session_name);
int drmaa2_rsession_is_valid(const char *session_name);

long long save_job(const char *session_name, long long template_id);
long long save_jtemplate(drmaa2_jtemplate jt, const char *session_name);

drmaa2_string_list get_jsession_names(drmaa2_string_list session_names);
drmaa2_j_list get_session_jobs(drmaa2_j_list jobs, const char *session_name);

drmaa2_j_list get_jobs(drmaa2_j_list jobs, drmaa2_jinfo filter);


// reservation session methods
int save_rsession(const char *contact, const char *session);
int delete_rsession(const char *session_name);
drmaa2_rsession get_rsession(const char *session_name);

long long save_reservation(const char *session_name, long long template_id, const char *name);
long long save_rtemplate(drmaa2_rtemplate rt, const char *session_name);

drmaa2_r drmaa2_get_reservation(const drmaa2_string reservationId);

drmaa2_string_list get_rsession_names(drmaa2_string_list session_names);
drmaa2_r_list drmaa2_get_session_reservations(drmaa2_r_list reservations, const char *session_name);

drmaa2_r_list get_reservations(drmaa2_r_list reservations);


drmaa2_rtemplate drmaa2_get_rtemplate(drmaa2_rtemplate rt, const char *reservationId);
drmaa2_rinfo drmaa2_get_rinfo(drmaa2_rinfo ri);


int drmaa2_get_job_status(drmaa2_j j);

drmaa2_jtemplate drmaa2_get_job_template(drmaa2_jtemplate jt, const char *jobId);

drmaa2_jinfo get_job_info(drmaa2_jinfo ji);

int drmaa2_setup_db(char *name);

int drmaa2_reset_db(char *name);


pid_t get_job_pid(drmaa2_j j);

int get_state(drmaa2_j j);

int save_state(drmaa2_j j, drmaa2_jstate state);
int save_state_id(long long row_id, drmaa2_jstate state);


long long save_jarray(const char *session_name, long long template_id, drmaa2_string_list sl);
int jarray_exists(const char *session_name, const char *jobarrayId);




//queries for wrapper

char *get_command(long long row_id);

int drmaa2_save_pid(long long row_id, pid_t pid);

int drmaa2_save_exit_status(long long row_id, int status);



#endif 
