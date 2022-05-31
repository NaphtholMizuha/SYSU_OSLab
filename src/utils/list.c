#include "list.h"
#include "os_constant.h"
#include "std_io.h"
ListItem li_new() {
    ListItem head;
    head.next = nullptr;
    head.prev = nullptr;
    return head;
}

void li_init(ListItem *head) {
    head->next = nullptr;
    head->prev = nullptr;
}

int li_size(ListItem *head) {
    ListItem *temp = head->next;
    int count = 0;

    while (temp) {
        temp = temp->next;
        ++count;
    }

    return count;
}

bool li_empty(ListItem *head) {
    return li_size(head) == 0;
}

ListItem *li_back(ListItem *head) {
    ListItem *temp = head->next;
    if (!temp) {
        return nullptr;
    }
    while (temp->next) {
        temp = temp->next;
    }
    return temp;
}

void li_push_back(ListItem *head, ListItem *x) {
    ListItem *temp = li_back(head);
    if (!temp) {
        temp = head;
    }
    temp->next = x;
    x->prev = temp;
    x->next = nullptr;
}

void li_pop_back(ListItem *head) {
    ListItem *temp = li_back(head);
    if (temp) {
        temp->prev->next = nullptr;
        temp->prev = nullptr;
        temp->next = nullptr;
    }
}

ListItem *li_front(ListItem *head) {
    return head->next;
}

void li_push_front(ListItem *head, ListItem *x) {
    ListItem *temp = head->next;
    if (temp) {
        temp->prev = x;
    }
    head->next = x;
    x->prev = head;
    x->next = temp;
}

void li_pop_front(ListItem *head) {
    ListItem *temp = head->next;
    if (temp) {
        if (temp->next) {
            temp->next->prev = head;
        }
        head->next = temp->next;
        temp->prev = nullptr;
        temp->next = nullptr;
    }
}

void li_insert(ListItem *head, ListItem *x, int pos) {
    if (pos == 0) {
        li_push_front(head, x);
    } else {
        int length = li_size(head);
        if (pos == length) {
            li_push_back(head, x);
        } else if (pos < length) {
            ListItem *temp = li_at(head, pos);
            x->prev = temp->prev;
            x->next = temp;
            temp->prev->next = x;
            temp->prev = x;
        }
    }
}

void li_delete(ListItem *head, int pos) {
    if (pos == 0) {
        li_pop_front(head);
    } else {
        int length = li_size(head);
        if (pos < length) {
            ListItem *temp = li_at(head, pos);

            temp->prev->next = temp->next;
            if (temp->next) {
                temp->next->prev = temp->prev;
            }
        }
    }
}

void li_erase(ListItem *head, ListItem *x) {
    ListItem *temp = head->next;

    while (temp && temp != x) {
        temp = temp->next;
    }

    if (temp) {
        temp->prev->next = temp->next;
        if (temp->next) {
            temp->next->prev = temp->prev;
        }
    }
}

ListItem *li_at(ListItem *head, int pos) {
    ListItem *temp = head->next;

    for (int i = 0; i < pos && temp; ++i, temp = temp->next) {}

    return temp;
}

int li_find(ListItem *head, ListItem *x) {
    int pos = 0;
    ListItem *temp = head->next;

    while (temp && temp != x) {
        temp = temp->next;
        ++pos;
    }

    if (temp && temp == x) {
        return pos;
    } else {
        return -1;
    }
}

void li_print(ListItem *head) {
    ListItem *temp = head;
    while (temp->next) {
        printf("0x%x->", temp->next);
        temp = temp->next;
    }
    printf("NULL\n");
}