#include "PreemptionManager.h"
#include "../core/Logger.h"
#include <sstream>
#include <algorithm>

using namespace std;

PreemptionManager::PreemptionManager() {
    pthread_mutex_init(&preempt_mutex, nullptr);
}

PreemptionManager::~PreemptionManager() {
    pthread_mutex_destroy(&preempt_mutex);
}

double PreemptionManager::calculate_urgency(Operation* op) {
    if (op == nullptr) return 0.0;
    
    double urgency = 0.0;
    
    // Base urgency from queue level (lower queue = higher urgency)
    urgency += (4 - op->current_queue) * 20.0;
    
    // Add urgency from PIS score
    urgency += op->priority_score * 10.0;
    
    // Add urgency from wait time
    urgency += op->wait_time / 10.0;
    
    // Emergency operations have maximum urgency
    if (op->type == OP_EMERGENCY || (op->flight && op->flight->is_emergency())) {
        urgency = 1000.0;
    }
    
    return urgency;
}

double PreemptionManager::calculate_delay_cost(Operation* op) {
    if (op == nullptr) return 1.0;
    
    double cost = 1.0;
    
    // Higher cost for operations with more passengers
    if (op->flight) {
        cost += op->flight->passenger_count / 100.0;
        
        // International flights cost more to delay
        if (op->flight->flight_type == INTERNATIONAL) {
            cost *= 1.5;
        }
    }
    
    // Emergency has very high delay cost
    if (op->type == OP_EMERGENCY) {
        cost *= 10.0;
    }
    
    return cost;
}

double PreemptionManager::calculate_downstream_impact(Operation* op) {
    if (op == nullptr) return 0.0;
    
    double impact = 0.0;
    
    // Estimate downstream flights affected
    if (op->flight) {
        // More passengers = more potential connections
        impact += op->flight->passenger_count / 50.0;
        
        // Gate operations have higher downstream impact
        if (op->type == OP_GATE_ARRIVAL || op->type == OP_GATE_DEPARTURE) {
            impact += 5.0;
        }
    }
    
    return impact;
}

double PreemptionManager::calculate_benefit(Operation* high_priority) {
    // Preemption_Benefit = High_Priority_Urgency × High_Priority_Delay_Cost
    double urgency = calculate_urgency(high_priority);
    double delay_cost = calculate_delay_cost(high_priority);
    
    return urgency * delay_cost;
}

double PreemptionManager::calculate_cost(Operation* low_priority) {
    if (low_priority == nullptr) return 0.0;
    
    // Progress lost = what percentage of work done so far
    double progress_lost = 0.0;
    if (low_priority->total_time > 0) {
        double completed = low_priority->total_time - low_priority->remaining_time;
        progress_lost = completed / low_priority->total_time;
    }
    
    // Preemption_Cost = Progress_Lost × Context_Switch_Cost
    //                 + Resource_Reconfiguration_Cost
    //                 + Downstream_Impact_Cost
    double cost = progress_lost * CONTEXT_SWITCH_COST;
    cost += RESOURCE_RECONFIG_COST;
    cost += calculate_downstream_impact(low_priority);
    
    return cost;
}

bool PreemptionManager::should_preempt(Operation* high_priority, Operation* low_priority) {
    if (high_priority == nullptr || low_priority == nullptr) {
        return false;
    }
    
    pthread_mutex_lock(&preempt_mutex);
    
    double benefit = calculate_benefit(high_priority);
    double cost = calculate_cost(low_priority);
    
    // Decision Rule: Benefit > 1.5 × Cost
    bool should = (benefit > PREEMPTION_THRESHOLD * cost);
    
    pthread_mutex_unlock(&preempt_mutex);
    
    Logger* logger = Logger::get_instance();
    ostringstream msg;
    msg << "[PREEMPTION] Evaluating: Op " << high_priority->id 
        << " vs Op " << low_priority->id
        << " - Benefit: " << benefit << ", Cost: " << cost
        << ", Decision: " << (should ? "PREEMPT" : "NO");
    logger->log_scheduling(msg.str());
    
    return should;
}

void PreemptionManager::perform_preemption(Operation* high_priority, Operation* preempted) {
    if (preempted == nullptr) return;
    
    pthread_mutex_lock(&preempted->op_mutex);
    
    // Save state (operation already has remaining_time tracked)
    preempted->is_running = false;
    preempted->preemption_count++;
    
    // Provide quantum compensation (10% bonus for next run)
    int compensation = preempted->total_time / 10;
    preempted->quantum_compensation += compensation;
    
    // Demote to lower queue (MLFQ behavior) unless guaranteed service
    if (!preempted->guaranteed_service && preempted->current_queue < 4) {
        preempted->current_queue++;
    }
    
    pthread_mutex_unlock(&preempted->op_mutex);
    
    Logger* logger = Logger::get_instance();
    ostringstream msg;
    msg << "[PREEMPTION] Op " << preempted->id << " preempted by Op " 
        << high_priority->id << ". Demoted to Q" << preempted->current_queue
        << ", compensation: " << compensation;
    logger->log_scheduling(msg.str());
}
