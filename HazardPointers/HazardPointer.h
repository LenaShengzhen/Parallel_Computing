#ifndef HAZARD_POINTER_H
#define HAZARD_POINTER_H

// Hazard pointer record
class HPRecType  {
private:
	int    active_;
	//  Global header of the HP list
	static HPRecType * pHead_;
	//  The length of the HP list
	static int listLen_;

public:
	HPRecType* pNext_;

	// Can be used by the thread that acquired it
	void  * pHazard_;

	// get the Global header of the HP list
	static HPRecType* Head();

	// Acquires one hazard pointer and set the active = 1.
	static HPRecType* Acquire();

	// Releases a hazard pointer
	static void Release(HPRecType* p);

    // After end all the thread, clean the node in Hazard pointer list.
	static void clean_allHPNode_after_endAllThread();
};

// void Scan(HPRecType* head);

static int get_thread_id();

void put_point2BeReleased_on_rlist(void * p);

// Before starting the thread, initialize rlist
void init_rlist_before_startingThread(unsigned int size);

// After end all the thread, clean the every thread's rlist
void clean_rlist_after_endAllThread(unsigned int thread_id);

#endif