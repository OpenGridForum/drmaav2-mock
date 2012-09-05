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

