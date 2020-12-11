This program simulates the paging system, specifically the page table system interfacing with memory.
We have 3 functional executables that simulate different page replacement algorithms
--> main.c functions as main driver, with parsing functionality handled in funcs.
We use tsearch to search for pages in memory stored as nodes in a binary tree. Memory is ordered using a doubly linked list.
12million.addrtrace takes a longer runtime of one to one and a half minutes
Thanks for Grading