#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../drmaa2.h"
#include "../drmaa2-list.h"
#include "test_rsession.h"


void test_rsession_invalidation()
{
    drmaa2_rsession rs = drmaa2_create_rsession("mysession", NULL);
    drmaa2_destroy_rsession("mysession");
    CU_ASSERT_PTR_NULL(drmaa2_rsession_get_contact(rs));
    CU_ASSERT_PTR_NULL(drmaa2_rsession_get_session_name(rs));
    CU_ASSERT_EQUAL(drmaa2_lasterror(), DRMAA2_INVALID_SESSION);
    drmaa2_rsession_free(&rs);
}


void test_rsession_getter_methods()
{
    drmaa2_rsession rs = drmaa2_create_rsession("mysession", NULL);
    
    // test getter functions
    drmaa2_string name = drmaa2_rsession_get_session_name(rs);
    CU_ASSERT_STRING_EQUAL(name, "mysession");
    drmaa2_string_free(&name);
    drmaa2_string contact = drmaa2_rsession_get_contact(rs);
    CU_ASSERT_PTR_NULL(contact);
    drmaa2_string_free(&contact);

    drmaa2_destroy_rsession("mysession");
    drmaa2_rsession_free(&rs);
}


void test_rsession_get_reservation()
{
    drmaa2_rsession rs = drmaa2_create_rsession("mysession2", NULL);
    CU_ASSERT_PTR_NOT_NULL(rs);

    // test invalid reservation
    drmaa2_r r = drmaa2_rsession_get_reservation(rs, "123");
    CU_ASSERT_PTR_NULL(r);
    CU_ASSERT_EQUAL(drmaa2_lasterror(), DRMAA2_INVALID_ARGUMENT);

    // create and get reservation
    drmaa2_rtemplate rt = drmaa2_rtemplate_create();
    rt->maxSlots = 4;                                                    // perform advance reservation
    if (DRMAA2_TRUE == drmaa2_supports(DRMAA2_RT_MACHINEOS)) 
        rt->machineOS = DRMAA2_LINUX;
    r = drmaa2_rsession_request_reservation(rs, rt);

    drmaa2_string r_id  = drmaa2_r_get_id(r);
    drmaa2_r r2        = drmaa2_rsession_get_reservation(rs, r_id);
    drmaa2_string r_id2 = drmaa2_r_get_id(r2);

    CU_ASSERT_STRING_EQUAL(r_id, r_id2);
    drmaa2_string_free(&r_id);
    drmaa2_string_free(&r_id2);

    drmaa2_string sn  = drmaa2_r_get_session_name(r);
    drmaa2_string sn2 = drmaa2_r_get_session_name(r2);

    CU_ASSERT_STRING_EQUAL(sn, "mysession2");
    CU_ASSERT_STRING_EQUAL(sn, sn2);
    drmaa2_string_free(&sn);
    drmaa2_string_free(&sn2);

    drmaa2_r_free(&r);
    drmaa2_r_free(&r2);


    drmaa2_destroy_rsession("mysession2");
    drmaa2_rsession_free(&rs);
}

