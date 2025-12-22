#include "AgingManager.h"
#include "../core/Logger.h"
#include <sstream>
#include <algorithm>
#include <climits>

using namespace std;

AgingManager::AgingManager() {
    base_age_rate = 1.0;
    age_weight = 0.1;
    pthread_mutex_init(&aging_mutex, nullptr);
}

AgingManager::~AgingManager() {
    pthread_mutex_destroy(&aging_mutex);
}

double AgingManager::get_time_constant(int queue) {
    switch (queue) {
        case 0: return 0.0;                 // Emergency - no aging
        case 1: return TIME_CONSTANT_Q1;    // Critical
        case 2: return TIME_CONSTANT_Q2;    // High
        case 3: return TIME_CONSTANT_Q3;    // Normal
        case 4: return TIME_CONSTANT_Q4;    // Low
        default: return TIME_CONSTANT_Q3;
    }
}

long long AgingManager::get_max_wait_threshold(int queue) {
    switch (queue) {
        case 0: return 0;           // Emergency - immediate
        case 1: return LLONG_MAX;   // Critical - no additional guarantee
        case 2: return MAX_WAIT_Q2;
        case 3: return MAX_WAIT_Q3;
        case 4: return MAX_WAIT_Q4;
        default: return MAX_WAIT_Q3;
    }
}

double AgingManager::calculate_age_increment(Operation* op) {
    if (op == nullptr) return 0.0;
    
    pthread_mutex_lock(&aging_mutex);
    
    int queue = op->current_queue;
    
    // Queue 0 (Emergency) doesn't age
    if (queue == 0) {
        pthread_mutex_unlock(&aging_mutex);
        return 0.0;
    }
    
    double time_constant = get_time_constant(queue);
    if (time_constant <= 0.0) {
        pthread_mutex_unlock(&aging_mutex);
        return 0.0;
    }
    
    // Age_Increment = Base_Age_Rate × e^(Wait_Time / Time_Constant)
    double age_increment = base_age_rate * exp((double)op->wait_time / time_constant);
    
    pthread_mutex_unlock(&aging_mutex);
    
    return age_increment;
}

void AgingManager::apply_aging(Operation* op, long long current_time) {
    if (op == nullptr) return;
    
    pthread_mutex_lock(&op->op_mutex);
    
    // Update wait time
    op->wait_time = current_time - op->arrival_time;
    
    // Calculate age increment
    double age_increment = calculate_age_increment(op);
    
    // Priority_Boost = Current_Priority - (Age_Increment × Age_Weight)
    // Lower priority number = higher priority, so we subtract
    pthread_mutex_lock(&aging_mutex);
    double priority_boost = age_increment * age_weight;
    pthread_mutex_unlock(&aging_mutex);
    
    // Apply boost to PIS (higher PIS = higher priority for scheduling)
    op->priority_score += priority_boost;
    
    // Check for guaranteed service
    if (needs_guaranteed_service(op)) {
        promote_for_guaranteed_service(op);
    }
    
    pthread_mutex_unlock(&op->op_mutex);
}

bool AgingManager::needs_guaranteed_service(Operation* op) {
    if (op == nullptr) return false;
    
    // Already guaranteed or emergency
    if (op->guaranteed_service || op->current_queue == 0) {
        return false;
    }
    
    long long max_wait = get_max_wait_threshold(op->current_queue);
    return op->wait_time > max_wait;
}

void AgingManager::promote_for_guaranteed_service(Operation* op) {
    if (op == nullptr) return;
    
    Logger* logger = Logger::get_instance();
    
    int old_queue = op->current_queue;
    
    // Promote to Queue 1 (Critical) for guaranteed service
    op->current_queue = 1;
    op->guaranteed_service = true;
    
    ostringstream msg;
    msg << "[AGING] Operation " << op->id << " promoted from Q" << old_queue 
        << " to Q1 for guaranteed service (waited " << op->wait_time << " time units)";
    logger->log_scheduling(msg.str());
}

void AgingManager::set_base_age_rate(double rate) {
    pthread_mutex_lock(&aging_mutex);
    base_age_rate = max(0.1, min(5.0, rate));
    pthread_mutex_unlock(&aging_mutex);
}

void AgingManager::set_age_weight(double weight) {
    pthread_mutex_lock(&aging_mutex);
    age_weight = max(0.01, min(1.0, weight));
    pthread_mutex_unlock(&aging_mutex);
}
