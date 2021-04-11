## HazardPointers

A memory reclamation mechanism for lock-free objects.

### paper-version

This version is implemented according to the design and code in the paper [2].

- Hazard pointers(class HPRecType) : Used to store the memory object currently being accessed by this thread, the memory object being accessed cannot be released by any thread
  - Hazard pointers are single write and multiple read
  - Add clean_allHPNode_after_endAllThread()
- retire list(rlist) : The memory object deleted by this thread, but has not been released 
  - retire list is single write and single read
  - Add clean_rlist_after_endAllThread()

#### data structure

- rlist is a vector, insert from the end and delete from the beginning. When deleting, use swap and then pop_back to improve the efficiency of vector.
- hazardPoint is implemented as a linked list. The addresses to be protected by all threads are in this  linked list.
  - Efficiency of applying for a new node: Each time a new node is applied for, the first inactive node in the linked list is searched and returned. If it is not found, a new node is created and inserted at the head of the linked list.
  - Scan efficiency: In the paper[2], the values of the linked list are stored in the array, then the array is sorted, and then binary search is in it.









## Reference



[2] Andrei Alexandrescu, Maged Michael  "**Lock-Free Data Structures with Hazard Pointers**". 2004.