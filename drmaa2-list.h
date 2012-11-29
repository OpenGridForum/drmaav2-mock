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



// helper functions to store lists in a single database field and reconstruct them
drmaa2_string_list string_split(const char *string, char separator);
drmaa2_string string_join(drmaa2_string_list sl, char separator);


#endif 
