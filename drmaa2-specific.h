#ifndef DRMAA2_MOCK_H
#define DRMAA2_MOCK_H

#include "drmaa2.h"


// tool functions used both by the SPI and the API implementation

 void call_state_change_notification(drmaa2_j j, drmaa2_jstate state);

// implementation-specific drmaa inface types

typedef struct drmaa2_jsession_s
{
    int id;
    drmaa2_string contact;
    drmaa2_string name;
    drmaa2_j_list jobs;
} drmaa2_jsession_s;

typedef struct drmaa2_rsession_s
{
    drmaa2_string contact;
    drmaa2_string name;
    drmaa2_r_list reservations;
} drmaa2_rsession_s;

typedef struct drmaa2_msession_s
{
    drmaa2_string name;
} drmaa2_msession_s;

typedef struct drmaa2_j_s
{
    drmaa2_string id;
    drmaa2_string session_name;
} drmaa2_j_s;

typedef struct drmaa2_jarray_s
{
    drmaa2_string id;
    drmaa2_string session_name;
} drmaa2_jarray_s;

typedef struct drmaa2_r_s
{
    drmaa2_string id;
    drmaa2_string session_name;
} drmaa2_r_s;


drmaa2_string_list add_supported_job_categories(drmaa2_string_list jl);

drmaa2_j submit_job_to_DRMS(drmaa2_jsession js, long long job_id, drmaa2_jtemplate jt);

drmaa2_string drmaa2_generate_unique_name(char* prefix);

int drmaa2_random_int();


#endif 
