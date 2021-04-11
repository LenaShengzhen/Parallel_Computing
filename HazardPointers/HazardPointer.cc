#include "HazardPointer.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <assert.h>

using namespace std;

// rlist's max size, then scan Hazard pointer and releases
const int M_SIZE = 100;

// Per-thread private variable
vector<vector<void *>> rlist;

static int _tidSeed = 0;

HPRecType* HPRecType::pHead_ = NULL;

// recored the length of Hazard pointer list.
int HPRecType::listLen_ = 0;

HPRecType* HPRecType::Head() {
    return pHead_;
}

// Releases a hazard pointer
void HPRecType::Release(HPRecType* p) {
    p->pHazard_ = 0;
    p->active_ = 0;
}

HPRecType* HPRecType::Acquire() {
    // Try to reuse a retired HP record 
    HPRecType * p = pHead_;
    for (; p; p = p->pNext_) {
        if (p->active_ || !__sync_bool_compare_and_swap(&p->active_, 0, 1))
            continue;
        // Got one!
        return p;
    }

    // Increment the list length
    int oldLen;
    do {
        oldLen = listLen_;
    } while (!__sync_bool_compare_and_swap(&listLen_, oldLen, oldLen + 1));

    // Allocate a new one
    p = new HPRecType;
    p->active_ = 1;
    p->pHazard_ = 0;

    // Push it to the front
    HPRecType* old;
    do {
        old = pHead_;
        p->pNext_ = old;
    } while (!__sync_bool_compare_and_swap(&pHead_, old, p));
    return p;
}

// After end all the thread, clean the node in Hazard pointer list.
void HPRecType::clean_allHPNode_after_endAllThread() {
    // cout << "listLen_ = " << listLen_ << endl;
    HPRecType * p = pHead_;
    HPRecType * delep;
    while (p) {
        delep = p;
        p = p->pNext_;
        free(delep);
        listLen_--;
    }
    pHead_ = NULL;
}

void Scan(HPRecType * head) {
    // Stage 1: Scan hazard pointers list
    // collecting all non-null ptrs
    vector<void*> hp;
    while (head) {
        void * p = head->pHazard_;
        if (p) hp.push_back(p);
        head = head->pNext_;
    }

    // Stage 2: sort the hazard pointers
    sort(hp.begin(), hp.end());

    // Stage 3: Search for'em!
    int id = get_thread_id();
    for(int i = 0; i < rlist[id].size(); i++) {
        void * address = rlist[id][i];
        if(!binary_search(hp.begin(), hp.end(), address)) {
            free(address);
            int n  = rlist[id].size();
            swap(rlist[id][i], rlist[id][n-1]);
            rlist[id].pop_back();
            i--;
        }
    }
}

int get_thread_id() {
    static __thread int _tid = -1;
    if (_tid >= 0) return _tid;
    _tid = __sync_fetch_and_add(&_tidSeed, 1);
    return _tid;
}

void init_rlist_before_startingThread(unsigned int size) {
    _tidSeed = 0;
    rlist = vector<vector<void *>>(size, vector<void *>());
}

void put_point2BeReleased_on_rlist(void * p) {
    int id = get_thread_id();
    rlist[id].push_back(p);

    if (rlist[id].size() >= M_SIZE) {
        Scan(HPRecType::Head());
    }
}

void clean_rlist_after_endAllThread(unsigned int id) {
    for(int i = rlist[id].size() - 1; i >= 0; i--) {
        void * address = rlist[id][i];
        free(address);
        rlist[id].pop_back();
    }
}
