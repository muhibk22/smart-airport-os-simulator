#ifndef CLOCK_REPLACER_H
#define CLOCK_REPLACER_H

#include "Page.h"
#include "PageFrame.h"
#include <vector>
#include <pthread.h>

using namespace std;

// ClockReplacer implements AWSC-PPC victim selection
// Multi-pass clock algorithm with composite victim scoring
// Formula from README:
// Victim_Score = (Frequency × W_freq) + (Recency × W_rec) + (Phase × W_phase) + ...

class ClockReplacer {
private:
    vector<PageFrame*> frames;
    int clock_hand;
    int num_frames;
    
    // Weights for victim scoring
    static constexpr double W_FREQ = 0.20;      // Reference frequency
    static constexpr double W_REC = 0.25;       // Recency
    static constexpr double W_PHASE = 0.15;     // Process phase
    static constexpr double W_PREDICT = 0.20;   // Predicted reuse
    static constexpr double W_TIER = 0.10;      // Data tier
    static constexpr double W_DIRTY = 0.10;     // Dirty bit penalty
    
    pthread_mutex_t clock_mutex;
    
    // Calculate victim score (higher = better victim)
    double calculate_victim_score(Page* page, long long current_time);
    
public:
    ClockReplacer(int frame_count);
    ~ClockReplacer();
    
    // Initialize frames
    void add_frame(PageFrame* frame);
    
    // Find victim page to evict (returns frame_id)
    int find_victim(long long current_time);
    
    // Multi-pass algorithm
    int clock_sweep(long long current_time);
    
    // Statistics
    int get_hand_position() const { return clock_hand; }
};

#endif // CLOCK_REPLACER_H
