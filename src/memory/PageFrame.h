#ifndef PAGE_FRAME_H
#define PAGE_FRAME_H

#include "Page.h"
#include <pthread.h>

using namespace std;

// PageFrame represents a slot in physical memory

class PageFrame {
private:
    int frame_id;
    Page* resident_page;    // Page currently in this frame
    bool is_free;
    bool is_pinned;         // Cannot be evicted (e.g., active flight data)
    
    // For Clock algorithm
    long long load_time;
    
    pthread_mutex_t frame_mutex;
    
public:
    PageFrame(int id);
    ~PageFrame();
    
    // Load page into frame
    bool load_page(Page* page, long long current_time);
    
    // Evict page from frame
    Page* evict_page();
    
    // Pin/unpin frame
    void pin() { is_pinned = true; }
    void unpin() { is_pinned = false; }
    
    // Getters
    int get_frame_id() const { return frame_id; }
    Page* get_page() const { return resident_page; }
    bool is_empty() const { return is_free; }
    bool get_pinned() const { return is_pinned; }
    long long get_load_time() const { return load_time; }
};

#endif // PAGE_FRAME_H
