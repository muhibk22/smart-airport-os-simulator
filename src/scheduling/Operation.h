#ifndef OPERATION_H
#define OPERATION_H

#include "../airport/Flight.h"
#include <pthread.h>

using namespace std;

// Operation types for the scheduler
enum OperationType {
    OP_LANDING,
    OP_TAKEOFF,
    OP_TAXIING,
    OP_GATE_ARRIVAL,
    OP_GATE_DEPARTURE,
    OP_REFUELING,
    OP_CLEANING,
    OP_CATERING,
    OP_BOARDING,
    OP_BAGGAGE,
    OP_EMERGENCY
};

// Operation complexity for quantum calculation
enum OperationComplexity {
    COMPLEXITY_SIMPLE,   // Factor 0.7 (taxiing)
    COMPLEXITY_MEDIUM,   // Factor 1.0 (refueling)
    COMPLEXITY_COMPLEX   // Factor 1.3 (full turnaround)
};

// Scheduler operation - represents a unit of work to be scheduled
struct Operation {
    int id;
    Flight* flight;
    OperationType type;
    OperationComplexity complexity;
    
    // Timing
    long long arrival_time;         // When operation entered queue
    long long start_time;           // When operation started execution
    long long remaining_time;       // Time remaining to complete
    long long total_time;           // Total time required
    
    // Priority (lower number = higher priority)
    int current_queue;              // Which MLFQ queue (0-4, 0=emergency)
    double priority_score;          // PIS score
    
    // Aging and preemption
    long long wait_time;            // How long waiting in queue
    bool guaranteed_service;        // Starvation prevention flag
    int preemption_count;           // Times this operation was preempted
    int quantum_compensation;       // Extra quantum from preemption
    
    // State
    bool is_running;
    bool is_completed;
    bool is_blocked;                // Waiting for resource
    
    // For scheduler queue management
    pthread_mutex_t op_mutex;
    
    Operation() {
        id = 0;
        flight = nullptr;
        type = OP_LANDING;
        complexity = COMPLEXITY_MEDIUM;
        arrival_time = 0;
        start_time = 0;
        remaining_time = 0;
        total_time = 0;
        current_queue = 3;  // Default to normal priority
        priority_score = 0.0;
        wait_time = 0;
        guaranteed_service = false;
        preemption_count = 0;
        quantum_compensation = 0;
        is_running = false;
        is_completed = false;
        is_blocked = false;
        pthread_mutex_init(&op_mutex, nullptr);
    }
    
    ~Operation() {
        pthread_mutex_destroy(&op_mutex);
    }
};

#endif // OPERATION_H
