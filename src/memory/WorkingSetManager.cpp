#include "WorkingSetManager.h"
#include <algorithm>

using namespace std;

WorkingSetManager::WorkingSetManager() {
    current_memory_usage = 0.5;  // Start at 50%
    pthread_mutex_init(&ws_mutex, nullptr);
}

WorkingSetManager::~WorkingSetManager() {
    pthread_mutex_destroy(&ws_mutex);
}

double WorkingSetManager::get_phase_multiplier(ProcessPhase phase) {
    switch (phase) {
        case PHASE_INIT: return PHASE_INIT_MULT;
        case PHASE_COMPUTATION: return PHASE_COMP_MULT;
        case PHASE_IO: return PHASE_IO_MULT;
        case PHASE_TERMINATION: return PHASE_TERM_MULT;
        default: return PHASE_COMP_MULT;
    }
}

LoadLevel WorkingSetManager::get_load_level() {
    pthread_mutex_lock(&ws_mutex);
    LoadLevel level;
    
    if (current_memory_usage < 0.4) {
        level = LOAD_LIGHT;
    } else if (current_memory_usage < 0.7) {
        level = LOAD_MEDIUM;
    } else if (current_memory_usage < 0.9) {
        level = LOAD_HEAVY;
    } else {
        level = LOAD_CRITICAL;
    }
    
    pthread_mutex_unlock(&ws_mutex);
    return level;
}

double WorkingSetManager::get_load_multiplier() {
    LoadLevel level = get_load_level();
    switch (level) {
        case LOAD_LIGHT: return LOAD_LIGHT_MULT;
        case LOAD_MEDIUM: return LOAD_MEDIUM_MULT;
        case LOAD_HEAVY: return LOAD_HEAVY_MULT;
        case LOAD_CRITICAL: return LOAD_CRITICAL_MULT;
        default: return LOAD_MEDIUM_MULT;
    }
}

double WorkingSetManager::get_fault_rate_multiplier(double fault_rate) {
    if (fault_rate < 0.05) {
        return FAULT_LOW_MULT;
    } else if (fault_rate <= 0.15) {
        return FAULT_NORMAL_MULT;
    } else {
        return FAULT_HIGH_MULT;
    }
}

int WorkingSetManager::calculate_window(ProcessPhase phase, double fault_rate) {
    // Δ_actual = Δ_base × Phase_Multiplier × Load_Multiplier × Fault_Rate_Multiplier
    double phase_mult = get_phase_multiplier(phase);
    double load_mult = get_load_multiplier();
    double fault_mult = get_fault_rate_multiplier(fault_rate);
    
    double delta = DELTA_BASE * phase_mult * load_mult * fault_mult;
    
    // Clamp to reasonable bounds
    return max(5, min(100, (int)delta));
}

vector<int> WorkingSetManager::get_working_set(PageTable* page_table, 
                                                long long current_time, int window) {
    // Return pages accessed within window
    // In real implementation, would check last access times
    // For now, return all valid pages as working set
    if (page_table == nullptr) {
        return vector<int>();
    }
    return page_table->get_valid_pages();
}

void WorkingSetManager::set_memory_usage(double usage) {
    pthread_mutex_lock(&ws_mutex);
    current_memory_usage = max(0.0, min(1.0, usage));
    pthread_mutex_unlock(&ws_mutex);
}
