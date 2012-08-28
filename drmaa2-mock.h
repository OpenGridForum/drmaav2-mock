#ifndef DRMAA2_MOCK_H
#define DRMAA2_MOCK_H


typedef struct drmaa2_jsession_s
{
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

typedef struct drmaa2_r_s
{
    drmaa2_string id;
    drmaa2_string session_name;
    //drmaa2_rtemplate template;
    //drmaa2_rinfo info;
} drmaa2_r_s;


#endif 
