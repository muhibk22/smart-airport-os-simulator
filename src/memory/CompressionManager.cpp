#include "CompressionManager.h"
#include <algorithm>

using namespace std;

CompressionManager::CompressionManager() {
    pages_compressed = 0;
    pages_decompressed = 0;
    bytes_saved = 0;
    pthread_mutex_init(&compress_mutex, nullptr);
}

CompressionManager::~CompressionManager() {
    pthread_mutex_destroy(&compress_mutex);
}

bool CompressionManager::is_compressible(Page* page) {
    if (page == nullptr) return false;
    
    // Simulate: cold data is more compressible
    DataTier tier = page->get_tier();
    return tier == TIER_L3 || tier == TIER_L4;
}

double CompressionManager::estimate_ratio(Page* page) {
    if (page == nullptr) return 1.0;
    
    // Simulate compression ratio based on tier
    switch (page->get_tier()) {
        case TIER_L4: return 0.3;  // Historical logs compress well
        case TIER_L3: return 0.5;
        case TIER_L2: return 0.7;
        case TIER_L1: return 0.9;  // Hot data doesn't compress well
        default: return 0.7;
    }
}

bool CompressionManager::compress_page(Page* page) {
    if (page == nullptr) return false;
    if (!is_compressible(page)) return false;
    if (page->get_state() == PAGE_COMPRESSED) return true;
    
    pthread_mutex_lock(&compress_mutex);
    
    int original_size = page->get_data_size();
    double ratio = estimate_ratio(page);
    int compressed_size = max(MIN_PAGE_SIZE, (int)(original_size * ratio));
    
    page->set_compressed_size(compressed_size);
    page->set_state(PAGE_COMPRESSED);
    
    pages_compressed++;
    bytes_saved += (original_size - compressed_size);
    
    pthread_mutex_unlock(&compress_mutex);
    return true;
}

bool CompressionManager::decompress_page(Page* page) {
    if (page == nullptr) return false;
    if (page->get_state() != PAGE_COMPRESSED) return true;
    
    pthread_mutex_lock(&compress_mutex);
    
    page->set_compressed_size(0);
    page->set_state(PAGE_VALID);
    pages_decompressed++;
    
    pthread_mutex_unlock(&compress_mutex);
    return true;
}

double CompressionManager::get_average_ratio() const {
    if (pages_compressed == 0) return 1.0;
    // Approximate based on bytes saved
    return 1.0 - ((double)bytes_saved / (pages_compressed * 4096));
}
