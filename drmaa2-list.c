#include "drmaa2-list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


void drmaa2_list_free (drmaa2_list * listref)
{
    drmaa2_list l = *listref;
    if (l == NULL)
        return;

    drmaa2_list_item head = (drmaa2_list_item)l->head;
    drmaa2_list_item tmp;
    while (head != NULL)
    {
        tmp = head;
        head = tmp->next;
        if (l->free_entry != NULL) l->free_entry((void **)&(tmp->data));
        free(tmp);
    }
    free(l);
    *listref = NULL;
}


const void *drmaa2_list_get (const drmaa2_list l, long pos)
{
    if (l == NULL)
        return NULL;

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
    if (l == NULL)
        return DRMAA2_INVALID_ARGUMENT;
  
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
    if (l == NULL)
        return DRMAA2_INVALID_ARGUMENT;

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


drmaa2_string_list string_split(const char *string, char separator)
{
    if (string == NULL)
        return DRMAA2_UNSET_LIST;

    drmaa2_string_list sl = drmaa2_list_create(DRMAA2_STRINGLIST, (drmaa2_list_entryfree)drmaa2_string_free);
    char *rest = (char *)string;
    char *to_insert;
    char *separator_pos;
    while((separator_pos = strchr(rest, separator)) != NULL)
    {
        to_insert = (char *)malloc(sizeof(char) * (separator_pos-rest+1));
        strncpy(to_insert, rest, separator_pos-rest);
        to_insert[separator_pos-rest] = '\0';
        drmaa2_list_add(sl, to_insert);
        rest = separator_pos + 1;
    }
    to_insert = (char *)malloc(sizeof(char) * (strlen(rest)+1));
    strncpy(to_insert, rest, strlen(rest));
    to_insert[strlen(rest)] = '\0';
    drmaa2_list_add(sl, to_insert);

    return sl;
}

drmaa2_string string_join(drmaa2_string_list sl, char separator)
{
    if (sl == NULL || drmaa2_list_size(sl) == 0)
        return NULL;
    size_t character_count = 0;
    //we calculate space for items, separators and terminating '\0'
    size_t i;
    for (i = 0; i < drmaa2_list_size(sl); i++)
    {
        character_count += strlen(drmaa2_list_get(sl, i)) + 1;
    }
    
    drmaa2_string string = (char *)malloc(sizeof(char) * character_count);
    char *next_cpy_pos = string;
    char *to_insert = NULL;
    for (i = 0; i < drmaa2_list_size(sl); i++)
    {
        to_insert = (char *)drmaa2_list_get(sl, i);
        size_t item_len = strlen(to_insert);
        strncpy(next_cpy_pos, to_insert, item_len);
        next_cpy_pos[item_len] = separator;
        next_cpy_pos += item_len + 1;
    }
    string[character_count-1] = '\0';
    return string;
}




