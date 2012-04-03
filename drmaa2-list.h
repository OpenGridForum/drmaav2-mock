#include "drmaa2.h"

struct drmaa2_list_s {
  drmaa2_listtype type;
  int size;
  void * head;
};
typedef struct drmaa2_list_s drmaa2_list_t;


struct drmaa2_list_item_s {
	void *data;
	struct drmaa2_list_item_s *next;
};
typedef struct drmaa2_list_item_s drmaa2_list_item_t;
typedef drmaa2_list_item_t *drmaa2_list_item_h;


char **stringlist_get_array(drmaa2_list l);
void stringlist_free_array(char * const array[]);
