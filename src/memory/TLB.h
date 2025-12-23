#ifndef TLB_H
#define TLB_H

#include <unordered_map>
#include <list>
#include <pthread.h>

using namespace std;

// Translation Lookaside Buffer - fast page table cache
// Uses LRU replacement policy

struct TLBEntry {
    int page_id;
    int frame_id;
    int process_id;
    bool valid;
};

class TLB {
private:
    int capacity;
    unordered_map<int, list<TLBEntry>::iterator> cache_map;
    list<TLBEntry> lru_list;  // Front = most recent, Back = least recent
    
    pthread_mutex_t tlb_mutex;
    
    // Statistics
    int hits;
    int misses;
    
    // Make key from process_id and page_id
    int make_key(int proc_id, int page_id);
    
public:
    TLB(int cap = 64);  // Default 64 entries
    ~TLB();
    
    // Lookup - returns frame_id or -1 if miss
    int lookup(int proc_id, int page_id);
    
    // Insert new mapping
    void insert(int proc_id, int page_id, int frame_id);
    
    // Invalidate entry
    void invalidate(int proc_id, int page_id);
    
    // Flush all entries for a process
    void flush_process(int proc_id);
    
    // Flush entire TLB
    void flush_all();
    
    // Statistics
    int get_hits() const { return hits; }
    int get_misses() const { return misses; }
    double get_hit_rate() const;
};

#endif // TLB_H
