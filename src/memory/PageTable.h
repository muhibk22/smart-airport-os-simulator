#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "Page.h"
#include "PageFrame.h"
#include <vector>
#include <unordered_map>
#include <pthread.h>

using namespace std;

// PageTableEntry maps virtual page to physical frame
struct PageTableEntry {
    int page_id;
    int frame_id;
    bool valid;
    bool dirty;
    bool accessed;
    int protection;  // Read/Write/Execute bits
};

class PageTable {
private:
    int process_id;  // Flight ID
    unordered_map<int, PageTableEntry> entries;
    pthread_mutex_t table_mutex;
    
    // Page fault statistics
    int page_faults;
    int page_hits;
    
public:
    PageTable(int proc_id);
    ~PageTable();
    
    // Lookup page in table
    PageTableEntry* lookup(int page_id);
    
    // Add/update entry
    void add_entry(int page_id, int frame_id);
    void remove_entry(int page_id);
    void invalidate_entry(int page_id);
    
    // Mark as accessed/dirty
    void mark_accessed(int page_id);
    void mark_dirty(int page_id);
    void clear_accessed_bits();
    
    // Statistics
    void record_fault() { page_faults++; }
    void record_hit() { page_hits++; }
    int get_page_faults() const { return page_faults; }
    int get_page_hits() const { return page_hits; }
    double get_fault_rate() const;
    
    // Get all valid entries
    vector<int> get_valid_pages();
    int get_entry_count();
    
    int get_process_id() const { return process_id; }
};

#endif // PAGE_TABLE_H
