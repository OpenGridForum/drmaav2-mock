#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "drmaa2.h"


int main ()
{
    printf("======================= TEST_MSESSION ==========================\n");
    drmaa2_error error_code;

    //test empty reservation list
    drmaa2_msession ms = drmaa2_open_msession("my_msession");
    drmaa2_r_list reservations = drmaa2_msession_get_all_reservations(ms);
    assert(drmaa2_list_size(reservations) == 0);
    drmaa2_list_free(reservations);


    //test single reservation
    drmaa2_rtemplate rt = drmaa2_rtemplate_create();
    drmaa2_rsession rs = drmaa2_create_rsession("my_rsession", NULL);
    drmaa2_r r = drmaa2_rsession_request_reservation(rs, rt);
    drmaa2_close_rsession(rs);

    reservations = drmaa2_msession_get_all_reservations(ms);
    assert(drmaa2_list_size(reservations) == 1);
    drmaa2_list_free(reservations);

    //test more reservations
    rs = drmaa2_open_rsession("my_rsession");
    //printf("%s\n", drmaa2_rsession_get_session_name(rs));
    r = drmaa2_rsession_request_reservation(rs, rt);
    r = drmaa2_rsession_request_reservation(rs, rt);
    drmaa2_close_rsession(rs);

    reservations = drmaa2_msession_get_all_reservations(ms);
    assert(drmaa2_list_size(reservations) == 3);
    drmaa2_list_free(reservations);


    //test multiple reservation_sessions
    rs = drmaa2_create_rsession("my_rsession2", NULL);
    r = drmaa2_rsession_request_reservation(rs, rt);
    drmaa2_close_rsession(rs);

    reservations = drmaa2_msession_get_all_reservations(ms);
    assert(drmaa2_list_size(reservations) == 4);
    drmaa2_list_free(reservations);



    drmaa2_close_msession(ms);
    drmaa2_destroy_rsession("my_rsession");
    drmaa2_rtemplate_free(rt);




    

    printf("===================FINISHED TEST_MSESSION ======================\n");
}

