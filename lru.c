/////////////////////////////////////////////////////////////
// CS 537, Fall 2020
// Program 4, Page Replacement
// Michael Sachen, 9073631716, sachen
// Matt Jadin, 9065235468, jadin
/////////////////////////////////////////////////////////////
#define _GNU_SOURCE 
#include <stdlib.h>
#include <stdio.h>
#include <search.h>
#include <time.h>

// Some queue variable
int frames = 256;
int size = 0;
struct node * tail;
struct node * head;

void init(int f) {
  frames = f;
  size = 0;
}

struct node {
  int vpn;
  int ppn;
  struct node * next;
  struct node * prev;
};

struct node * makeNode(int vpn) {
  struct node * n = (struct node * ) malloc(sizeof(struct node));
  if (!n) {
    printf("memory failure, exiting\n");
    exit(EXIT_FAILURE);
  }
  n -> vpn = vpn;
  n -> next = NULL;
  n -> prev = NULL;
  return n;
}

void add(struct node * page) {
  if (size == 0) {
    head = page;
  } else {
    page -> prev = tail;
    tail -> next = page;
  }
  tail = page;
  size++;

}

void removeNode(struct node * page) {
  if (page -> next || page -> prev) {
    size--;

    if (size == 0) {
      head = NULL;
      tail = NULL;
      page -> prev = NULL;
      page -> next = NULL;
    } else if (page -> prev != NULL) { //if removing from middle of list

      if (tail == page) {
        tail = page -> prev;
        page -> prev -> next = NULL;
      } else {
        page -> prev -> next = page -> next;
      }
      page -> prev = NULL;
      page -> next = NULL;

    } else { //if removing from front of list

      head = page -> next;

      head -> prev = NULL;

      page -> next = NULL;
      page -> prev = NULL;

    }
  }
}

void action(const void * nodep, VISIT which, int depth) {
  struct node * a = (struct node * ) nodep;
  removeNode(a);
}

void deleteTree(void * root) {
  twalk(root, action);
}

int compare(const void * pa,
  const void * pb) {
  struct node * a = (struct node * ) pa;
  struct node * b = (struct node * ) pb;

  if (a -> vpn < b -> vpn)
    return -1;
  if (a -> vpn > b -> vpn)
    return 1;
  return 0;
}

int run(int pid, int vpn, void ** root, int ready, FILE * file, long int pos) {

  struct node * n;
  //create a node
  struct node * ptr = makeNode(vpn);
  //search tree
  void * t = tfind(ptr, root, compare);
  if (t == NULL) {
    void * r = tsearch(ptr, root, compare);
    if (r == NULL) exit(EXIT_FAILURE);
    n = * (struct node ** ) r;
  } else {
    n = * (struct node ** ) t;
    free(ptr);
  }

  //Move recently used node to end of list
  if (n -> next != NULL) {
    removeNode(n);
    add(n);
    return 0;
  } else if (tail == n || head == n) {
    return 0;
  }
  if (n -> next == NULL && tail != n) {
    if (size < frames && ready) {
      add(n);
    } else if (ready) {
      struct node * temp = head;
      removeNode(head);
      tdelete(temp, root, compare);
      add(n);
    } else {
      return 1;
    }
  }
  return 0;
}