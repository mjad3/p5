/////////////////////////////////////////////////////////////
// CS 537, Fall 2020
// Program 4, Page Replacement Simulator
// Michael Sachen, 9073631716, sachen
// Matt Jadin, 9065235468, jadin
/////////////////////////////////////////////////////////////
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <search.h>
#include <stdlib.h>
#include "replacement.h"

/* We have each process stored as a data structure to keep track of metadata */
struct process {
  int pid;
  long int start; // pointer to where processes should start in trace
  long int end; // pointer to end of process in trace
  unsigned long int time; // store next runnable time
  int ready; //process IO status
  void * root; //used for tsearch tree
};

/* Initialize Process as we read them in */
struct process * makeProcess(char * pid, long int start) {
  struct process * e = (struct process * ) calloc(1, sizeof(struct process));
  if (!e) {
    printf("memory failure, exiting\n");
    exit(EXIT_FAILURE);
  }
  e -> pid = atoi(pid);
  e -> start = start;
  e -> ready = 0;
  e -> time = 0;
  e -> root = NULL;
  return e; // Return pointer to 
}

/* This is the first pass of file parsing.
It counts and inits the PID table*/
void firstPass(FILE * file, long int * counter, int * spot, struct process ** p_arr, long int * fileEnd) {
  char line[100];
  while (fgets(line, sizeof(line), file)) {
    ( * counter) ++; //Increment line counter
    long int lineStart = ftell(file) - strlen(line); // Get Byte start

    char * pid = calloc(1, sizeof(char) * 10); // PID Buffer
    if (!pid) {
      printf("memory failure, exiting\n");
      exit(EXIT_FAILURE);
    }
    int d = 0;
    int z = 0;
    char t; // Temp Char checking

    // Checking Validity of Line
    for (int x = 0; x < strlen(line); x++) {
      t = line[x];
      // Breaking for whitespace or end of line
      if (t == EOF || t == '\n') break;
      if ((t == ' ' || t == '\t') && z == 1) break;
      if (t != ' ' && t != '\t') {
        pid[d] = t;
        d++;
        z = 1;
      }
    }

    int c = 0; //tracks unused PIDs
    //set pid trace endpoint
    for (int x = 0; x < * spot; x++) {
      if (p_arr[x] -> pid == atoi(pid)) {
        c = 1;
        p_arr[x] -> end = ftell(file);
        * fileEnd = ftell(file);
      }
    }
    if (c != 1) {
      p_arr[ * spot] = makeProcess(pid, lineStart);
      ( * spot) ++;
      c = 0;
    }

  }

}

/*  Removes Process from memory*/
void removeProc(struct process * proc) {
  if (proc -> pid != -1) deleteTree(proc -> root);
  proc -> pid = -1;
}