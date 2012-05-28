#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../drmaa2.h"
#include "test_sessions.h"


void test_empty_jsession_list()
{
    drmaa2_string_list sl = drmaa2_get_jsession_names();
    CU_ASSERT_EQUAL(drmaa2_list_size(sl), 0);
    drmaa2_list_free(sl);
}

void test_basic_session_methods()
{
     // test creation and closing of sessions
    drmaa2_jsession js = drmaa2_create_jsession("js1", NULL);
    CU_ASSERT_PTR_NOT_NULL(js);
    drmaa2_close_jsession(js);


    js = drmaa2_open_jsession("js1");
    CU_ASSERT_PTR_NOT_NULL(js);
    drmaa2_string_list sl = drmaa2_get_jsession_names();
    CU_ASSERT_EQUAL(drmaa2_list_size(sl), 1);
    CU_ASSERT_STRING_EQUAL((char *)drmaa2_list_get(sl, 0), "js1");
    drmaa2_list_free(sl);

    //test deletion of single session
    drmaa2_error error_code = drmaa2_destroy_jsession("js1");
    CU_ASSERT_EQUAL(error_code, DRMAA2_SUCCESS);
    sl = drmaa2_get_jsession_names();
    CU_ASSERT_EQUAL(drmaa2_list_size(sl), 0);
    drmaa2_list_free(sl);

    //test of opening non existing session
    js = drmaa2_open_jsession("js1");
    CU_ASSERT_PTR_NULL(js);
}


void test_multiple_sessions()
{
    //test creation of multiple sessions
    drmaa2_jsession js1, js2, js3, js4;
    js1 = drmaa2_create_jsession("js1", NULL);
    js2 = drmaa2_create_jsession("js2", NULL);
    js3 = drmaa2_create_jsession("js3", NULL);
    js4 = drmaa2_create_jsession("js4", NULL);

    drmaa2_string_list sl = drmaa2_get_jsession_names();
    CU_ASSERT_EQUAL(drmaa2_list_size(sl), 4);
    drmaa2_list_free(sl);

    //test deletion of non existing session
    drmaa2_error error_code = drmaa2_destroy_jsession("non existing js");
    CU_ASSERT_EQUAL(error_code, DRMAA2_INVALID_ARGUMENT);

    //test deletion of sessions
    error_code = drmaa2_destroy_jsession("js2");
    CU_ASSERT_EQUAL(error_code, DRMAA2_SUCCESS);
    error_code = drmaa2_destroy_jsession("js4");
    CU_ASSERT_EQUAL(error_code, DRMAA2_SUCCESS);
    sl = drmaa2_get_jsession_names();
    CU_ASSERT_EQUAL(drmaa2_list_size(sl), 2);
    CU_ASSERT_STRING_EQUAL((char *)drmaa2_list_get(sl, 1), "js3");
}
