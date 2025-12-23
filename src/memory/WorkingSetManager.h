#ifndef WORKING_SET_MANAGER_H
#define WORKING_SET_MANAGER_H

#include "Page.h"
#include "PageTable.h"
#include <vector>
#include <unordered_set>
#include <pthread.h>

using namespace std;

// WorkingSetManager implements AWSC-PPC dynamic working set calculation
// Formula from README:
// Δ_actual = Δ_base × Phase_Multiplier × Load_Multiplier × Fault_Rate_Multiplier

enum ProcessPhase {
    PHASE_INIT,         // Initialization (1.5x)
    PHASE_COMPUTATION,  // Normal computation (1.0x)
    PHASE_IO,           // I/O bound (0.7x)
    PHASE_TERMINATION   // Shutting down (0.5x)
};

enum LoadLevel {
    LOAD_LIGHT,     // <40% memory (1.3x)
    LOAD_MEDIUM,    // 40-70% (1.0x)
    LOAD_HEAVY,     // 70-90% (0.8x)
    LOAD_CRITICAL   // >90% (0.6x)
};

class WorkingSetManager {
private:
    static constexpr int DELTA_BASE = 15;  // Base window size
    
    // Phase multipliers
    static constexpr double PHASE_INIT_MULT = 1.5;
    static constexpr double PHASE_COMP_MULT = 1.0;
    static constexpr double PHASE_IO_MULT = 0.7;
    static constexpr double PHASE_TERM_MULT = 0.5;
    
    // Load multipliers
    static constexpr double LOAD_LIGHT_MULT = 1.3;
    static constexpr double LOAD_MEDIUM_MULT = 1.0;
    static constexpr double LOAD_HEAVY_MULT = 0.8;
    static constexpr double LOAD_CRITICAL_MULT = 0.6;
    
    // Fault rate multipliers
    static constexpr double FAULT_LOW_MULT = 0.9;     // <5%
    static constexpr double FAULT_NORMAL_MULT = 1.0;  // 5-15%
    static constexpr double FAULT_HIGH_MULT = 1.2;    // >15%
    
    // System state
    double current_memory_usage;  // 0.0 to 1.0
    
    pthread_mutex_t ws_mutex;
    
public:
    WorkingSetManager();
    ~WorkingSetManager();
    
    // Calculate dynamic window size for a process
    int calculate_window(ProcessPhase phase, double fault_rate);
    
    // Get working set for a process (pages accessed in window)
    vector<int> get_working_set(PageTable* page_table, long long current_time, int window);
    
    // Update system load
    void set_memory_usage(double usage);
    LoadLevel get_load_level();
    
    // Get multipliers
    double get_phase_multiplier(ProcessPhase phase);
    double get_load_multiplier();
    double get_fault_rate_multiplier(double fault_rate);
};

#endif // WORKING_SET_MANAGER_H
