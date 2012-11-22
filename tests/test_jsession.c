#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../drmaa2.h"
#include "../drmaa2-list.h"
#include "test_jsession.h"


void test_jsession_invalidation()
{
    drmaa2_jsession js = drmaa2_create_jsession("mysession", NULL);
    drmaa2_destroy_jsession("mysession");
    CU_ASSERT_PTR_NULL(drmaa2_jsession_get_contact(js));
    CU_ASSERT_PTR_NULL(drmaa2_jsession_get_session_name(js));
    CU_ASSERT_PTR_NULL(drmaa2_jsession_get_jobs(js, NULL));
    CU_ASSERT_PTR_NULL(drmaa2_jsession_get_job_categories(js));
    CU_ASSERT_EQUAL(drmaa2_lasterror(), DRMAA2_INVALID_SESSION);
    drmaa2_jsession_free(&js);
}


void test_jsession_getter_methods()
{
    drmaa2_jsession js = drmaa2_create_jsession("mysession", NULL);
    
    // test getter functions
    drmaa2_string name = drmaa2_jsession_get_session_name(js);
    CU_ASSERT_STRING_EQUAL(name, "mysession");
    drmaa2_string_free(&name);
    drmaa2_string contact = drmaa2_jsession_get_contact(js);
    CU_ASSERT_PTR_NULL(contact);
    drmaa2_string_free(&contact);

    drmaa2_destroy_jsession("mysession");
    drmaa2_jsession_free(&js);
}


void test_job_list()
{
    drmaa2_jsession js = drmaa2_create_jsession("mysession", NULL);

    //test empty joblist
    drmaa2_j_list jobs = drmaa2_jsession_get_jobs(js, NULL);
    CU_ASSERT_EQUAL(drmaa2_list_size(jobs), 0);
    drmaa2_list_free(&jobs);
    
    //test joblist
    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    jt->remoteCommand = strdup("/bin/date");
    drmaa2_j j1 = drmaa2_jsession_run_job(js, jt);
    drmaa2_j j2 = drmaa2_jsession_run_job(js, jt);

    jobs = drmaa2_jsession_get_jobs(js, NULL);
    CU_ASSERT_EQUAL(drmaa2_list_size(jobs), 2);
    drmaa2_list_free(&jobs);

    drmaa2_j_wait_terminated(j1, DRMAA2_INFINITE_TIME);
    drmaa2_j_wait_terminated(j2, DRMAA2_INFINITE_TIME);

    drmaa2_j_free(&j1);
    drmaa2_j_free(&j2);

    drmaa2_jtemplate_free(&jt);
    drmaa2_destroy_jsession("mysession");
    drmaa2_jsession_free(&js);
}


void test_job_categories()
{
    drmaa2_jsession js = drmaa2_create_jsession("mysession", NULL);

    drmaa2_string_list jc = drmaa2_jsession_get_job_categories(js);
    drmaa2_list_free(&jc);

    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    jt->remoteCommand = strdup("/bin/date");
    jt->jobCategory = strdup("non-existing jobCategory");
    drmaa2_j j3 = drmaa2_jsession_run_job(js, jt);
    CU_ASSERT_PTR_NULL(j3);
    CU_ASSERT_EQUAL(drmaa2_lasterror(), DRMAA2_INVALID_ARGUMENT);
    drmaa2_jtemplate_free(&jt);
    drmaa2_destroy_jsession("mysession");
    drmaa2_jsession_free(&js);

}


void test_wait_any_terminated()
{
    drmaa2_jsession js = drmaa2_create_jsession("mysession_termination", NULL);

    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    jt->remoteCommand = strdup("./a.out");
    drmaa2_j j1 = drmaa2_jsession_run_job(js, jt);
    printf("jobID %s\n", drmaa2_j_get_id(j1));
    drmaa2_string_free(&jt->remoteCommand);
    jt->remoteCommand = strdup("/bin/date");
    sleep(5);
    drmaa2_j j2 = drmaa2_jsession_run_job(js, jt);
    drmaa2_j j3 = drmaa2_jsession_run_job(js, jt);

    drmaa2_j_list jl = drmaa2_jsession_get_jobs(js, NULL);
    printf("JOB list size: %ld\n", drmaa2_list_size(jl));
    CU_ASSERT_EQUAL(drmaa2_list_size(jl), 3);

    drmaa2_j terminated_j = drmaa2_jsession_wait_any_terminated(js, jl, DRMAA2_INFINITE_TIME);
    CU_ASSERT_PTR_NOT_NULL(terminated_j);
    drmaa2_string j_id = drmaa2_j_get_id(terminated_j);
    printf("jobID %s\n", j_id);

    drmaa2_string_free(&j_id);
    drmaa2_j_free(&terminated_j);
    drmaa2_j_free(&j1);
    drmaa2_j_free(&j2);
    drmaa2_j_free(&j3);

    drmaa2_list_free(&jl);
    drmaa2_jtemplate_free(&jt);
    drmaa2_destroy_jsession("mysession_termination");
    drmaa2_jsession_free(&js);


}





