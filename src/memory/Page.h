#ifndef PAGE_H
#define PAGE_H

#include <pthread.h>

using namespace std;

// Page represents a unit of virtual memory
// In airport context: flight data, passenger manifests, baggage info

enum PageState {
    PAGE_INVALID,       // Not loaded
    PAGE_VALID,         // In memory
    PAGE_MODIFIED,      // Modified (dirty)
    PAGE_COMPRESSED     // Compressed in memory
};

enum DataTier {
    TIER_L1,    // Hot data - runway status, active flights (fast access)
    TIER_L2,    // Warm data - current passengers, baggage (medium)
    TIER_L3,    // Cool data - completed flights (slower)
    TIER_L4     // Cold data - historical logs (slowest, simulated disk)
};

class Page {
private:
    int page_id;
    int process_id;         // Flight ID owning this page
    PageState state;
    DataTier tier;
    
    // Access tracking for replacement algorithm
    int reference_count;
    bool reference_bit;     // For Clock algorithm
    bool modified_bit;      // Dirty bit
    long long last_access_time;
    long long load_time;
    
    // Content metadata
    int data_size;          // Bytes of actual data
    int compressed_size;    // If compressed
    
    pthread_mutex_t page_mutex;
    
public:
    Page(int id, int proc_id);
    ~Page();
    
    // Access methods
    void access(long long current_time);
    void modify();
    
    // State management
    void set_state(PageState new_state);
    PageState get_state() const { return state; }
    
    void set_tier(DataTier new_tier);
    DataTier get_tier() const { return tier; }
    
    // Clock algorithm support
    bool get_reference_bit() const { return reference_bit; }
    void clear_reference_bit() { reference_bit = false; }
    void set_reference_bit() { reference_bit = true; }
    
    bool get_modified_bit() const { return modified_bit; }
    void clear_modified_bit() { modified_bit = false; }
    
    // Getters
    int get_page_id() const { return page_id; }
    int get_process_id() const { return process_id; }
    int get_reference_count() const { return reference_count; }
    long long get_last_access_time() const { return last_access_time; }
    long long get_load_time() const { return load_time; }
    int get_data_size() const { return data_size; }
    int get_compressed_size() const { return compressed_size; }
    
    // Setters
    void set_load_time(long long time) { load_time = time; }
    void set_data_size(int size) { data_size = size; }
    void set_compressed_size(int size) { compressed_size = size; }
};

#endif // PAGE_H
