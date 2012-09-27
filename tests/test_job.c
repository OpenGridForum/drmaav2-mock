#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../drmaa2.h"
#include "../drmaa2-list.h"
#include "test_reservation.h"


void test_j_get_id_name()
{
    drmaa2_jsession js = drmaa2_create_jsession("my_job_session", NULL);

    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    jt->remoteCommand = strdup("/bin/date");
    drmaa2_j j = drmaa2_jsession_run_job(js, jt);
    drmaa2_jtemplate_free(&jt);

    drmaa2_string j_id = drmaa2_j_get_id(j);
    CU_ASSERT_PTR_NOT_NULL(j_id);
    drmaa2_string_free(&j_id);

    drmaa2_string j_sname = drmaa2_j_get_session_name(j);
    CU_ASSERT_STRING_EQUAL(j_sname, "my_job_session");
    drmaa2_string_free(&j_sname);

    drmaa2_j_wait_terminated(j, DRMAA2_INFINITE_TIME);
    drmaa2_j_free(&j);

    drmaa2_destroy_jsession("my_job_session");
    drmaa2_jsession_free(&js);
}


void test_j_get_info()
{
    drmaa2_jsession js = drmaa2_create_jsession("my_job_session", NULL);

    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    jt->remoteCommand = strdup("/bin/date");
    drmaa2_j j = drmaa2_jsession_run_job(js, jt);
    drmaa2_jtemplate_free(&jt);

    drmaa2_jinfo ji = drmaa2_j_get_info(j);
    drmaa2_jinfo_free(&ji);

    drmaa2_j_wait_terminated(j, DRMAA2_INFINITE_TIME);
    drmaa2_j_free(&j);
    drmaa2_destroy_jsession("my_job_session");
    drmaa2_jsession_free(&js);
}


void test_j_get_template()
{
drmaa2_jsession js = drmaa2_create_jsession("my_job_session", NULL);

    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    jt->remoteCommand = strdup("/bin/date");
    drmaa2_j j = drmaa2_jsession_run_job(js, jt);

    drmaa2_jtemplate jt2 = drmaa2_j_get_jt(j);
    CU_ASSERT_PTR_NOT_NULL(jt2);

    CU_ASSERT_STRING_EQUAL(jt2->remoteCommand, "/bin/date");
    CU_ASSERT_STRING_EQUAL(jt->remoteCommand, jt2->remoteCommand);

    drmaa2_jtemplate_free(&jt2);

    drmaa2_jtemplate_free(&jt);

    drmaa2_j_wait_terminated(j, DRMAA2_INFINITE_TIME);
    drmaa2_j_free(&j);
    drmaa2_destroy_jsession("my_job_session");
    drmaa2_jsession_free(&js);
}

