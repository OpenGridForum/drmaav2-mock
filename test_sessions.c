#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "drmaa2.h"


int main ()
{
    printf("======================= TEST_SESSIONS ==========================\n");
    drmaa2_error error_code;

    // test getting empty list
    drmaa2_string_list sl = drmaa2_get_jsession_names();
    assert(drmaa2_list_size(sl) == 0);
    drmaa2_list_free(sl);


    // test creation and closing of sessions
    drmaa2_jsession js = drmaa2_create_jsession("js1", NULL);
    drmaa2_close_jsession(js);

    js = drmaa2_open_jsession("js1");
    assert(js != NULL);
    sl = drmaa2_get_jsession_names();
    assert(drmaa2_list_size(sl) == 1);
    assert(strcmp((char *)drmaa2_list_get(sl, 0), "js1") == 0);
    drmaa2_list_free(sl);

    //test deletion of single session
    error_code = drmaa2_destroy_jsession("js1");
    assert(error_code == DRMAA2_SUCCESS);
    sl = drmaa2_get_jsession_names();
    assert(drmaa2_list_size(sl) == 0);
    drmaa2_list_free(sl);

    //test of opening non existing session
    js = drmaa2_open_jsession("js1");
    assert(js == NULL);


    //test creation of multiple sessions
    drmaa2_jsession js1, js2, js3, js4;
    js1 = drmaa2_create_jsession("js1", NULL);
    js2 = drmaa2_create_jsession("js2", NULL);
    js3 = drmaa2_create_jsession("js3", NULL);
    js4 = drmaa2_create_jsession("js4", NULL);

    sl = drmaa2_get_jsession_names();
    assert(drmaa2_list_size(sl) == 4);
    drmaa2_list_free(sl);

    //test deletion of non existing session
    error_code = drmaa2_destroy_jsession("non existing js");
    assert(error_code == DRMAA2_INVALID_ARGUMENT);


    //test deletion of sessions
    error_code = drmaa2_destroy_jsession("js2");
    assert(error_code == DRMAA2_SUCCESS);
    error_code = drmaa2_destroy_jsession("js4");
    assert(error_code == DRMAA2_SUCCESS);
    sl = drmaa2_get_jsession_names();
    assert(drmaa2_list_size(sl) == 2);
    assert(strcmp((char *)drmaa2_list_get(sl, 1), "js3") == 0);


    

    printf("===================FINISHED TEST_SESSIONS ======================\n");
}

