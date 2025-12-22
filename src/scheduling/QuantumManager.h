#ifndef QUANTUM_MANAGER_H
#define QUANTUM_MANAGER_H

#include "Operation.h"
#include <pthread.h>

using namespace std;

// QuantumManager - Implements dynamic quantum adjustment
// Formula from README:
// Actual_Quantum = Base_Quantum × Load_Factor × Operation_Factor
// Load_Factor = 1 - (Active_Operations / Max_Operations)^2, range [0.4, 1.0]

class QuantumManager {
private:
    // Base quantum by queue (time units)
    static constexpr int BASE_QUANTUM_Q0 = 0;    // Unlimited (run to completion)
    static constexpr int BASE_QUANTUM_Q1 = 200;  // Critical
    static constexpr int BASE_QUANTUM_Q2 = 150;  // High
    static constexpr int BASE_QUANTUM_Q3 = 100;  // Normal
    static constexpr int BASE_QUANTUM_Q4 = 50;   // Low
    
    // Operation complexity factors
    static constexpr double FACTOR_SIMPLE = 0.7;   // Taxiing
    static constexpr double FACTOR_MEDIUM = 1.0;   // Refueling
    static constexpr double FACTOR_COMPLEX = 1.3;  // Full turnaround
    
    int active_operations;
    int max_operations;
    
    pthread_mutex_t quantum_mutex;
    
public:
    QuantumManager();
    ~QuantumManager();
    
    // Calculate actual quantum for an operation
    int calculate_quantum(Operation* op);
    
    // Get base quantum for a queue
    int get_base_quantum(int queue);
    
    // Calculate load factor
    double calculate_load_factor();
    
    // Get operation complexity factor
    double get_operation_factor(OperationComplexity complexity);
    
    // Update system load
    void set_active_operations(int count);
    void set_max_operations(int max_val);
    
    // Getters
    int get_active_operations() const { return active_operations; }
    int get_max_operations() const { return max_operations; }
};

#endif // QUANTUM_MANAGER_H
