#include "FatigueModel.h"
#include <algorithm>

using namespace std;

FatigueModel::FatigueModel() {}
FatigueModel::~FatigueModel() {}

double FatigueModel::get_time_factor(double duty_hours) {
    // Fatigue accelerates exponentially past 8 hours
    if (duty_hours <= 8.0) {
        return 1.0;
    } else {
        return 1.0 + (duty_hours - 8.0) * 0.2;  // +20% per hour over 8
    }
}

double FatigueModel::get_workload_factor(int passengers, bool is_emergency) {
    double factor = 1.0;
    
    // More passengers = more work
    factor += passengers / 500.0;  // +0.2 per 100 passengers
    
    // Emergency = high stress
    if (is_emergency) {
        factor *= 1.5;
    }
    
    return min(2.0, factor);
}

double FatigueModel::calculate_fatigue_increase(double duty_hours, double workload) {
    // Fatigue_Increase = Base × Time_Factor × Workload_Factor
    double time_factor = get_time_factor(duty_hours);
    double increase = BASE_FATIGUE_RATE * duty_hours * time_factor * workload;
    return min(1.0, increase);
}

double FatigueModel::calculate_recovery(double rest_hours, double current_fatigue) {
    // Recovery = Rest_Hours × Recovery_Rate × (1 + current_fatigue)
    // More fatigued = slightly faster initial recovery
    double recovery = rest_hours * REST_RECOVERY_RATE * (1.0 + current_fatigue * 0.5);
    return min(current_fatigue, recovery);
}

bool FatigueModel::needs_mandatory_rest(double fatigue_score, double duty_hours) {
    return fatigue_score >= 0.8 || duty_hours >= MAX_DUTY_HOURS;
}

double FatigueModel::predict_post_duty_fatigue(double current, double duty_hours, double workload) {
    double increase = calculate_fatigue_increase(duty_hours, workload);
    return min(1.0, current + increase);
}
