#ifndef FATIGUE_MODEL_H
#define FATIGUE_MODEL_H

#include "Crew.h"
#include <cmath>

using namespace std;

// FatigueModel implements fatigue score calculation from README
// Formula: Fatigue_Score = Base_Fatigue × Time_Factor × Workload_Factor × Rest_Factor

class FatigueModel {
private:
    static constexpr double BASE_FATIGUE_RATE = 0.01;  // Per hour
    static constexpr double REST_RECOVERY_RATE = 0.02; // Per hour of rest
    static constexpr double MAX_DUTY_HOURS = 12.0;     // Max shift before mandatory rest
    
public:
    FatigueModel();
    ~FatigueModel();
    
    // Calculate fatigue increase for duty period
    static double calculate_fatigue_increase(double duty_hours, double workload);
    
    // Calculate fatigue recovery for rest period
    static double calculate_recovery(double rest_hours, double current_fatigue);
    
    // Get time factor (fatigue accelerates over long shifts)
    static double get_time_factor(double duty_hours);
    
    // Get workload factor
    static double get_workload_factor(int passengers, bool is_emergency);
    
    // Check if crew needs mandatory rest
    static bool needs_mandatory_rest(double fatigue_score, double duty_hours);
    
    // Predict fatigue after duty
    static double predict_post_duty_fatigue(double current, double duty_hours, double workload);
};

#endif // FATIGUE_MODEL_H
