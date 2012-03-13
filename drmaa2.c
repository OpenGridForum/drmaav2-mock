#include "drmaa2.h"
#include <stdlib.h>
#include <stdio.h>



drmaa2_job_template_t  drmaa2_job_template_create(void){
    drmaa2_job_template_t job_template = (drmaa2_job_template_t)malloc(sizeof(drmaa2_job_template_s));
    return job_template;
}


drmaa2_error_t drmaa2_job_template_free(drmaa2_job_template_t jt){
    free(jt);
    return 0;
}


drmaa2_job_h drmaa2_js_run_job(const drmaa2_jobsession_h js, const drmaa2_job_template_t jt){
    printf("run %s\n", jt->remoteCommand);
    return NULL;
}


drmaa2_job_h drmaa2_j_wait_terminated(const drmaa2_job_h j, const time_t timeout){
    return NULL;
}


drmaa2_jobsession_h drmaa2_create_jobsession(const char * session_name, const char * contact){
    return NULL;
}


drmaa2_error_t drmaa2_destroy_jobsession(const char * session_name){
    return 0;
}