#ifndef LEARNING_ENGINE_H
#define LEARNING_ENGINE_H

#include "PISCalculator.h"
#include <pthread.h>

using namespace std;

// LearningEngine - Implements adaptive weight adjustment
// Formula from README:
// New_Estimate = 0.7 × Old_Estimate + 0.3 × Actual_Value

class LearningEngine {
private:
    PISCalculator* pis_calculator;
    
    // Historical averages (weighted moving average)
    double avg_completion_time;
    double avg_wait_time;
    double on_time_rate;
    
    // Learning rate
    static constexpr double LEARNING_RATE_OLD = 0.7;
    static constexpr double LEARNING_RATE_NEW = 0.3;
    static constexpr double WEIGHT_ADJUSTMENT = 0.01;
    
    pthread_mutex_t learning_mutex;
    
public:
    LearningEngine(PISCalculator* calculator);
    ~LearningEngine();
    
    // Update estimates with actual values
    void update_completion_time(double actual_time);
    void update_wait_time(double actual_wait);
    void update_on_time_rate(bool was_on_time);
    
    // Adjust PIS weights based on performance
    void adjust_weights();
    
    // Getters
    double get_avg_completion_time() const { return avg_completion_time; }
    double get_avg_wait_time() const { return avg_wait_time; }
    double get_on_time_rate() const { return on_time_rate; }
};

#endif // LEARNING_ENGINE_H
