#ifndef AGING_MANAGER_H
#define AGING_MANAGER_H

#include "Operation.h"
#include <pthread.h>
#include <cmath>

using namespace std;

// AgingManager - Implements exponential aging to prevent starvation
// Formula from README:
// Age_Increment = Base_Age_Rate × e^(Wait_Time / Time_Constant)
// Priority_Boost = Current_Priority - (Age_Increment × Age_Weight)

class AgingManager {
private:
    // Time constants by queue (in time units, from README)
    // Lower = faster aging (Queue 4 ages fastest)
    static constexpr double TIME_CONSTANT_Q4 = 120.0;  // 2 min = 120 time units
    static constexpr double TIME_CONSTANT_Q3 = 180.0;  // 3 min = 180 time units
    static constexpr double TIME_CONSTANT_Q2 = 300.0;  // 5 min = 300 time units
    static constexpr double TIME_CONSTANT_Q1 = 480.0;  // 8 min = 480 time units
    // Queue 0 (Emergency) - no aging
    
    // Max wait thresholds for guaranteed service (time units)
    static constexpr long long MAX_WAIT_Q4 = 900;   // 15 min
    static constexpr long long MAX_WAIT_Q3 = 1200;  // 20 min
    static constexpr long long MAX_WAIT_Q2 = 1800;  // 30 min
    
    double base_age_rate;   // Default 1.0
    double age_weight;      // Default 0.1
    
    pthread_mutex_t aging_mutex;
    
public:
    AgingManager();
    ~AgingManager();
    
    // Calculate age increment for an operation based on its queue and wait time
    double calculate_age_increment(Operation* op);
    
    // Apply aging boost to an operation
    void apply_aging(Operation* op, long long current_time);
    
    // Check if operation needs guaranteed service (starvation prevention)
    bool needs_guaranteed_service(Operation* op);
    
    // Promote operation to Queue 1 for guaranteed service
    void promote_for_guaranteed_service(Operation* op);
    
    // Get time constant for a queue
    double get_time_constant(int queue);
    
    // Get max wait threshold for a queue
    long long get_max_wait_threshold(int queue);
    
    // Setters
    void set_base_age_rate(double rate);
    void set_age_weight(double weight);
};

#endif // AGING_MANAGER_H
