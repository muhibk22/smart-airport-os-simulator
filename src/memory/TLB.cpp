#include "TLB.h"

using namespace std;

TLB::TLB(int cap) {
    capacity = cap;
    hits = 0;
    misses = 0;
    pthread_mutex_init(&tlb_mutex, nullptr);
}

TLB::~TLB() {
    pthread_mutex_destroy(&tlb_mutex);
}

int TLB::make_key(int proc_id, int page_id) {
    // Simple key combining process and page
    return (proc_id << 16) | (page_id & 0xFFFF);
}

int TLB::lookup(int proc_id, int page_id) {
    pthread_mutex_lock(&tlb_mutex);
    
    int key = make_key(proc_id, page_id);
    auto it = cache_map.find(key);
    
    if (it != cache_map.end() && it->second->valid) {
        hits++;
        
        // Move to front (most recently used)
        TLBEntry entry = *(it->second);
        lru_list.erase(it->second);
        lru_list.push_front(entry);
        cache_map[key] = lru_list.begin();
        
        pthread_mutex_unlock(&tlb_mutex);
        return entry.frame_id;
    }
    
    misses++;
    pthread_mutex_unlock(&tlb_mutex);
    return -1;
}

void TLB::insert(int proc_id, int page_id, int frame_id) {
    pthread_mutex_lock(&tlb_mutex);
    
    int key = make_key(proc_id, page_id);
    
    // If already exists, update
    auto it = cache_map.find(key);
    if (it != cache_map.end()) {
        lru_list.erase(it->second);
        cache_map.erase(it);
    }
    
    // Evict LRU if full
    if ((int)lru_list.size() >= capacity) {
        TLBEntry& lru = lru_list.back();
        int lru_key = make_key(lru.process_id, lru.page_id);
        cache_map.erase(lru_key);
        lru_list.pop_back();
    }
    
    // Insert new entry at front
    TLBEntry entry;
    entry.page_id = page_id;
    entry.frame_id = frame_id;
    entry.process_id = proc_id;
    entry.valid = true;
    
    lru_list.push_front(entry);
    cache_map[key] = lru_list.begin();
    
    pthread_mutex_unlock(&tlb_mutex);
}

void TLB::invalidate(int proc_id, int page_id) {
    pthread_mutex_lock(&tlb_mutex);
    
    int key = make_key(proc_id, page_id);
    auto it = cache_map.find(key);
    if (it != cache_map.end()) {
        it->second->valid = false;
    }
    
    pthread_mutex_unlock(&tlb_mutex);
}

void TLB::flush_process(int proc_id) {
    pthread_mutex_lock(&tlb_mutex);
    
    auto it = lru_list.begin();
    while (it != lru_list.end()) {
        if (it->process_id == proc_id) {
            int key = make_key(it->process_id, it->page_id);
            cache_map.erase(key);
            it = lru_list.erase(it);
        } else {
            ++it;
        }
    }
    
    pthread_mutex_unlock(&tlb_mutex);
}

void TLB::flush_all() {
    pthread_mutex_lock(&tlb_mutex);
    cache_map.clear();
    lru_list.clear();
    pthread_mutex_unlock(&tlb_mutex);
}

double TLB::get_hit_rate() const {
    int total = hits + misses;
    if (total == 0) return 0.0;
    return (double)hits / total;
}
