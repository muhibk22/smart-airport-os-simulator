#include "PageFrame.h"

using namespace std;

PageFrame::PageFrame(int id) {
    frame_id = id;
    resident_page = nullptr;
    is_free = true;
    is_pinned = false;
    load_time = 0;
    
    pthread_mutex_init(&frame_mutex, nullptr);
}

PageFrame::~PageFrame() {
    pthread_mutex_destroy(&frame_mutex);
}

bool PageFrame::load_page(Page* page, long long current_time) {
    pthread_mutex_lock(&frame_mutex);
    
    if (!is_free) {
        pthread_mutex_unlock(&frame_mutex);
        return false;
    }
    
    resident_page = page;
    is_free = false;
    load_time = current_time;
    
    if (page) {
        page->set_state(PAGE_VALID);
        page->set_load_time(current_time);
    }
    
    pthread_mutex_unlock(&frame_mutex);
    return true;
}

Page* PageFrame::evict_page() {
    pthread_mutex_lock(&frame_mutex);
    
    if (is_pinned || is_free) {
        pthread_mutex_unlock(&frame_mutex);
        return nullptr;
    }
    
    Page* evicted = resident_page;
    if (evicted) {
        evicted->set_state(PAGE_INVALID);
    }
    
    resident_page = nullptr;
    is_free = true;
    load_time = 0;
    
    pthread_mutex_unlock(&frame_mutex);
    return evicted;
}
