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


typedef void *(*drmaa2_copy_data_callback)(const void *data);

drmaa2_list drmaa2_list_create_copy(drmaa2_list l, const drmaa2_list_entryfree callback, const drmaa2_copy_data_callback copy);


#endif DRMAA2_LIST_H
