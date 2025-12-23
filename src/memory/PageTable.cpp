#include "PageTable.h"

using namespace std;

PageTable::PageTable(int proc_id) {
    process_id = proc_id;
    page_faults = 0;
    page_hits = 0;
    pthread_mutex_init(&table_mutex, nullptr);
}

PageTable::~PageTable() {
    pthread_mutex_destroy(&table_mutex);
}

PageTableEntry* PageTable::lookup(int page_id) {
    pthread_mutex_lock(&table_mutex);
    
    auto it = entries.find(page_id);
    if (it != entries.end() && it->second.valid) {
        page_hits++;
        it->second.accessed = true;
        pthread_mutex_unlock(&table_mutex);
        return &(it->second);
    }
    
    page_faults++;
    pthread_mutex_unlock(&table_mutex);
    return nullptr;
}

void PageTable::add_entry(int page_id, int frame_id) {
    pthread_mutex_lock(&table_mutex);
    
    PageTableEntry entry;
    entry.page_id = page_id;
    entry.frame_id = frame_id;
    entry.valid = true;
    entry.dirty = false;
    entry.accessed = true;
    entry.protection = 0x7;  // RWX
    
    entries[page_id] = entry;
    
    pthread_mutex_unlock(&table_mutex);
}

void PageTable::remove_entry(int page_id) {
    pthread_mutex_lock(&table_mutex);
    entries.erase(page_id);
    pthread_mutex_unlock(&table_mutex);
}

void PageTable::invalidate_entry(int page_id) {
    pthread_mutex_lock(&table_mutex);
    auto it = entries.find(page_id);
    if (it != entries.end()) {
        it->second.valid = false;
    }
    pthread_mutex_unlock(&table_mutex);
}

void PageTable::mark_accessed(int page_id) {
    pthread_mutex_lock(&table_mutex);
    auto it = entries.find(page_id);
    if (it != entries.end()) {
        it->second.accessed = true;
    }
    pthread_mutex_unlock(&table_mutex);
}

void PageTable::mark_dirty(int page_id) {
    pthread_mutex_lock(&table_mutex);
    auto it = entries.find(page_id);
    if (it != entries.end()) {
        it->second.dirty = true;
    }
    pthread_mutex_unlock(&table_mutex);
}

void PageTable::clear_accessed_bits() {
    pthread_mutex_lock(&table_mutex);
    for (auto& pair : entries) {
        pair.second.accessed = false;
    }
    pthread_mutex_unlock(&table_mutex);
}

double PageTable::get_fault_rate() const {
    int total = page_faults + page_hits;
    if (total == 0) return 0.0;
    return (double)page_faults / total;
}

vector<int> PageTable::get_valid_pages() {
    vector<int> valid;
    pthread_mutex_lock(&table_mutex);
    for (const auto& pair : entries) {
        if (pair.second.valid) {
            valid.push_back(pair.first);
        }
    }
    pthread_mutex_unlock(&table_mutex);
    return valid;
}

int PageTable::get_entry_count() {
    pthread_mutex_lock(&table_mutex);
    int count = entries.size();
    pthread_mutex_unlock(&table_mutex);
    return count;
}
