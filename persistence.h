#ifndef DRMAA2_PERSISTENCE_H
#define DRMAA2_PERSISTENCE_H

#include "drmaa2.h"

#define DB_NAME "drmaa.sqlite"


int drmaa2_setup_db(char *name);

int drmaa2_reset_db(char *name);




int save_jsession(char *db_name, const char *contact, const char *session);

int delete_jsession(char *db_name, const char *session_name);

drmaa2_jsession get_jsession(char *db_name, const char *session_name);

long long save_job(char *db_name, const char *session_name, long long template_id);

long long save_jtemplate(char *db_name, drmaa2_jtemplate jt);

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


#endif DRMAA2_PERSISTENCE_H
