#include "Prefetcher.h"
#include <algorithm>

using namespace std;

Prefetcher::Prefetcher() {
    successful_prefetches = 0;
    failed_prefetches = 0;
    pthread_mutex_init(&prefetch_mutex, nullptr);
}

Prefetcher::~Prefetcher() {
    pthread_mutex_destroy(&prefetch_mutex);
}

void Prefetcher::record_access(int process_id, int page_id) {
    pthread_mutex_lock(&prefetch_mutex);
    
    // Update history
    auto& history = process_history[process_id];
    
    // Record transition from previous page
    if (!history.empty()) {
        int prev_page = history.back();
        transitions[prev_page][page_id]++;
    }
    
    history.push_back(page_id);
    
    // Keep history bounded
    if ((int)history.size() > PATTERN_LENGTH * 2) {
        history.erase(history.begin());
    }
    
    pthread_mutex_unlock(&prefetch_mutex);
}

vector<int> Prefetcher::predict_next(int process_id, int current_page) {
    vector<int> predictions;
    
    pthread_mutex_lock(&prefetch_mutex);
    
    auto it = transitions.find(current_page);
    if (it != transitions.end()) {
        // Find most likely next pages
        vector<pair<int, int>> candidates;
        int total = 0;
        
        for (const auto& p : it->second) {
            candidates.push_back(p);
            total += p.second;
        }
        
        // Sort by frequency
        sort(candidates.begin(), candidates.end(),
             [](const pair<int,int>& a, const pair<int,int>& b) {
                 return a.second > b.second;
             });
        
        // Return predictions above confidence threshold
        for (const auto& c : candidates) {
            int confidence = (c.second * 100) / max(1, total);
            if (confidence >= MIN_CONFIDENCE) {
                predictions.push_back(c.first);
            }
            if (predictions.size() >= 3) break;  // Max 3 prefetches
        }
    }
    
    pthread_mutex_unlock(&prefetch_mutex);
    return predictions;
}

vector<int> Prefetcher::get_prefetch_candidates(int process_id) {
    pthread_mutex_lock(&prefetch_mutex);
    
    vector<int> candidates;
    auto it = process_history.find(process_id);
    if (it != process_history.end() && !it->second.empty()) {
        int current = it->second.back();
        pthread_mutex_unlock(&prefetch_mutex);
        return predict_next(process_id, current);
    }
    
    pthread_mutex_unlock(&prefetch_mutex);
    return candidates;
}

void Prefetcher::record_prefetch_hit() {
    pthread_mutex_lock(&prefetch_mutex);
    successful_prefetches++;
    pthread_mutex_unlock(&prefetch_mutex);
}

void Prefetcher::record_prefetch_miss() {
    pthread_mutex_lock(&prefetch_mutex);
    failed_prefetches++;
    pthread_mutex_unlock(&prefetch_mutex);
}

double Prefetcher::get_prefetch_accuracy() const {
    int total = successful_prefetches + failed_prefetches;
    if (total == 0) return 0.0;
    return (double)successful_prefetches / total;
}
