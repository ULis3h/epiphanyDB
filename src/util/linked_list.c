#include <stdio.h>
#include <stdlib.h>

#include "util/linked_list.h"

int init_list(LinkList *L) {
  *L = (LNode*)malloc(sizeof(LNode));

  if (*L) {
    (*L)->next = NULL;
    return 0;
  }
  return -1;
}

int list_length(LinkList L) {
  int len = 0;
  LNode *p = (LNode*)L;
  while (p) {
    p = L->next;
    len++;
  }
  return len;
}

int list_insert(LinkList *L, int index, int element) {

  LNode *p = (LNode*)L;
  LNode *target = (LNode*)malloc(sizeof(LNode));

  while (index--) {
    p = p->next;
  }

  target->data = element;
  target->next = p->next;

  p->next = target;
  return 0;
}

int list_delete(LinkList *L, int index) {

  LNode *p = (LNode*)L;
  while (--index && p->next) {
    p = p->next;
  }

  free(p->next);

  p->next = p->next->next;
  return 0;
}

int list_search(LinkList *L, int key, int *index) {

  int i = -1;
  LNode *p = (LNode*)L;
  while (p->next && p->data != key) {
    p = p->next;
    ++i;
  }
  *index = i;
  return *index;
}

void list_destroy(LinkList *L) {
  LNode *p = (LNode*)L;
  while (p) {
    LNode *q = p;
    p = p->next;
    free(q);
  }
}