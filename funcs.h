#ifndef funcs
#define funcs
struct process {
  int pid; 
  long int start; // pointer to where processes should start in trace
  long int end; // pointer to end of process in trace
  unsigned long int time; // store next runnable time
  int ready; //process IO status
  void * root; //used for tsearch tree
};
struct process * makeProcess(char * pid, long int start);
void firstPass(FILE * file, long int * counter, int * spot, struct process ** p_arr, long int * fileEnd);
void removeProc(struct process * proc);
#endif