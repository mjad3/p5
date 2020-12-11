/////////////////////////////////////////////////////////////
// CS 537, Fall 2020
// Program 4, Page Replacement
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
#include "replacement.h"
#include "funcs.h"

int main(int argc, char * argv[]) {
  long int * fileEnd = malloc(sizeof(long int));
  if (!fileEnd) {
    printf("memory failure, exiting\n");
    exit(EXIT_FAILURE);
  }

  struct process * p_arr[50];
  // Open File
  FILE * file = fopen(argv[1], "r");

  long int * counter = malloc(sizeof(long int));
  if (!counter) {
    printf("memory failure, exiting\n");
    exit(EXIT_FAILURE);
  }
  int * spot = malloc(sizeof(int)); //tracks number of PIDs
  if (!spot) {
    printf("memory failure, exiting\n");
    exit(EXIT_FAILURE);
  }
  int page_size = 4096; // Size of frame in Bytes
  int mem_size = 1; // Size of memory in MB
  //arg parsing
  int arg = 0;
  while ((arg = getopt(argc, argv, "pm")) != -1) {
    switch (arg) {
    case 'p':
      printf("%d", arg);
      page_size = arg;
      break;
    case 'm':
      printf("%d", arg);
      mem_size = arg;
      break;
    }
  }

  int frames = mem_size * (1048576) / page_size;
  init(frames);

  //stats variables
  long int AMU = 0;
  long int ARP = 0;
  long int TPI = 0;
  unsigned long int clock = 0; //global clock in ns

  // First File Iteration
  // -----------------------------------------------------------------------

  firstPass(file, counter, spot, p_arr, fileEnd); 
  rewind(file);
  // End of First Pass
  // -----------------------------------------------------------------------
  // Start of Second Pass
  char line[100];
  while (fgets(line, sizeof(line), file)) {

    // Setting up vars for iterators
    long int lineStart = ftell(file) - strlen(line);
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

    int pid = atoi(p);
    int vpn = atoi(v);

    // find matching pid
    for (int x = 0; x < * spot; x++) {
      if (p_arr[x] -> pid == pid) {

        // And if the process is runnable
        if (p_arr[x] -> time <= clock && p_arr[x] -> start == lineStart) {

          // Run the Process --> This passes the paramaters to our algorihtm
          int t = run(pid, vpn, & (p_arr[x] -> root), p_arr[x] -> ready, file, ftell(file));
          clock++;
         
          //update AMU
          AMU += (size / frames);
          //update ARP
          for (int j = 0; j < ( * spot); j++) {
            if (p_arr[j] -> time <= clock && p_arr[j] -> pid != -1) ARP++;
          }
          // If the process gets blocked/page fault (data structure is full and has to goto i.o)
          if (t == 1) {

            TPI++;
            //set new start point for process && set ready for io
            p_arr[x] -> start = lineStart;
            p_arr[x] -> ready = 1;
            //finds next available time slot in io queue
            long int maxTime = clock;

            for (int j = 0; j < * spot; j++) {
              if (p_arr[j] -> time > maxTime) maxTime = p_arr[j] -> time;
            }
            p_arr[x] -> time = maxTime + 2000000; // increment time for io for this process

          } else { // no fault... does not have to increment i.o.               
            p_arr[x] -> start = ftell(file);
            if (p_arr[x] -> end <= p_arr[x] -> start && p_arr[x] -> pid != -1) {
              p_arr[x] -> pid = -1;
              deleteTree(p_arr[x] -> root);
            }
            p_arr[x] -> ready = 0;
          }
        }
      }
    }

    for (int x = 0; x < * spot; x++) {
      if ((p_arr[x] -> pid != pid) && p_arr[x] -> start == lineStart) {
        p_arr[x] -> start = ftell(file);
        if (p_arr[x] -> start >= p_arr[x] -> end) {
          removeProc(p_arr[x]);
        }
      }
    }

    //switch to runnable process, move line pointer to its start        
    long int min = * fileEnd; //init minimum start point to eof for comparison
    int runnable = -2;
    int pTime; //index of process with next io time ending
    long int minTime = clock + 2000000; // sets min possible time of next io completion

    // For each process
    for (int j = 0; j < * spot; j++) {
      // If the process time is less than the next low time, set time slot to jump to
      // Or find earliest runnable process in trace file
      if (p_arr[j] -> time <= clock && p_arr[j] -> start < min) {
        min = p_arr[j] -> start;
        runnable = j;
      } else if (p_arr[j] -> time <= minTime && p_arr[j] -> start < min) {
        minTime = p_arr[j] -> time;
        pTime = j;
      }
    } // If none are runnable

    if (runnable == -2) {

      if (p_arr[pTime] -> start < * fileEnd && minTime > clock) {
        AMU += ((minTime - clock) * size / frames);
        // increment clock, jump to process
        clock = minTime;
        fseek(file, p_arr[pTime] -> start, SEEK_SET);
      }
    } else {
      //next proc is p_arr[runnable];      		
      fseek(file, p_arr[runnable] -> start, SEEK_SET);
    }
  }
  // End of Second Pass
  // -----------------------------------------------------------------------

  float a = (float) AMU / (float) clock;
  float r = (float) ARP / clock;
  printf("AMU: %f, ARP: %f, TMR: %ld, TPI: %ld, RTime: %ld\n", a, r, * counter, TPI, clock);
  //free binary tree
  for (int j = 0; j < * spot; j++) {
    free(p_arr[j]);
  }
  fclose(file);
  free(fileEnd);
  free(counter);

  return 0;
}