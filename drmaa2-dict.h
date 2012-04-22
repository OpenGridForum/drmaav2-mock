#ifndef DRMAA2_DICT_H
#define DRMAA2_DICT_H

#include "drmaa2.h"


typedef struct drmaa2_dict_item_s {
	const char *key;
	const char *value;
	struct drmaa2_dict_item_s *next;
} drmaa2_dict_item_s;
typedef drmaa2_dict_item_s *drmaa2_dict_item;

typedef struct drmaa2_dict_s {
	drmaa2_dict_entryfree free_entry;
	drmaa2_dict_item head;
} drmaa2_dict_s;

#endif DRMAA2_DICT_H
