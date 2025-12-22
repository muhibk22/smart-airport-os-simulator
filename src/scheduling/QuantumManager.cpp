#include "QuantumManager.h"
#include <cmath>
#include <algorithm>

using namespace std;

QuantumManager::QuantumManager() {
    active_operations = 0;
    max_operations = 50;
    pthread_mutex_init(&quantum_mutex, nullptr);
}

QuantumManager::~QuantumManager() {
    pthread_mutex_destroy(&quantum_mutex);
}

int QuantumManager::get_base_quantum(int queue) {
    switch (queue) {
        case 0: return BASE_QUANTUM_Q0;  // Unlimited
        case 1: return BASE_QUANTUM_Q1;
        case 2: return BASE_QUANTUM_Q2;
        case 3: return BASE_QUANTUM_Q3;
        case 4: return BASE_QUANTUM_Q4;
        default: return BASE_QUANTUM_Q3;
    }
}

double QuantumManager::get_operation_factor(OperationComplexity complexity) {
    switch (complexity) {
        case COMPLEXITY_SIMPLE:  return FACTOR_SIMPLE;
        case COMPLEXITY_MEDIUM:  return FACTOR_MEDIUM;
        case COMPLEXITY_COMPLEX: return FACTOR_COMPLEX;
        default: return FACTOR_MEDIUM;
    }
}

double QuantumManager::calculate_load_factor() {
    pthread_mutex_lock(&quantum_mutex);
    
    if (max_operations <= 0) {
        pthread_mutex_unlock(&quantum_mutex);
        return 1.0;
    }
    
    // Load_Factor = 1 - (Active_Operations / Max_Operations)^2
    double ratio = (double)active_operations / max_operations;
    double load_factor = 1.0 - (ratio * ratio);
    
    // Clamp to range [0.4, 1.0]
    load_factor = max(0.4, min(1.0, load_factor));
    
    pthread_mutex_unlock(&quantum_mutex);
    
    return load_factor;
}

int QuantumManager::calculate_quantum(Operation* op) {
    if (op == nullptr) return BASE_QUANTUM_Q3;
    
    pthread_mutex_lock(&op->op_mutex);
    
    int queue = op->current_queue;
    OperationComplexity complexity = op->complexity;
    int compensation = op->quantum_compensation;
    
    pthread_mutex_unlock(&op->op_mutex);
    
    // Queue 0 (Emergency) - unlimited quantum
    if (queue == 0) {
        return 0; // 0 means run to completion
    }
    
    int base_quantum = get_base_quantum(queue);
    double load_factor = calculate_load_factor();
    double op_factor = get_operation_factor(complexity);
    
    // Actual_Quantum = Base_Quantum × Load_Factor × Operation_Factor
    int actual_quantum = (int)(base_quantum * load_factor * op_factor);
    
    // Add any compensation from previous preemption
    actual_quantum += compensation;
    
    // Ensure minimum quantum of 10
    return max(10, actual_quantum);
}

void QuantumManager::set_active_operations(int count) {
    pthread_mutex_lock(&quantum_mutex);
    active_operations = max(0, count);
    pthread_mutex_unlock(&quantum_mutex);
}

void QuantumManager::set_max_operations(int max) {
    pthread_mutex_lock(&quantum_mutex);
    max_operations = max(1, max);
    pthread_mutex_unlock(&quantum_mutex);
}
