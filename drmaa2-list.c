#include "drmaa2-list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

char **stringlist_get_array(drmaa2_list l)
{
    assert(l->type == DRMAA2_STRINGLIST);
    char **array = (char **)malloc(l->size * sizeof(char *));
    drmaa2_list_item_h current_item = (drmaa2_list_item_h)l->head;
    int i;
    char *tmp;
    for (i = 0; i < l->size; i++)
    {
        tmp = (char *) malloc((strlen(current_item->data) + 1) * sizeof(char));
        array[i] = tmp;
        strcpy(tmp, (char *)current_item->data);
        current_item = current_item->next;
    }
    return array;
}

void stringlist_free_array(char * const array[])
{
}

drmaa2_error drmaa2_list_free (drmaa2_list l)
{
    drmaa2_list_item_h head = (drmaa2_list_item_h)l->head;
    drmaa2_list_item_h tmp;
    while (head != NULL)
    {
        tmp = head;
        head = tmp->next;
        free(tmp);
    }
    free(l);
    return DRMAA2_SUCCESS;
}


void *drmaa2_list_get (drmaa2_list l, int pos)
{
    if (pos < 0 || pos >= l->size)
        return NULL;
    drmaa2_list_item_h current_item = (drmaa2_list_item_h)l->head;
    int i;
    for (i = 0; i < pos; i++)
        current_item = current_item->next;
    return current_item->data;
}


drmaa2_error drmaa2_list_add (drmaa2_list l, void * value)
{   
    drmaa2_list_item_h new_item;
    if ((new_item = (drmaa2_list_item_h)malloc(sizeof(drmaa2_list_item_t))) == NULL)
    {
        printf("bad allocation\n");
        return DRMAA2_OUT_OF_RESOURCE;
    }
    new_item->data = value;
    new_item->next = NULL;

   if (l->head == NULL)
    {
        l->head = new_item;
    }
    else
    {
        drmaa2_list_item_h current_item = (drmaa2_list_item_h)l->head;
        while (current_item->next)
            current_item = current_item->next;
        current_item->next = new_item;
    }
    l->size++;

    return DRMAA2_SUCCESS;
}

drmaa2_error drmaa2_list_del (drmaa2_list l, int pos)
{
    if (pos < 0 || pos >= l->size)
        return DRMAA2_INVALID_ARGUMENT;

    drmaa2_list_item_h current_item = (drmaa2_list_item_h)l->head;
    if (pos == 0)
    {
        drmaa2_list_item_h to_delete = current_item;
        l->head = to_delete->next;
        l->size--;
        free(to_delete);

        return DRMAA2_SUCCESS;
    }

    int i;
    for (i = 0; i < pos-1; i++)
        current_item = current_item->next;
    //current_item points to the item before the one we want to remove
    drmaa2_list_item_h to_delete = current_item->next;
    current_item->next = to_delete->next;
    l->size--;
    free(to_delete);

    return DRMAA2_SUCCESS;
}


int drmaa2_list_size (const drmaa2_list l)
{
    return l->size;
}


drmaa2_list drmaa2_list_create (const drmaa2_listtype t, const drmaa2_list_entryfree callback) {

  drmaa2_list l;

  l = (drmaa2_list)malloc(sizeof(drmaa2_list_t));
  l->type = t;
  l->size = 0;
  l->head = NULL;
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

