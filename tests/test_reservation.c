#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../drmaa2.h"
#include "../drmaa2-list.h"
#include "test_reservation.h"



void test_r_get_info() {
    drmaa2_rsession rs = drmaa2_create_rsession("mysession", NULL);

    // create and get reservation
    drmaa2_rtemplate rt = drmaa2_rtemplate_create();
    rt->maxSlots = 4;
    drmaa2_r r = drmaa2_rsession_request_reservation(rs, rt);
    drmaa2_rtemplate_free(&rt);

    drmaa2_rinfo ri = drmaa2_r_get_info(r);
    drmaa2_string r_id = drmaa2_r_get_id(r);
    CU_ASSERT_STRING_EQUAL(r_id, ri->reservationId);
    drmaa2_string_free(&r_id);
    drmaa2_rinfo_free(&ri);

    drmaa2_r_free(&r);


    drmaa2_destroy_rsession("mysession");
    drmaa2_rsession_free(&rs);
}


void test_r_get_template() {
    drmaa2_rsession rs = drmaa2_create_rsession("mysession", NULL);

    // create and get reservation
    drmaa2_rtemplate rt = drmaa2_rtemplate_create();
    rt->reservationName = strdup("first_reservation");
    rt->maxSlots = 4;
    drmaa2_r r = drmaa2_rsession_request_reservation(rs, rt);

    drmaa2_rtemplate rt2 = drmaa2_r_get_reservation_template(r);
    CU_ASSERT_PTR_NOT_NULL(r);

    CU_ASSERT_STRING_EQUAL(rt2->reservationName, "first_reservation");
    CU_ASSERT_STRING_EQUAL(rt->reservationName, rt2->reservationName);


    drmaa2_rtemplate_free(&rt);
    drmaa2_rtemplate_free(&rt2);


    drmaa2_r_free(&r);


    drmaa2_destroy_rsession("mysession");
    drmaa2_rsession_free(&rs);
}


