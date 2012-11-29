#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../drmaa2.h"
#include "test_msession.h"
#include "../drmaa2-persistence.h"



void test_reservation_lists() {
    drmaa2_reset_db();
    drmaa2_error error_code;

    //test empty reservation list
    drmaa2_msession ms = drmaa2_open_msession("my_msession");
    drmaa2_r_list reservations = drmaa2_msession_get_all_reservations(ms);
    CU_ASSERT_EQUAL(drmaa2_list_size(reservations), 0);
    drmaa2_list_free(&reservations);


    //test single reservation
    drmaa2_rtemplate rt = drmaa2_rtemplate_create();
    drmaa2_rsession rs = drmaa2_create_rsession("my_rsession", NULL);
    drmaa2_r r = drmaa2_rsession_request_reservation(rs, rt);
    drmaa2_close_rsession(rs);
    drmaa2_rsession_free(&rs);
    drmaa2_r_free(&r);

    reservations = drmaa2_msession_get_all_reservations(ms);
    CU_ASSERT_EQUAL(drmaa2_list_size(reservations), 1);
    drmaa2_list_free(&reservations);

    //test more reservations
    rs = drmaa2_open_rsession("my_rsession");
    r = drmaa2_rsession_request_reservation(rs, rt);
    drmaa2_r_free(&r);
    r = drmaa2_rsession_request_reservation(rs, rt);
    drmaa2_r_free(&r);
    drmaa2_close_rsession(rs);
    drmaa2_rsession_free(&rs);


    reservations = drmaa2_msession_get_all_reservations(ms);
    CU_ASSERT_EQUAL(drmaa2_list_size(reservations), 3);
    drmaa2_list_free(&reservations);


    //test multiple reservation_sessions
    rs = drmaa2_create_rsession("my_rsession2", NULL);
    r = drmaa2_rsession_request_reservation(rs, rt);
    drmaa2_r_free(&r);
    drmaa2_close_rsession(rs);
    drmaa2_rsession_free(&rs);

    reservations = drmaa2_msession_get_all_reservations(ms);
    CU_ASSERT_EQUAL(drmaa2_list_size(reservations), 4);
    drmaa2_list_free(&reservations);


    drmaa2_close_msession(ms);
    drmaa2_msession_free(&ms);
    drmaa2_destroy_rsession("my_rsession");
    drmaa2_destroy_rsession("my_rsession2");
    drmaa2_rtemplate_free(&rt);
}


void test_job_lists() {
    drmaa2_reset_db();
    drmaa2_error error_code;

    //test empty job list
    drmaa2_msession ms = drmaa2_open_msession("my_msession");
    drmaa2_j_list jobs = drmaa2_msession_get_all_jobs(ms, NULL);
    CU_ASSERT_EQUAL(drmaa2_list_size(jobs), 0);
    drmaa2_list_free(&jobs);


    //test single job
    drmaa2_jtemplate jt = drmaa2_jtemplate_create();
    jt->remoteCommand = strdup("/bin/date");
    drmaa2_jsession js = drmaa2_create_jsession("my_jsession", NULL);
    drmaa2_j j = drmaa2_jsession_run_job(js, jt);
    drmaa2_close_jsession(js);

    jobs = drmaa2_msession_get_all_jobs(ms, NULL);
    CU_ASSERT_EQUAL(drmaa2_list_size(jobs), 1);
    drmaa2_list_free(&jobs);

    //test more jobs
    js = drmaa2_open_jsession("my_jsession");
    j = drmaa2_jsession_run_job(js, jt);
    j = drmaa2_jsession_run_job(js, jt);
    drmaa2_close_jsession(js);

    jobs = drmaa2_msession_get_all_jobs(ms, NULL);
    CU_ASSERT_EQUAL(drmaa2_list_size(jobs), 3);
    drmaa2_list_free(&jobs);


    //test multiple job_sessions
    js = drmaa2_create_jsession("my_jsession2", NULL);
    j = drmaa2_jsession_run_job(js, jt);
    drmaa2_close_jsession(js);

    jobs = drmaa2_msession_get_all_jobs(ms, NULL);
    CU_ASSERT_EQUAL(drmaa2_list_size(jobs), 4);

    int i;
    for (i = 0; i < drmaa2_list_size(jobs); i++)
    {
        drmaa2_j_wait_terminated((drmaa2_j)drmaa2_list_get(jobs, i), DRMAA2_INFINITE_TIME);
    }
    drmaa2_list_free(&jobs);

    drmaa2_close_msession(ms);
    drmaa2_destroy_jsession("my_jsession");
    drmaa2_destroy_jsession("my_jsession2");
    drmaa2_jtemplate_free(&jt);
}


void test_get_queues() {
    drmaa2_msession ms = drmaa2_open_msession("my_msession");
    drmaa2_queueinfo_list ql = drmaa2_msession_get_all_queues(ms, NULL);
    CU_ASSERT_PTR_NOT_NULL(ql);

    //drmaa2 mock specific
    CU_ASSERT_EQUAL(drmaa2_list_size(ql), 1);
    CU_ASSERT_STRING_EQUAL(((drmaa2_queueinfo)drmaa2_list_get(ql, 0))->name, "default");

    drmaa2_list_free(&ql);
    drmaa2_close_msession(ms);
    drmaa2_msession_free(&ms);
}


