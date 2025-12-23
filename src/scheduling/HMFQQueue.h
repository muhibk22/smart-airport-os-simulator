#ifndef HMFQ_QUEUE_H
#define HMFQ_QUEUE_H

#include "Operation.h"
#include "PISCalculator.h"
#include "AgingManager.h"
#include "QuantumManager.h"
#include "PreemptionManager.h"
#include "LearningEngine.h"
#include "PriorityInheritance.h"
#include <vector>
#include <queue>
#include <pthread.h>

using namespace std;

// HMFQ-PPRA: Hybrid Multi-Feedback Queue with Preemptive Priority Recalculation Algorithm
// 5-level priority queue (0=Emergency, 1=Critical, 2=High, 3=Normal, 4=Low)

class HMFQQueue {
private:
    // 5 priority queues (vector of operations)
    vector<Operation*> queues[5];
    
    // Currently running operation
    Operation* current_operation;
    
    // Component managers
    PISCalculator* pis_calculator;
    AgingManager* aging_manager;
    QuantumManager* quantum_manager;
    PreemptionManager* preemption_manager;
    LearningEngine* learning_engine;
    PriorityInheritance* priority_inheritance;
    
    // Thread safety
    pthread_mutex_t scheduler_mutex;
    pthread_cond_t operation_available;
    
    // Statistics
    int total_operations_scheduled;
    int total_preemptions;
    int total_context_switches;
    long long total_wait_time;
    
    // Operation ID counter
    int next_operation_id;
    
    // Find operation with highest priority in all queues
    Operation* find_next_operation();
    
    // Recalculate priorities for all operations
    void recalculate_priorities(long long current_time);
    
public:
    HMFQQueue();
    ~HMFQQueue();
    
    // Initialize the scheduler
    void initialize();
    
    // Add operation to appropriate queue
    void enqueue(Operation* op);
    
    // Get next operation to run
    Operation* dequeue(long long current_time);
    
    // Complete an operation
    void complete(Operation* op);
    
    // Block operation (waiting for resource)
    void block(Operation* op);
    
    // Unblock operation
    void unblock(Operation* op);
    
    // Check if preemption needed and perform if so
    bool check_preemption(Operation* new_op);
    
    // Apply aging to all waiting operations
    void apply_aging(long long current_time);
    
    // Create operation from flight
    Operation* create_operation(Flight* flight, OperationType type, long long current_time);
    
    // Get queue statistics
    int get_queue_size(int queue_level);
    int get_total_operations() const { return total_operations_scheduled; }
    int get_total_preemptions() const { return total_preemptions; }
    double get_average_wait_time() const;
    
    // Get current operation
    Operation* get_current_operation() { return current_operation; }
    
    // Getters for component managers
    PISCalculator* get_pis_calculator() { return pis_calculator; }
    QuantumManager* get_quantum_manager() { return quantum_manager; }
    LearningEngine* get_learning_engine() { return learning_engine; }
    PriorityInheritance* get_priority_inheritance() { return priority_inheritance; }
    
    // Trigger learning adjustment
    void trigger_learning_adjustment();
};

#endif // HMFQ_QUEUE_H
