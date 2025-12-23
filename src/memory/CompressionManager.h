#ifndef COMPRESSION_MANAGER_H
#define COMPRESSION_MANAGER_H

#include "Page.h"
#include <pthread.h>

using namespace std;

// CompressionManager handles page compression to increase effective memory

class CompressionManager {
private:
    static constexpr double TARGET_RATIO = 0.5;   // Target 50% compression
    static constexpr int MIN_PAGE_SIZE = 512;     // Minimum compressed size
    
    int pages_compressed;
    int pages_decompressed;
    long long bytes_saved;
    
    pthread_mutex_t compress_mutex;
    
public:
    CompressionManager();
    ~CompressionManager();
    
    // Compress a page (simuates compression)
    bool compress_page(Page* page);
    
    // Decompress a page
    bool decompress_page(Page* page);
    
    // Check if page is compressible (based on data patterns)
    bool is_compressible(Page* page);
    
    // Estimate compression ratio for page
    double estimate_ratio(Page* page);
    
    // Statistics
    int get_pages_compressed() const { return pages_compressed; }
    long long get_bytes_saved() const { return bytes_saved; }
    double get_average_ratio() const;
};

#endif // COMPRESSION_MANAGER_H
