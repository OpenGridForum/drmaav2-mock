/*
 *  CUnit - A Unit testing framework library for C.
 *  Copyright (C) 2004  Anil Kumar, Jerry St.Clair
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include "../drmaa2-persistence.h"

#include "test_app.h"
#include "test_dict.h"
#include "test_list.h"
#include "test_sessions.h"
#include "test_msession.h"
#include "test_jsession.h"
#include "test_rsession.h"
#include "test_reservation.h"
#include "test_job.h"



int main(int argc, char* argv[]) {
    CU_BasicRunMode mode = CU_BRM_VERBOSE;
    CU_ErrorAction error_action = CUEA_IGNORE;
    int i;

    setvbuf(stdout, NULL, _IONBF, 0);

    for (i=1 ; i<argc ; i++) {
        if (!strcmp("-i", argv[i])) {
            error_action = CUEA_IGNORE;
        }
        else if (!strcmp("-f", argv[i])) {
            error_action = CUEA_FAIL;
        }
        else if (!strcmp("-A", argv[i])) {
            error_action = CUEA_ABORT;
        }
        else if (!strcmp("-s", argv[i])) {
            mode = CU_BRM_SILENT;
        }
        else if (!strcmp("-n", argv[i])) {
            mode = CU_BRM_NORMAL;
        }
        else if (!strcmp("-v", argv[i])) {
            mode = CU_BRM_VERBOSE;
        }
        else {
            printf("\nUsage:  BasicTest [options]\n\n"
                   "Options:   -i   ignore framework errors [default].\n"
                   "           -f   fail on framework error.\n"
                   "           -A   abort on framework error.\n\n"
                   "           -s   silent mode - no output to screen.\n"
                   "           -n   normal mode - standard output to screen.\n"
                   "           -v   verbose mode - max output to screen [default].\n\n"
                   "           -h   print this message and exit.\n\n");
            return 0;
        }
    }

    if (CU_initialize_registry()) {
        printf("\nInitialization of Test Registry failed.");
    }
    else {
        // add Tests
        CU_pSuite pSuite = NULL;

        // First Suite
        pSuite = CU_add_suite("Dictionary", NULL, NULL);
        if (NULL == pSuite) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        /* add the tests to the suite */   
        if (NULL == CU_add_test(pSuite, "NullFree", testFreeNullDict)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Error handling", testDictErrorHandling)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Basic dictionary methods", testBasicDict)) {
            CU_cleanup_registry();
            return CU_get_error();
        } 
        if (NULL == CU_add_test(pSuite, "Dictionary methods", testDict)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "DictList methods", testDictList)) {
            CU_cleanup_registry();
            return CU_get_error();
        }


        // 2nd Suite
        pSuite = CU_add_suite("List", NULL, NULL);
        if (NULL == pSuite) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        /* add the tests to the suite */   
        if (NULL == CU_add_test(pSuite, "NullFree", testFreeNullList)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Error handling", testListErrorHandling)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Basic list methods", testList)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "String split method", testStringSplit)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "String join method", testStringJoin)) {
            CU_cleanup_registry();
            return CU_get_error();
        }


        // 3rd Suite
        pSuite = CU_add_suite("Applications", NULL, NULL);
        if (NULL == pSuite) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        /* add the tests to the suite */   
        if (NULL == CU_add_test(pSuite, "Simple App", test_simple_app)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Advanced App", test_advanced_app)) {
            CU_cleanup_registry();
            return CU_get_error();
        }

        // 4th Suite
        pSuite = CU_add_suite("Session Management", NULL, NULL);
        if (NULL == pSuite) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        /* add the tests to the suite */
        if (NULL == CU_add_test(pSuite, "Get Empty Session List", test_empty_jsession_list)) {
            CU_cleanup_registry();
            return CU_get_error();
        }   
        if (NULL == CU_add_test(pSuite, "Basic Session Handling", test_basic_session_methods)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Multiple Sessions", test_multiple_sessions)) {
            CU_cleanup_registry();
            return CU_get_error();
        }


        // 5th Suite
        pSuite = CU_add_suite("Job Session", NULL, NULL);
        if (NULL == pSuite) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        /* add the tests to the suite */
        if (NULL == CU_add_test(pSuite, "Session Invalidation", test_jsession_invalidation)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Getter Methods", test_jsession_getter_methods)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Getter Methods2", test_jsession_getter_methods2)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Job Lists", test_job_list)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Job Categories", test_job_categories)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Wait any terminated", test_wait_any_terminated)) {
            CU_cleanup_registry();
            return CU_get_error();
        }


        // 6th Suite
        pSuite = CU_add_suite("Monitoring Session", NULL, NULL);
        if (NULL == pSuite) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        /* add the tests to the suite */
        if (NULL == CU_add_test(pSuite, "Reservation Lists", test_reservation_lists)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Job Lists", test_job_lists)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Queue Lists", test_get_queues)) {
            CU_cleanup_registry();
            return CU_get_error();
        }


        // 7th Suite
        pSuite = CU_add_suite("Reservation Session", NULL, NULL);
        if (NULL == pSuite) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        /* add the tests to the suite */
        if (NULL == CU_add_test(pSuite, "Session Invalidation", test_rsession_invalidation)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Getter Methods", test_rsession_getter_methods)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Get reservation", test_rsession_get_reservation)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Get reservations", test_rsession_get_reservations)) {
            CU_cleanup_registry();
            return CU_get_error();
        }


        // 8th Suite
        pSuite = CU_add_suite("Reservation", NULL, NULL);
        if (NULL == pSuite) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        /* add the tests to the suite */
        if (NULL == CU_add_test(pSuite, "Get reservation info", test_r_get_info)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Get reservation template", test_r_get_template)) {
            CU_cleanup_registry();
            return CU_get_error();
        }


        // 8th Suite
        pSuite = CU_add_suite("Job", NULL, NULL);
        if (NULL == pSuite) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        /* add the tests to the suite */
        if (NULL == CU_add_test(pSuite, "Get job id/name", test_j_get_id_name)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Get job info", test_j_get_info)) {
            CU_cleanup_registry();
            return CU_get_error();
        }
        if (NULL == CU_add_test(pSuite, "Get job template", test_j_get_template)) {
            CU_cleanup_registry();
            return CU_get_error();
        }

        setenv("DRMAA2_DB_NAME", "drmaa2_test.sqlite", 1);
        drmaa2_reset_db();


        CU_basic_set_mode(mode);
        CU_set_error_action(error_action);
        printf("\nTests completed with return value %d.\n", CU_basic_run_tests());
        CU_cleanup_registry();
    }

    return 0;
}
