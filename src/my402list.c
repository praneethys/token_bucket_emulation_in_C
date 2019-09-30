/*
 * Linked List implementation in C
 * Author: Praneeth Yerrapragada
 */

#include<stdlib.h>
#include<stdio.h>

#include"cs402.h"
#include"my402list.h"

extern int My402ListInit(My402List* list)
{
  list->anchor.next = &(list->anchor);
  list->anchor.prev = &(list->anchor);
  list->num_members = 0;
  return 1;
}

extern My402ListElem *My402ListFirst(My402List* list)
{
  if (list->num_members == 0) {
    return NULL;
  } else {
    return (My402ListElem*) list->anchor.next;
  }
}

extern My402ListElem *My402ListLast(My402List* list)
{
  if (list->num_members == 0) {
    return NULL;
  } else {
    return (My402ListElem*) list->anchor.prev;
  }
}

extern int My402ListLength(My402List* list)
{
  return list->num_members;
}

extern int My402ListEmpty(My402List* list)
{
  if (list->num_members == 0) {
    return 1;
  } else {
    return 0;
  }
}

extern My402ListElem *My402ListNext(My402List* list, My402ListElem* elem)
{
  if (elem == My402ListLast(list)) {
    return 0;
  } else {
    return elem->next;
  }
}

extern My402ListElem *My402ListPrev(My402List* list, My402ListElem* elem)
{
  if (elem == My402ListFirst(list)) {
    return 0;
  } else {
    return elem->prev;
  }
}

extern My402ListElem *My402ListFind(My402List* list, void* num)
{
  My402ListElem *elem = NULL;
  for (elem = My402ListFirst(list); elem != NULL;
      elem = My402ListNext(list, elem)) {
    if (elem->obj == num) {
      return elem;
    }
  }
  return 0;
}

extern int My402ListAppend(My402List* list, void* num)
{
  if (list->num_members == 0) {
    My402ListElem * elem = (My402ListElem*) malloc(sizeof(My402ListElem));
    if (elem == NULL) {
      fprintf(stderr, "Memory allocation unsuccessful.\n");
      exit(-1);
    }
    elem->prev = &(list->anchor);
    elem->next = &(list->anchor);
    list->anchor.next = elem;
    list->anchor.prev = elem;
    elem->obj = num;
    (list->num_members)++;
    return 1;
  } else {
    My402ListElem * elem = My402ListLast(list);
    My402ListElem * ptemp = NULL;
    elem->next = (My402ListElem*) malloc(sizeof(My402ListElem));
    if (elem->next == NULL) {
      fprintf(stderr, "Memory allocation unsuccessful.\n");
      exit(-1);
    }
    ptemp = elem;
    elem = elem->next;
    list->anchor.prev = elem;
    ptemp->next = elem;
    elem->obj = num;
    elem->prev = ptemp;
    elem->next = &(list->anchor);
    (list->num_members)++;
    return 1;
  }
  return 0;
}
extern int My402ListPrepend(My402List* list, void* num)
{
  if (list->num_members == 0) {
    My402ListElem * elem = (My402ListElem*) malloc(sizeof(My402ListElem));
    if (elem == NULL) {
      fprintf(stderr, "Memory allocation unsuccessful.\n");
      exit(-1);
    }
    elem->prev = &(list->anchor);
    elem->next = &(list->anchor);
    list->anchor.next = elem;
    list->anchor.prev = elem;
    elem->obj = num;
    (list->num_members)++;
    return 1;
  } else {
    My402ListElem * elem = My402ListFirst(list);
    My402ListElem * ptemp = NULL;
    elem->prev = (My402ListElem*) malloc(sizeof(My402ListElem));
    if (elem->prev == NULL) {
      fprintf(stderr, "Memory allocation unsuccessful.\n");
      exit(-1);
    }
    ptemp = elem;
    elem = elem->prev;
    list->anchor.next = elem;
    ptemp->prev = elem;
    elem->obj = num;
    elem->next = ptemp;
    elem->prev = &(list->anchor);
    (list->num_members)++;
    return 1;
  }
  return 0;
}
extern void My402ListUnlink(My402List* list, My402ListElem* elem)
{
  My402ListElem *ptemp = elem;
  My402ListElem *ptemp2 = elem->prev;
  if (list->num_members > 1) {
    if (elem == My402ListFirst(list)) {
      elem = elem->next;
      elem->prev = &(list->anchor);
      list->anchor.next = elem;
    } else if (elem == My402ListLast(list)) {
      elem = elem->prev;
      elem->next = &(list->anchor);
      list->anchor.prev = elem;
    } else {
      elem = elem->next;
      elem->prev = ptemp2;
      ptemp2->next = elem;
    }
    free(ptemp);
    ptemp = NULL;
  } else if (list->num_members == 1) {
    list->anchor.next = &(list->anchor);
    list->anchor.prev = &(list->anchor);
    free(elem);
    elem = NULL;
  }
  (list->num_members)--;
}

extern void My402ListUnlinkAll(My402List* list)
{
  My402ListElem * elem = My402ListFirst(list);
  My402ListElem * ptemp = elem;

  int i, len = My402ListLength(list);

  for (i = 0; i < len; i++) {
    ptemp = elem->next;
    My402ListUnlink(list, elem);
    elem = ptemp;
  }
}

extern int My402ListInsertAfter(My402List* list, void* num, My402ListElem* elem)
{
  My402ListElem * elem2;
  My402ListElem * elem3 = elem->next;
  if (elem != NULL) {
    elem2 = (My402ListElem*) malloc(sizeof(My402ListElem));
    if (elem2 == NULL) {
      fprintf(stderr, "Memory allocation unsuccessful.\n");
      exit(-1);
    }

    elem2->obj = num;
    elem2->prev = elem;
    elem2->next = elem3;
    elem3->prev = elem2;
    elem->next = elem2;
    (list->num_members)++;
    return 1;
  } else {
    return My402ListAppend(list, num);
  }
  return 0;
}

extern int My402ListInsertBefore(My402List* list, void* num,
    My402ListElem* elem)
{
  My402ListElem * elem2;
  My402ListElem * elem3 = elem->prev;
  if (elem != NULL) {
    elem2 = (My402ListElem*) malloc(sizeof(My402ListElem));
    if (elem2 == NULL) {
      fprintf(stderr, "Memory allocation unsuccessful.\n");
      exit(-1);
    }

    elem2->obj = num;
    elem2->prev = elem3;
    elem2->next = elem;
    elem3->next = elem2;
    elem->prev = elem2;
    (list->num_members)++;
    return 1;
  } else {
    return My402ListPrepend(list, num);
  }
  return 0;
}
