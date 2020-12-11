/////////////////////////////////////////////////////////////
// CS 537, Fall 2020
// Program 4, Page Replacement
// Michael Sachen, 9073631716, sachen
// Matt Jadin, 9065235468, jadin
/////////////////////////////////////////////////////////////
#define _GNU_SOURCE
#include <stdlib.h>

#include <unistd.h>



#include <stdio.h>

#include <search.h>

#include <time.h>
#include <string.h>

 // Some queue variable

struct node * tail;
struct node * head;
int frames;
int size;

void init(int f) {
  frames = f;
  size = 0;
}

struct node {
  int pid;
  int vpn;
  int ppn;
  struct node * next;
  struct node * prev;
  long int pos;
};

struct node * makeNode(int pid, int vpn, long int pos) {
  struct node * n = (struct node * ) calloc(1, sizeof(struct node));
  if (!n) {
    printf("memory failure, exiting\n");
    exit(EXIT_FAILURE);
  }
  n -> pid = pid;
  n -> vpn = vpn;
  n -> next = NULL;
  n -> prev = NULL;
  n->pos = pos;
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

  size--;

  if (size == 0) {
    head = NULL;
    tail = NULL;
    page -> prev = NULL;
    page -> next = NULL;
  } else if (page -> prev != NULL) { //if removing from middle/end of list

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

void action(const void * nodep, VISIT which, int depth) {
  struct node * a = (struct node * ) nodep;
  if (a -> next != NULL || a -> prev != NULL) {

    removeNode(a);
  }
}

void deleteTree(void * root) {
  twalk(root, action); //change order?  
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
  struct node * ptr = makeNode(pid, vpn, pos);
  //search tree
  
  void * t = tfind(ptr, root, compare);
  if (t == NULL) {
    void * r = tsearch(ptr, root, compare);
    if (r == NULL) exit(EXIT_FAILURE);
    n = * (struct node ** ) r;

  } else {
    n = * (struct node ** ) t;
    n->pos = pos;
    free(ptr);
  }
  
  if (n -> next == NULL && n -> prev == NULL) {
    if (size < frames && ready) {
      add(n);
    } else if (ready) {
      //iterate through file to find last use
      printf("pid: %d\n", pid);
      struct node * temp = head; //starting point to iterate through linked list
      struct node * remove = head;
      long int max = temp -> pos;
      while (temp != tail) {
        fseek(file, pos, SEEK_SET);
        char line[100];
        while (fgets(line, sizeof(line), file)) {
          //parse file
          char * p = calloc(1, sizeof(char) * 10); //pid
          if (!p) {
            printf("memory failure, exiting\n");
            exit(EXIT_FAILURE);
          }
          char * v = calloc(1, sizeof(char) * 10); //vpn
          if (!v) {
            printf("memory failure, exiting\n");
            exit(EXIT_FAILURE);
          }
          int d = 0; //for keeping track of place of char in string
          int z = 0; //tracks seperation of pid and vpn in line
          char t;

          //parse pid and vpn of line
          for (int x = 0; x < strlen(line); x++) {
            t = line[x];
            if (t == EOF || t == '\n') break;
            if ((t == ' ' || t == '\t') && z == 1) {
              z++;
              d = 0;
            }
            if (t != ' ' && t != '\t') {
              if (z <= 1) {
                p[d] = t;
                d++;
                z = 1;
              } else if (z > 1) {
                v[d] = t;
                d++;
              }
            }
          }

          // Set up more vars

          int t_pid = atoi(p);
          int t_vpn = atoi(v);
          //if this node's pid and vpn match, check if position is further in the file
          if(temp->pid == t_pid && temp->vpn == t_vpn && ftell(file) > max){ 
            max =  ftell(file);
            remove = temp;
            break;
          }
          
        }
        temp = temp->next; //go to next node
      }
      fseek(file, pos, SEEK_SET);
      printf("pid: %d\n", remove->pid);
      removeNode(remove);
      tdelete(remove, root, compare);
      add(n);
    } else {
      return 1;
    }
  }
  return 0;
}