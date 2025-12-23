#include "ThrashingDetector.h"
#include <algorithm>

using namespace std;

ThrashingDetector::ThrashingDetector() {
    recent_accesses.resize(SAMPLE_WINDOW, true);  // Start with all hits
    current_index = 0;
    total_samples = 0;
    is_thrashing = false;
    consecutive_critical = 0;
    pthread_mutex_init(&detector_mutex, nullptr);
}

ThrashingDetector::~ThrashingDetector() {
    pthread_mutex_destroy(&detector_mutex);
}

void ThrashingDetector::record_hit() {
    pthread_mutex_lock(&detector_mutex);
    
    recent_accesses[current_index] = true;
    current_index = (current_index + 1) % SAMPLE_WINDOW;
    total_samples++;
    
    pthread_mutex_unlock(&detector_mutex);
}

void ThrashingDetector::record_fault() {
    pthread_mutex_lock(&detector_mutex);
    
    recent_accesses[current_index] = false;
    current_index = (current_index + 1) % SAMPLE_WINDOW;
    total_samples++;
    
    pthread_mutex_unlock(&detector_mutex);
}

double ThrashingDetector::get_current_fault_rate() {
    pthread_mutex_lock(&detector_mutex);
    
    int samples = min(total_samples, SAMPLE_WINDOW);
    if (samples == 0) {
        pthread_mutex_unlock(&detector_mutex);
        return 0.0;
    }
    
    int faults = 0;
    for (int i = 0; i < samples; i++) {
        if (!recent_accesses[i]) {
            faults++;
        }
    }
    
    pthread_mutex_unlock(&detector_mutex);
    return (double)faults / samples;
}

bool ThrashingDetector::check_thrashing() {
    double fault_rate = get_current_fault_rate();
    
    pthread_mutex_lock(&detector_mutex);
    
    if (fault_rate >= FAULT_RATE_CRITICAL) {
        consecutive_critical++;
        if (consecutive_critical >= 3) {  // 3 consecutive critical readings
            is_thrashing = true;
        }
    } else if (fault_rate < FAULT_RATE_WARNING) {
        consecutive_critical = 0;
        is_thrashing = false;
    }
    
    bool result = is_thrashing;
    pthread_mutex_unlock(&detector_mutex);
    return result;
}

int ThrashingDetector::get_recommended_reduction() {
    double fault_rate = get_current_fault_rate();
    
    if (fault_rate >= FAULT_RATE_CRITICAL) {
        // Critical: suspend ~30% of processes
        return 3;
    } else if (fault_rate >= FAULT_RATE_WARNING) {
        // Warning: suspend ~10%
        return 1;
    }
    return 0;
}

void ThrashingDetector::reset() {
    pthread_mutex_lock(&detector_mutex);
    
    fill(recent_accesses.begin(), recent_accesses.end(), true);
    current_index = 0;
    total_samples = 0;
    is_thrashing = false;
    consecutive_critical = 0;
    
    pthread_mutex_unlock(&detector_mutex);
}
