#include "drmaa2-dict.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

drmaa2_dict drmaa2_dict_create(const drmaa2_dict_entryfree callback)
{
    drmaa2_dict d = (drmaa2_dict)malloc(sizeof(drmaa2_dict_s));
    d->free_entry = callback;
    d->head = NULL;
    return d;
}


void drmaa2_dict_free(drmaa2_dict * dRef)
{
    drmaa2_dict d = *dRef;
    if (d == NULL)
        return;

    drmaa2_dict_item head = d->head;
    drmaa2_dict_item tmp;
    while (head != NULL)
    {
        tmp = head;
        head = head->next;
        if (d->free_entry != NULL) 
            d->free_entry((char **)&(tmp->key), (char **)&(tmp->value));
        free(tmp);
    }
    free(d);
    *dRef = NULL;
} 


drmaa2_string_list drmaa2_dict_list(const drmaa2_dict d)
{
    if (d == NULL)
        return NULL;

    drmaa2_string_list sl = drmaa2_list_create(DRMAA2_STRINGLIST, (drmaa2_list_entryfree)drmaa2_string_free);
    drmaa2_dict_item current_item = d->head;
    while (current_item != NULL)
    {
        drmaa2_list_add(sl, strdup(current_item->key));
        current_item = current_item->next;
    }
    return sl;
}


drmaa2_bool drmaa2_dict_has(const drmaa2_dict d, const char * key)
{
    if (d == NULL)
        return DRMAA2_FALSE;

    drmaa2_dict_item current_item = d->head;
    while (current_item != NULL)
    {
        if (!strcmp(current_item->key, key))
        {
            // found
            return DRMAA2_TRUE;
        }
        current_item = current_item->next;
    }
    return DRMAA2_FALSE;
}


const char * drmaa2_dict_get(const drmaa2_dict d, const char * key)
{
    if (d == NULL)
        return NULL;

    drmaa2_dict_item current_item = d->head;
    while (current_item != NULL)
    {
        if (!strcmp(current_item->key, key))
        {
            // found -> return
            return current_item->value;
        }
        current_item = current_item->next;
    }
    return NULL;
}


drmaa2_error drmaa2_dict_del(drmaa2_dict d, const char * key)
{
    if (d == NULL)
        return DRMAA2_INVALID_ARGUMENT;

    drmaa2_dict_item current_item = d->head;
    drmaa2_dict_item prev = NULL;

    while (current_item != NULL)
    {
        if (!strcmp(current_item->key, key))
        {
            // found -> delete
            if (prev)
            {
                prev->next = current_item->next;
            }
            else
            {
                d->head = current_item->next;
            }
            if (d->free_entry != NULL) d->free_entry((char **)&(current_item->key), (char **)&(current_item->value));
            free(current_item);
            return DRMAA2_SUCCESS;
        }
        prev = current_item;
        current_item = current_item->next;
    }
    return DRMAA2_INVALID_ARGUMENT;
}

drmaa2_error drmaa2_dict_set(drmaa2_dict d, const char * key, const char * val)
{
    if (d == NULL)
        return DRMAA2_INVALID_ARGUMENT;

    if (d->head == NULL)
    {
        //empty dict
        drmaa2_dict_item new = (drmaa2_dict_item)malloc(sizeof(drmaa2_dict_item_s));
        new->key = key;
        new->value = val;
        new->next = NULL;
        d->head = new;
        return DRMAA2_SUCCESS;
    }
    drmaa2_dict_item current_item = d->head;
    drmaa2_dict_item prev = NULL;
    while (current_item != NULL)
    {
        if (!strcmp(current_item->key, key))
        {
            // found -> replace
            if (d->free_entry != NULL) d->free_entry(NULL, (char **)&(current_item->value));
            current_item->value = val;
            return DRMAA2_SUCCESS;
        }
        prev = current_item;
        current_item = current_item->next;
    }
    drmaa2_dict_item new = (drmaa2_dict_item)malloc(sizeof(drmaa2_dict_item_s));
    new->key = key;
    new->value = val;
    new->next = NULL;
    prev->next = new;
    return DRMAA2_FALSE;
}
