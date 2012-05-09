#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "drmaa2.h"


int main ()
{
    printf("======================= TEST_JSESSION ==========================\n");
    drmaa2_error error_code;

    drmaa2_jsession js = drmaa2_create_jsession("mysession", NULL);
    
    // test getter functions
    char *name = drmaa2_jsession_get_session_name(js);
    assert(strcmp(name, "mysession") == 0);
    drmaa2_string_free(name);
    char *contact = drmaa2_jsession_get_contact(js);
    assert(contact == DRMAA2_UNSET_STRING);
    drmaa2_string_free(contact);


    //test empty joblist
    drmaa2_j_list jobs = drmaa2_jsession_get_jobs(js, NULL);
    assert(drmaa2_list_size(jobs) == 0);
    drmaa2_list_free(jobs);

    
    //test joblist
    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    jt->remoteCommand = strdup("/bin/date");
    drmaa2_j j1 = drmaa2_jsession_run_job(js, jt);
    drmaa2_j j2 = drmaa2_jsession_run_job(js, jt);

    jobs = drmaa2_jsession_get_jobs(js, NULL);
    assert(drmaa2_list_size(jobs) == 2);
    drmaa2_list_free(jobs);

    drmaa2_j_wait_terminated(j1, DRMAA2_INFINITE_TIME);
    drmaa2_j_wait_terminated(j2, DRMAA2_INFINITE_TIME);
    drmaa2_jtemplate_free(jt);



    drmaa2_destroy_jsession("mysession");




    

    printf("===================FINISHED TEST_JSESSION ======================\n");
}

