#ifndef DRMAA2_PERSISTENCE_H
#define DRMAA2_PERSISTENCE_H

#include "drmaa2.h"
//#include <sys/types.h>
//#include <sys/wait.h>
//#include <unistd.h>



typedef struct drmaa2_jsession_s
{
    const char *contact;
    const char *name;
    drmaa2_j_list jobs;
} drmaa2_jsession_s;

typedef struct drmaa2_rsession_s
{
    const char *contact;
    const char *name;
    drmaa2_r_list reservations;
} drmaa2_rsession_s;

typedef struct drmaa2_msession_s
{
    const char *name;
} drmaa2_msession_s;

typedef struct drmaa2_j_s
{
    const char *id;
    const char *session_name;
    //pid_t pid;
    //drmaa2_jtemplate template;
    //drmaa2_jinfo info;
} drmaa2_j_s;

typedef struct drmaa2_r_s
{
    const char *id;
    const char *session_name;
    drmaa2_rtemplate template;
    drmaa2_rinfo info;
} drmaa2_r_s;



int save_jsession(char *db_name, const char *contact, const char *session);

int delete_jsession(char *db_name, const char *session_name);

drmaa2_jsession get_jsession(char *db_name, const char *session_name);

long long save_job(char *db_name, const char *session_name, long long template_id);

long long save_jtemplate(char *db_name, drmaa2_jtemplate jt);

drmaa2_string_list get_jsession_names(char *db_name, drmaa2_string_list session_names);

drmaa2_j_list get_jobs(char *db_name, const char *session_name, drmaa2_j_list jobs);



#endif DRMAA2_PERSISTENCE_H
