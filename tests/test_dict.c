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
