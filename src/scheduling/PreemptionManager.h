#ifndef PREEMPTION_MANAGER_H
#define PREEMPTION_MANAGER_H

#include "Operation.h"
#include <pthread.h>

using namespace std;

// PreemptionManager - Implements cost-benefit preemption analysis
// From README:
// Preemption_Benefit = High_Priority_Urgency × High_Priority_Delay_Cost
// Preemption_Cost = Low_Priority_Progress_Lost × Context_Switch_Cost
//                 + Resource_Reconfiguration_Cost + Downstream_Impact_Cost
// Rule: If (Benefit > 1.5 × Cost) then preempt

class PreemptionManager {
private:
    static constexpr double PREEMPTION_THRESHOLD = 1.5;
    static constexpr double CONTEXT_SWITCH_COST = 5.0;
    static constexpr double RESOURCE_RECONFIG_COST = 10.0;
    
    pthread_mutex_t preempt_mutex;
    
public:
    PreemptionManager();
    ~PreemptionManager();
    
    // Decide if high-priority op should preempt low-priority op
    bool should_preempt(Operation* high_priority, Operation* low_priority);
    
    // Calculate preemption benefit
    double calculate_benefit(Operation* high_priority);
    
    // Calculate preemption cost
    double calculate_cost(Operation* low_priority);
    
    // Perform preemption - save state and provide compensation
    void perform_preemption(Operation* high_priority, Operation* preempted);
    
    // Calculate downstream impact
    double calculate_downstream_impact(Operation* op);
    
    // Calculate urgency score
    double calculate_urgency(Operation* op);
    
    // Calculate delay cost per time unit
    double calculate_delay_cost(Operation* op);
};

#endif // PREEMPTION_MANAGER_H
