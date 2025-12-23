#include "Page.h"

using namespace std;

Page::Page(int id, int proc_id) {
    page_id = id;
    process_id = proc_id;
    state = PAGE_INVALID;
    tier = TIER_L2;  // Default to warm data
    
    reference_count = 0;
    reference_bit = false;
    modified_bit = false;
    last_access_time = 0;
    load_time = 0;
    
    data_size = 4096;      // 4KB default page size
    compressed_size = 0;
    
    pthread_mutex_init(&page_mutex, nullptr);
}

Page::~Page() {
    pthread_mutex_destroy(&page_mutex);
}

void Page::access(long long current_time) {
    pthread_mutex_lock(&page_mutex);
    
    reference_count++;
    reference_bit = true;
    last_access_time = current_time;
    
    pthread_mutex_unlock(&page_mutex);
}

void Page::modify() {
    pthread_mutex_lock(&page_mutex);
    
    modified_bit = true;
    state = PAGE_MODIFIED;
    
    pthread_mutex_unlock(&page_mutex);
}

void Page::set_state(PageState new_state) {
    pthread_mutex_lock(&page_mutex);
    state = new_state;
    pthread_mutex_unlock(&page_mutex);
}

void Page::set_tier(DataTier new_tier) {
    pthread_mutex_lock(&page_mutex);
    tier = new_tier;
    pthread_mutex_unlock(&page_mutex);
}
