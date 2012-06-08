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
