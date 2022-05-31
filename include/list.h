#ifndef LIST_H
#define LIST_H
#include "os_type.h"

typedef struct ListItem {
    struct ListItem *prev;
    struct ListItem *next;
} ListItem;


ListItem li_new();

void li_init(ListItem *li);

int li_size(ListItem *li);

bool li_empty(ListItem *li);

ListItem *li_back(ListItem *li);

void li_push_back(ListItem *li, ListItem *x);

void li_pop_back(ListItem *li);

ListItem *li_front(ListItem *li);

void li_push_front(ListItem *li, ListItem *x);

void li_pop_front(ListItem *li);

void li_insert(ListItem *li, ListItem *x, int pos);

void li_delete(ListItem *li, int pos);

void li_erase(ListItem *head, ListItem *x);

ListItem *li_at(ListItem *head, int pos);

int li_find(ListItem *head, ListItem *x);

void li_print(ListItem *head);


#endif