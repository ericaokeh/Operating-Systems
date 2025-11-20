// list/list.c
//
// Implementation for linked list.
//
// <Author>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/* Helpers */
static node_t *getNode(elem value) {
  node_t *n = (node_t *)malloc(sizeof(node_t));
  if (!n) return NULL;
  n->value = value;
  n->next = NULL;
  return n;
}

list_t *list_alloc() {
  list_t *mylist = (list_t *)malloc(sizeof(list_t));
  if (!mylist) return NULL;
  mylist->head = NULL;
  return mylist;               // <-- you were missing this return
}

void list_free(list_t *l) {
  if (!l) return;
  node_t *cur = l->head;
  while (cur) {
    node_t *tmp = cur;
    cur = cur->next;
    free(tmp);
  }
  free(l);
}

void list_print(list_t *l) {
  if (!l) { printf("NULL\n"); return; }
  node_t *cur = l->head;
  while (cur) {
    printf("%d->", cur->value);
    cur = cur->next;
  }
  printf("NULL\n");
}

char *listToString(list_t *l) {
  /* simple heap buffer for tests; caller doesn't free in main.c */
  char *buf = (char *)malloc(10024);
  if (!buf) return NULL;
  buf[0] = '\0';               // <-- need to initialize before strcat
  char tbuf[32];

  node_t *curr = l->head;
  while (curr != NULL) {
    sprintf(tbuf, "%d->", curr->value);
    strcat(buf, tbuf);
    curr = curr->next;
  }
  strcat(buf, "NULL");
  return buf;
}

int list_length(list_t *l) {
  int cnt = 0;
  for (node_t *c = l->head; c; c = c->next) cnt++;
  return cnt;
}

void list_add_to_front(list_t *l, elem value) {
  node_t *cur_node = getNode(value);
  if (!cur_node) return;
  cur_node->next = l->head;
  l->head = cur_node;
}

void list_add_to_back(list_t *l, elem value) {
  node_t *n = getNode(value);
  if (!n) return;
  if (!l->head) { l->head = n; return; }
  node_t *c = l->head;
  while (c->next) c = c->next;
  c->next = n;
}

/* 1-based indexing as implied by your tests:
   index <= 1 -> push front
   index == len+1 -> push back
   index in (1, len] -> insert before that position
   index > len+1 -> no-op
*/
void list_add_at_index(list_t *l, elem value, int index) {
  if (index <= 1 || !l->head) { list_add_to_front(l, value); return; }

  int len = list_length(l);
  if (index == len + 1) { list_add_to_back(l, value); return; }
  if (index > len + 1) return;

  node_t *n = getNode(value);
  if (!n) return;

  node_t *c = l->head;
  /* stop at node just before target slot */
  for (int i = 1; i < index - 1 && c; i++) c = c->next;
  if (!c) { free(n); return; }
  n->next = c->next;
  c->next = n;
}

elem list_remove_from_front(list_t *l) {
  if (!l->head) return -1;
  node_t *tmp = l->head;
  elem v = tmp->value;
  l->head = tmp->next;
  free(tmp);
  return v;
}

elem list_remove_from_back(list_t *l) {
  if (!l->head) return -1;
  if (!l->head->next) {
    elem v = l->head->value;
    free(l->head);
    l->head = NULL;
    return v;
  }
  node_t *c = l->head;
  while (c->next->next) c = c->next;
  elem v = c->next->value;
  free(c->next);
  c->next = NULL;
  return v;
}

/* 1-based indexing; out-of-range -> return -1 and leave list unchanged */
elem list_remove_at_index(list_t *l, int index) {
  if (index <= 0 || !l->head) return -1;
  if (index == 1) return list_remove_from_front(l);

  node_t *c = l->head;
  for (int i = 1; i < index - 1 && c; i++) c = c->next;
  if (!c || !c->next) return -1;

  node_t *victim = c->next;
  elem v = victim->value;
  c->next = victim->next;
  free(victim);
  return v;
}

bool list_is_in(list_t *l, elem value) {
  for (node_t *c = l->head; c; c = c->next)
    if (c->value == value) return true;
  return false;
}

/* 1-based indexing; invalid index -> -1 */
elem list_get_elem_at(list_t *l, int index) {
  if (index <= 0) return -1;
  node_t *c = l->head;
  for (int i = 1; c && i < index; i++) c = c->next;
  return c ? c->value : -1;
}

/* 1-based index of first occurrence; not found -> -1 */
int list_get_index_of(list_t *l, elem value) {
  int i = 1;
  for (node_t *c = l->head; c; c = c->next, i++)
    if (c->value == value) return i;
  return -1;
}
