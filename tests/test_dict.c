#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../drmaa2-dict.h"
#include "../drmaa2.h"
#include "test_dict.h"


void testFreeNullDict()
{
    drmaa2_dict d = NULL;
    drmaa2_dict_free(&d);
    CU_ASSERT(1);
}


void testDictErrorHandling()
{
    drmaa2_dict d = NULL;
    drmaa2_error e = drmaa2_dict_set(d, "hallo", "1");
    CU_ASSERT_EQUAL(e, DRMAA2_INVALID_ARGUMENT);

    e = drmaa2_dict_del(d, "hallo");
    CU_ASSERT_EQUAL(e, DRMAA2_INVALID_ARGUMENT);

    drmaa2_bool b = drmaa2_dict_has(d, "halo");
    CU_ASSERT_EQUAL(b, DRMAA2_FALSE);

    const char *item = drmaa2_dict_get(d, "hallo");
    CU_ASSERT_PTR_NULL(item);


    d = drmaa2_dict_create(NULL);
    drmaa2_dict_set(d, "hallo", "1");
    drmaa2_dict_set(d, "hallo2", "1");
    drmaa2_dict_set(d, "hallo3", "2");

    e = drmaa2_dict_del(d, "halo");
    CU_ASSERT_EQUAL(e, DRMAA2_INVALID_ARGUMENT);

    item = drmaa2_dict_get(d, "halo");
    CU_ASSERT_PTR_NULL(item);

    drmaa2_dict_free(&d);
}


void testBasicDict()
{
    drmaa2_dict d = drmaa2_dict_create(NULL);

    drmaa2_dict_set(d, "name", "drmaa2");
    drmaa2_dict_del(d, "name");
    drmaa2_dict_set(d, "name", "drmaa2");
    CU_ASSERT_EQUAL(drmaa2_dict_has(d, "name"), DRMAA2_TRUE);
    CU_ASSERT_EQUAL(drmaa2_dict_has(d, "language"), DRMAA2_FALSE);

    const char *v = drmaa2_dict_get(d, "name");
    CU_ASSERT_STRING_EQUAL(v, "drmaa2");

    drmaa2_dict_set(d, "language", "c");
    CU_ASSERT_EQUAL(drmaa2_dict_has(d, "language"), DRMAA2_TRUE);

    drmaa2_dict_del(d, "name");
    CU_ASSERT_EQUAL(drmaa2_dict_has(d, "name"), DRMAA2_FALSE);
    v = drmaa2_dict_get(d, "language");
    CU_ASSERT_STRING_EQUAL(v, "c");

    drmaa2_dict_free(&d);
    CU_ASSERT(1);
}

void testDict()
{
    drmaa2_dict d = drmaa2_dict_create(NULL);
    drmaa2_dict_set(d, "name", "drmaa2");
    drmaa2_dict_set(d, "language", "c");
    drmaa2_dict_set(d, "version", "2");
    drmaa2_dict_set(d, "age", "2 weeks");

    CU_ASSERT_EQUAL(drmaa2_dict_has(d, "language"), DRMAA2_TRUE);

    drmaa2_dict_del(d, "age");
    drmaa2_dict_del(d, "language");

    CU_ASSERT_EQUAL(drmaa2_dict_has(d, "version"), DRMAA2_TRUE);

    drmaa2_dict_del(d, "name");
    drmaa2_dict_set(d, "version", "3");
    const char *v = drmaa2_dict_get(d, "version");
    CU_ASSERT_STRING_EQUAL(v, "3");

    drmaa2_dict_del(d, "version");
    drmaa2_dict_set(d, "new_version", "4");
    CU_ASSERT_EQUAL(drmaa2_dict_has(d, "version"), DRMAA2_FALSE);
    CU_ASSERT_EQUAL(drmaa2_dict_has(d, "new_version"), DRMAA2_TRUE);

    drmaa2_dict_free(&d);
}


void testDictList()
{
    drmaa2_dict d = drmaa2_dict_create(NULL);
    drmaa2_dict_set(d, "name", "drmaa2");
    drmaa2_dict_set(d, "language", "c");
    drmaa2_dict_set(d, "version", "2");

    drmaa2_string_list sl = drmaa2_dict_list(d);

    CU_ASSERT_EQUAL(drmaa2_list_size(sl), 3);

    size_t i;
    for (i = 0; i < drmaa2_list_size(sl); i++) {
        CU_ASSERT_EQUAL(drmaa2_dict_has(d, drmaa2_list_get(sl, i)), DRMAA2_TRUE);
    }

    drmaa2_dict_free(&d);

    CU_ASSERT_STRING_NOT_EQUAL(drmaa2_list_get(sl, 0), drmaa2_list_get(sl, 1));
    CU_ASSERT_STRING_NOT_EQUAL(drmaa2_list_get(sl, 1), drmaa2_list_get(sl, 2));

    drmaa2_list_free(&sl);
}



