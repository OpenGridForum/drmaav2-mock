#include "drmaa2-list.h"
#include <stdlib.h>
#include <assert.h>

char ** stringlist_get_array(drmaa2_list l) {
  assert(l->type == DRMAA2_STRINGLIST);
  return NULL;
}

void stringlist_free_array(char * const array[]) {
}

drmaa2_error drmaa2_list_free (drmaa2_list l) {
  free(l);
  return DRMAA2_SUCCESS;
}

void * drmaa2_list_get (drmaa2_list l, int pos) {
}

drmaa2_error drmaa2_list_add (drmaa2_list l, void * value) {
}

drmaa2_error drmaa2_list_del (drmaa2_list l, int pos) {
}

int drmaa2_list_size (const drmaa2_list l) {
}

drmaa2_list drmaa2_list_create (const drmaa2_listtype t, const drmaa2_list_entryfree callback) {

  drmaa2_list l;

  l=(drmaa2_list) malloc(sizeof(drmaa2_list_s));
  l->type=t;
  switch (l->type) {
    case DRMAA2_STRINGLIST:
      return (drmaa2_string_list)l;
      break;
    case DRMAA2_JOBLIST:
      return (drmaa2_j_list)l;
      break;
    case DRMAA2_QUEUEINFOLIST:
      return (drmaa2_queueinfo_list)l;
      break;
    case DRMAA2_MACHINEINFOLIST:
      return (drmaa2_machineinfo_list)l;
      break;
    case DRMAA2_SLOTINFOLIST:
      return (drmaa2_slotinfo_list)l;
      break;
    case DRMAA2_RESERVATIONLIST:
      return (drmaa2_r_list)l;
      break;
  }
}

