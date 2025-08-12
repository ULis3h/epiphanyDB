#ifndef EPIPHANY_LINKED_LIST_H_
#define EPIPHANY_LINKED_LIST_H_

#include "common/types.h"

/**
 * Linked List Node Definition.
 */
typedef struct LNode {
  int data; // data field
  struct LNode *next; // pointer field
}LNode, *LinkList;

int init_list(LinkList *L);
int list_length(LinkList L);
int list_insert(LinkList *L, int i, int e);
int list_delete(LinkList *L, int i);
int list_search(LinkList *L, int key, int *index);
void list_destroy(LinkList *L);


#endif
