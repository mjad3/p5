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
int frames;
int size;

struct node * hand; //track hand for clock algo

void init(int f) {
  frames = f;
  size = 0;
}

struct node {
  int vpn;
  int ppn;
  struct node * next;
  struct node * prev;
  int r;
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
  n -> r = 0;
  return n;
}

void add(struct node * page) {
  if (size == 0) {
    hand = page;
    hand -> next = page;
    hand -> prev = page;
  }

  //set pages prev and next
  page -> prev = hand;
  page -> next = hand -> next;
  //set previous for next node

  hand -> next -> prev = page;
  //set next for last node
  hand -> next = page;
  page -> r = 1;
  size++;
  hand = page;
}

void removeNode(struct node * page) {
  if (page -> next && page -> prev) {
    size--;
    if (size == 0) {
      hand = NULL;
    } else {
      if (hand == page) {
        hand = page -> next;
      }
      page -> prev -> next = page -> next;
      page -> next -> prev = page -> prev;
    }

    page -> prev = NULL;
    page -> next = NULL;
    page -> r = 0;
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
    if ((n -> next && n -> prev)) { //if in mem
      n -> r = 1;
      hand = n;
      return 0;
    }
  }

  //add page to mem is IO is done/ready
  if (size < frames && ready) {
    add(n);
  } else if (ready) {
    while (hand -> r != 0) {
      hand -> r = 0;
      hand = hand -> next;
    }
    struct node * temp = hand;
    removeNode(hand);
    tdelete(temp, root, compare);
    add(n);
  } else {
    //return fault if not in mem and IO is not complete  
    return 1;
  }

  return 0;
}