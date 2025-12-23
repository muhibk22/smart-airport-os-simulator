#ifndef THRASHING_DETECTOR_H
#define THRASHING_DETECTOR_H

#include <pthread.h>
#include <vector>

using namespace std;

// ThrashingDetector monitors page fault rates to detect thrashing
// Triggers mitigation when fault rate exceeds threshold

class ThrashingDetector {
private:
    // Thresholds
    static constexpr double FAULT_RATE_WARNING = 0.15;    // 15% - warning
    static constexpr double FAULT_RATE_CRITICAL = 0.25;   // 25% - thrashing
    static constexpr int SAMPLE_WINDOW = 100;             // Recent accesses to consider
    
    vector<bool> recent_accesses;  // true = hit, false = fault
    int current_index;
    int total_samples;
    
    bool is_thrashing;
    int consecutive_critical;
    
    pthread_mutex_t detector_mutex;
    
public:
    ThrashingDetector();
    ~ThrashingDetector();
    
    // Record access result
    void record_hit();
    void record_fault();
    
    // Get current fault rate (0.0 to 1.0)
    double get_current_fault_rate();
    
    // Check thrashing status
    bool check_thrashing();
    bool is_in_thrashing_state() const { return is_thrashing; }
    
    // Get mitigation recommendations
    int get_recommended_reduction();  // Reduce active processes by this many
    
    // Reset detector
    void reset();
};

#endif // THRASHING_DETECTOR_H
