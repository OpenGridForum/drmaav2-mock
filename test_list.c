#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "drmaa2-list.h"

void callback_free(void *entry)
{
    free(entry);
}


int main ()
{
    printf("======================= TEST_LIST ==============================\n");
    drmaa2_string_list sl = drmaa2_list_create(DRMAA2_STRINGLIST, callback_free);
    assert(drmaa2_list_size(sl) == 0);

    //test add
    printf("test1\n");
    drmaa2_list_add(sl, "Hello world");
    drmaa2_list_add(sl, "foo");
    drmaa2_list_add(sl, "foobar");
    assert(drmaa2_list_size(sl) == 3);
    assert(!strcmp((char *)drmaa2_list_get(sl,2), "foobar"));

    printf("test2\n");
    drmaa2_list_add(sl, "no bar");
    drmaa2_list_add(sl, "last bar");
    assert(!strcmp((char *)drmaa2_list_get(sl,4), "last bar"));

    //test delete
    printf("test3\n");
    drmaa2_list_del(sl, 3);
    assert(!strcmp((char *)drmaa2_list_get(sl,3), "last bar"));

    printf("test4\n");
    drmaa2_list_del(sl, 0);
    assert(!strcmp((char *)drmaa2_list_get(sl,0), "foo"));

    printf("test5\n");
    drmaa2_list_add(sl, "hello world");
    printf("test5\n");
    drmaa2_list_del(sl, 1);
    drmaa2_list_add(sl, "very last bar");
    printf("test6\n");
    assert(drmaa2_list_size(sl) == 4);
    assert(!strcmp((char *)drmaa2_list_get(sl,1), "last bar"));


    char **array = stringlist_get_array(sl);
    int i;
    for (i = 0; i < sl->size; i++)
        printf("%s (%li)\n", array[i], strlen(array[i]));
    // TODO: cleanup array


    // cleanup
    drmaa2_list_free(sl);

    printf("===================FINISHED TEST_LIST ==========================\n");
}
