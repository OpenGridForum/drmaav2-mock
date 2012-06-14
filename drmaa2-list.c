#include "drmaa2-list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

const char **stringlist_get_array(drmaa2_list l)
{
    // no deep copy
    assert(l->type == DRMAA2_STRINGLIST);
    const char **array = (const char **)malloc(l->size * sizeof(char *));
    drmaa2_list_item current_item = (drmaa2_list_item)l->head;
    int i;
    for (i = 0; i < l->size; i++)
    {
        array[i] = current_item->data;
        current_item = current_item->next;
    }
    return array;
}

void stringlist_free_array(char **array)
{
    // no deep free
    free(array);
}

int stringlist_contains(drmaa2_string_list sl, char *string)
{
    drmaa2_list_item current_item = sl->head;
    while (current_item != NULL)
    {
        if (strcmp((char *)current_item->data, string) == 0) return 1;
        current_item = current_item->next;
    }
    return 0;
}


drmaa2_error drmaa2_list_free (drmaa2_list l)
{
    if (l == NULL)
    {
        return DRMAA2_SUCCESS;
    }

    drmaa2_list_item head = (drmaa2_list_item)l->head;
    drmaa2_list_item tmp;
    while (head != NULL)
    {
        tmp = head;
        head = tmp->next;
        if (l->free_entry != NULL) l->free_entry((void *)tmp->data);
        free(tmp);
    }
    free(l);
    return DRMAA2_SUCCESS;
}


const void *drmaa2_list_get (const drmaa2_list l, long pos)
{
    if (pos < 0 || pos >= l->size)
        return NULL;

    drmaa2_list_item current_item = (drmaa2_list_item)l->head;
    long i;
    for (i = 0; i < pos; i++)
        current_item = current_item->next;
    return current_item->data;
}


drmaa2_error drmaa2_list_add (drmaa2_list l, const void * value)
{   
    drmaa2_list_item new_item;
    if ((new_item = (drmaa2_list_item)malloc(sizeof(drmaa2_list_item))) == NULL)
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
        drmaa2_list_item current_item = (drmaa2_list_item)l->head;
        while (current_item->next)
            current_item = current_item->next;
        current_item->next = new_item;
    }
    l->size++;

    return DRMAA2_SUCCESS;
}

drmaa2_error drmaa2_list_del (drmaa2_list l, long pos)
{
    if (pos < 0 || pos >= l->size)
        return DRMAA2_INVALID_ARGUMENT;

    drmaa2_list_item current_item = (drmaa2_list_item)l->head;
    if (pos == 0)
    {
        drmaa2_list_item to_delete = current_item;
        l->head = to_delete->next;
        l->size--;
        if (l->free_entry != NULL) l->free_entry((void *)to_delete->data);
        free(to_delete);

        return DRMAA2_SUCCESS;
    }

    long i;
    for (i = 0; i < pos-1; i++)
        current_item = current_item->next;
    //current_item points to the item before the one we want to remove
    drmaa2_list_item to_delete = current_item->next;
    current_item->next = to_delete->next;
    l->size--;
    if (l->free_entry != NULL) l->free_entry((void *)to_delete->data);  
    free(to_delete);

    return DRMAA2_SUCCESS;
}


long drmaa2_list_size (const drmaa2_list l)
{
    if (l == NULL)
        return -1;
    return l->size;
}


drmaa2_list drmaa2_list_create (const drmaa2_listtype t, const drmaa2_list_entryfree callback)
{
  drmaa2_list l;

  l = (drmaa2_list)malloc(sizeof(drmaa2_list_s));
  l->free_entry = callback;
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
    default:
        return l;
  }
}


drmaa2_list drmaa2_list_create_copy(drmaa2_list l, const drmaa2_list_entryfree callback, const drmaa2_copy_data_callback copy)
{
    //the user is responsible for setting appropriate copy and free callbacks
    if (l == NULL)
        return DRMAA2_UNSET_LIST;

    drmaa2_list list = drmaa2_list_create(l->type, callback);
    
    drmaa2_list_item current_item, new_item;
    drmaa2_list_item *p_to_set;
    current_item = l->head;
    p_to_set = (drmaa2_list_item*)&(list->head);
    while (current_item != NULL)
    {
        new_item = (drmaa2_list_item)malloc(sizeof(drmaa2_list_item));
        *p_to_set = new_item;
        //make flat copy in case that no copy callback is set
        new_item->data = (copy != NULL) ? copy(current_item->data) : current_item->data;

        current_item = current_item->next;
        p_to_set = &(new_item->next);
    }
    new_item = NULL;
    list->size = l->size;
    return list;
}

