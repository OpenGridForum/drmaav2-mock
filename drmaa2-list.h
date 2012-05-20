#ifndef DRMAA2_LIST_H
#define DRMAA2_LIST_H

#include "drmaa2.h"


typedef struct drmaa2_list_item_s {
	const void *data;
	struct drmaa2_list_item_s *next;
} drmaa2_list_item_s;
typedef drmaa2_list_item_s *drmaa2_list_item;

typedef struct drmaa2_list_s {
	drmaa2_list_entryfree free_entry;
	drmaa2_listtype type;
	long size;
	void *head;
} drmaa2_list_s;



const char **stringlist_get_array(drmaa2_list l);
void stringlist_free_array(char **array);

int stringlist_contains(drmaa2_string_list sl, char *string);	// false: 0       true: !=0


#endif DRMAA2_LIST_H
