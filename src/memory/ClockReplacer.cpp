#include "ClockReplacer.h"
#include <algorithm>
#include <cmath>

using namespace std;

ClockReplacer::ClockReplacer(int frame_count) {
    num_frames = frame_count;
    clock_hand = 0;
    pthread_mutex_init(&clock_mutex, nullptr);
}

ClockReplacer::~ClockReplacer() {
    pthread_mutex_destroy(&clock_mutex);
}

void ClockReplacer::add_frame(PageFrame* frame) {
    pthread_mutex_lock(&clock_mutex);
    frames.push_back(frame);
    pthread_mutex_unlock(&clock_mutex);
}

double ClockReplacer::calculate_victim_score(Page* page, long long current_time) {
    if (page == nullptr) return 0.0;
    
    double score = 0.0;
    
    // Frequency component (lower frequency = better victim)
    // Normalize reference count
    int ref_count = page->get_reference_count();
    double freq_score = 1.0 / (1.0 + ref_count);  // Inverse - less refs = higher score
    score += W_FREQ * freq_score;
    
    // Recency component (older = better victim)
    long long age = current_time - page->get_last_access_time();
    double rec_score = min(1.0, age / 1000.0);  // Normalize to 1000 time units
    score += W_REC * rec_score;
    
    // Tier component (cold data = better victim)
    double tier_score = 0.0;
    switch (page->get_tier()) {
        case TIER_L4: tier_score = 1.0; break;  // Cold - best victim
        case TIER_L3: tier_score = 0.7; break;
        case TIER_L2: tier_score = 0.4; break;
        case TIER_L1: tier_score = 0.1; break;  // Hot - worst victim
    }
    score += W_TIER * tier_score;
    
    // Dirty bit penalty (clean pages preferred)
    if (!page->get_modified_bit()) {
        score += W_DIRTY * 1.0;  // Clean page bonus
    }
    
    // Reference bit (not referenced = better victim)
    if (!page->get_reference_bit()) {
        score += W_PHASE * 1.0;  // Not recently used bonus
    }
    
    return score;
}

int ClockReplacer::clock_sweep(long long current_time) {
    pthread_mutex_lock(&clock_mutex);
    
    if (frames.empty()) {
        pthread_mutex_unlock(&clock_mutex);
        return -1;
    }
    
    // Multi-pass sweep: first look for ref=0, modified=0
    // Then ref=0, modified=1
    // Then ref=1 (clear ref bit and continue)
    
    int passes = 0;
    const int MAX_PASSES = 4;
    
    while (passes < MAX_PASSES) {
        for (int i = 0; i < (int)frames.size(); i++) {
            PageFrame* frame = frames[clock_hand];
            clock_hand = (clock_hand + 1) % frames.size();
            
            if (frame->is_empty() || frame->get_pinned()) {
                continue;
            }
            
            Page* page = frame->get_page();
            if (page == nullptr) continue;
            
            bool ref = page->get_reference_bit();
            bool modified = page->get_modified_bit();
            
            // Pass 1: Find ref=0, modified=0 (best victim)
            if (passes == 0 && !ref && !modified) {
                pthread_mutex_unlock(&clock_mutex);
                return frame->get_frame_id();
            }
            // Pass 2: Find ref=0, modified=1
            else if (passes == 1 && !ref && modified) {
                pthread_mutex_unlock(&clock_mutex);
                return frame->get_frame_id();
            }
            // Pass 3 & 4: Clear reference bits and retry
            else if (passes >= 2) {
                page->clear_reference_bit();
            }
        }
        passes++;
    }
    
    // Fallback: use victim scoring to find best candidate
    int best_frame = -1;
    double best_score = -1.0;
    
    for (int i = 0; i < (int)frames.size(); i++) {
        PageFrame* frame = frames[i];
        if (frame->is_empty() || frame->get_pinned()) continue;
        
        double score = calculate_victim_score(frame->get_page(), current_time);
        if (score > best_score) {
            best_score = score;
            best_frame = frame->get_frame_id();
        }
    }
    
    pthread_mutex_unlock(&clock_mutex);
    return best_frame;
}

int ClockReplacer::find_victim(long long current_time) {
    return clock_sweep(current_time);
}
