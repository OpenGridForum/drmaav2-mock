#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../drmaa2-list.h"
#include "../drmaa2.h"
#include "test_list.h"
#include "../drmaa2-debug.h"


void testFreeNullList()
{
    drmaa2_list l = NULL;
    drmaa2_list_free(&l);
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
        DRMAA2_DEBUG_PRINT("%s (%li)\n", array[i], strlen(array[i]));
    // TODO: cleanup array


    // cleanup
    drmaa2_list_free(&sl);

    CU_ASSERT(1);
}


void testCreateCopy()
{

    drmaa2_list l = DRMAA2_UNSET_LIST;
    drmaa2_list copy = drmaa2_list_create_copy(l, NULL, NULL);
    CU_ASSERT_PTR_NULL(copy);

    l = drmaa2_list_create(DRMAA2_STRINGLIST, NULL);
    copy = drmaa2_list_create_copy(l, NULL, NULL);
    CU_ASSERT_EQUAL(drmaa2_list_size(l), drmaa2_list_size(copy));
    CU_ASSERT_EQUAL(l->type, copy->type);

    drmaa2_list_free(&copy);
    drmaa2_list_add(l, "hello");
    drmaa2_list_add(l, "world");


    copy = drmaa2_list_create_copy(l, NULL, NULL);
    CU_ASSERT_EQUAL(drmaa2_list_size(l), drmaa2_list_size(copy));
    CU_ASSERT_EQUAL(l->type, copy->type);
    CU_ASSERT_STRING_EQUAL(drmaa2_list_get(copy, 0), "hello");
    CU_ASSERT_STRING_EQUAL(drmaa2_list_get(copy, 1), "world");

    drmaa2_list_free(&copy);

    copy = drmaa2_list_create_copy(l, (drmaa2_list_entryfree)drmaa2_string_free, (drmaa2_copy_data_callback)strdup);
    drmaa2_list_add(copy, strdup("ich"));
    CU_ASSERT_STRING_EQUAL(drmaa2_list_get(copy, 2), "ich");
    drmaa2_list_free(&copy);

    CU_PASS("test finished")


}



