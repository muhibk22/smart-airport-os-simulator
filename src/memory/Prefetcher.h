#ifndef PREFETCHER_H
#define PREFETCHER_H

#include "Page.h"
#include <vector>
#include <unordered_map>
#include <pthread.h>

using namespace std;

// Prefetcher implements predictive prefetching using pattern detection
// Tracks access patterns and prefetches likely next pages

struct AccessPattern {
    vector<int> sequence;   // Recent page access sequence
    int next_predicted;     // Predicted next page
    int confidence;         // Prediction confidence (0-100)
};

class Prefetcher {
private:
    static constexpr int PATTERN_LENGTH = 4;   // History length to track
    static constexpr int MIN_CONFIDENCE = 60;  // Min confidence to prefetch
    
    // Markov-like transition tracking: current -> next -> count
    unordered_map<int, unordered_map<int, int>> transitions;
    
    // Recent access history per process
    unordered_map<int, vector<int>> process_history;
    
    pthread_mutex_t prefetch_mutex;
    
    // Statistics
    int successful_prefetches;
    int failed_prefetches;
    
public:
    Prefetcher();
    ~Prefetcher();
    
    // Record page access
    void record_access(int process_id, int page_id);
    
    // Predict next page(s) to prefetch
    vector<int> predict_next(int process_id, int current_page);
    
    // Get prefetch candidates based on patterns
    vector<int> get_prefetch_candidates(int process_id);
    
    // Record prefetch result
    void record_prefetch_hit();
    void record_prefetch_miss();
    
    // Statistics
    double get_prefetch_accuracy() const;
    int get_successful_prefetches() const { return successful_prefetches; }
};

#endif // PREFETCHER_H
