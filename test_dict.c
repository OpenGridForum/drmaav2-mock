#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "drmaa2-dict.h"
#include "drmaa2.h"

void callback_free(void *entry)
{
    free(entry);
}


int main ()
{
    printf("======================= TEST_DICT ==============================\n");

    drmaa2_dict_free(NULL);

    drmaa2_dict d = drmaa2_dict_create(NULL);

    drmaa2_dict_set(d, "name", "drmaa2");
    //drmaa2_dict_del(d, "name");
    //drmaa2_dict_set(d, "name", "drmaa2");
    assert(drmaa2_dict_has(d, "name") == DRMAA2_TRUE);
    assert(drmaa2_dict_has(d, "language") == DRMAA2_FALSE);

    const char *v = drmaa2_dict_get(d, "name");
    assert(!strcmp(v, "drmaa2"));

    drmaa2_dict_set(d, "language", "c");
    assert(drmaa2_dict_has(d, "language") == DRMAA2_TRUE);

    drmaa2_dict_del(d, "name");
    assert(drmaa2_dict_has(d, "name") == DRMAA2_FALSE);
    v = drmaa2_dict_get(d, "language");
    assert(!strcmp(v, "c"));

    drmaa2_dict_free(d);


    d = drmaa2_dict_create(NULL);
    drmaa2_dict_set(d, "name", "drmaa2");
    drmaa2_dict_set(d, "language", "c");
    drmaa2_dict_set(d, "version", "2");
    drmaa2_dict_set(d, "age", "2 weeks");

    assert(drmaa2_dict_has(d, "language") == DRMAA2_TRUE);

    drmaa2_dict_del(d, "age");
    drmaa2_dict_del(d, "language");

    assert(drmaa2_dict_has(d, "version") == DRMAA2_TRUE);

    drmaa2_dict_del(d, "name");
    drmaa2_dict_set(d, "version", "3");
    v = drmaa2_dict_get(d, "version");
    assert(!strcmp(v, "3"));

    drmaa2_dict_del(d, "version");
    drmaa2_dict_set(d, "new_version", "4");
    assert(drmaa2_dict_has(d, "version") == DRMAA2_FALSE);
    assert(drmaa2_dict_has(d, "new_version") == DRMAA2_TRUE);


    drmaa2_dict_free(d);

    printf("===================FINISHED TEST_DICT ==========================\n");
}
