#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../drmaa2-list.h"
#include "../drmaa2.h"
#include "test_list.h"


void testFreeNullList()
{
    drmaa2_dict_free(NULL);
    CU_ASSERT(1);
}


void testList()
{
    drmaa2_string_list sl = drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
    CU_ASSERT_EQUAL(drmaa2_list_size(sl), 0);

    // test add
    drmaa2_list_add(sl, "Hello world");
    drmaa2_list_add(sl, "foo");
    drmaa2_list_add(sl, "foobar");
    CU_ASSERT_EQUAL(drmaa2_list_size(sl), 3);
    CU_ASSERT_STRING_EQUAL((char *)drmaa2_list_get(sl,2), "foobar");
    
    drmaa2_list_add(sl, "no bar");
    drmaa2_list_add(sl, "last bar");
    CU_ASSERT_STRING_EQUAL((char *)drmaa2_list_get(sl,4), "last bar");

    // test delete
    drmaa2_list_del(sl, 3);
    CU_ASSERT_STRING_EQUAL((char *)drmaa2_list_get(sl,3), "last bar");
    drmaa2_list_del(sl, 0);
    CU_ASSERT_STRING_EQUAL((char *)drmaa2_list_get(sl,0), "foo");

    // test size
    drmaa2_list_add(sl, "hello world");
    drmaa2_list_del(sl, 1);
    drmaa2_list_add(sl, "very last bar");
    CU_ASSERT_EQUAL(drmaa2_list_size(sl), 4);
    CU_ASSERT_STRING_EQUAL((char *)drmaa2_list_get(sl,1), "last bar");

    const char **array = stringlist_get_array(sl);
    int i;
    for (i = 0; i < sl->size; i++)
        printf("%s (%li)\n", array[i], strlen(array[i]));
    // TODO: cleanup array


    // cleanup
    drmaa2_list_free(sl);

    CU_ASSERT(1);
}
