#include "drmaa2.h"

struct drmaa2_list_s {
  drmaa2_listtype type;
  int entries;
  void * head;
} drmaa2_list_s;

char ** stringlist_get_array(drmaa2_list l);
void stringlist_free_array(char * const array[]);
