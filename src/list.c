#include "../include/list.h"

page *page_create (int addr) {
  page *p = (page*)malloc(sizeof(page));
  p->addr = addr;
  p->next = NULL;
  p->prev = NULL;
  p->l = NULL;
  p->type = 0;
  return p;
}

void page_free (page *p) {
  free(p);
}

list *list_create () {
  list *l = (list*)malloc(sizeof(list));
  l->head = NULL;
  l->tail = NULL;
  l->size = 0;
  return l;
}

void list_free (list *l) {
  page *temp = l->head, *next;

  while (temp != NULL) {
    next = temp->next;
    page_free(temp);
    temp = next;
  }

  free(l);
}

page *list_remove (list *l, page *p) {

  // remove p from list
  if (l->size == 1) {
    l->head = NULL;
    l->tail = NULL;
  } else {
    if (l->tail == p) {
      l->tail = p->prev;
      l->tail->next = NULL;
    } else if (l->head == p) {
      l->head = p->next;
      l->head->prev = NULL;
    } else {
      p->prev->next = p->next;
      p->next->prev = p->prev;
    }
  }

  p->l = NULL;

  l->size--;

  return p;
}

page *list_pop_back (list *l) {
  if (l->size == 0) {
    return NULL;
  } else {
    return list_remove(l, l->tail);
  }
}

void list_push_front (list *l, page *p) {

  // insert at front
  if (l->size == 0) {
    l->head = p;
    l->tail = p;
  } else {
    p->next = l->head;
    l->head->prev = p;
    l->head = p;
    p->prev = NULL;
  }

  p->l = l;

  l->size++;
}


