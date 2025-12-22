#ifndef GATE_MANAGER_H
#define GATE_MANAGER_H

#include "Gate.h"
#include <vector>
#include <pthread.h>

using namespace std;

class GateManager {
private:
    vector<Gate*> gates;
    pthread_mutex_t manager_mutex;
    
    // Banker's algorithm data structures
    vector<int> available;    // Available resources per gate type
    vector<vector<int>> allocation;  // Current allocation
    vector<vector<int>> maximum;     // Maximum need
    
    bool is_safe_state(int flight_index, const vector<int>& request);
    
public:
    GateManager();
    ~GateManager();
    
    void add_gate(Gate* gate);
    
    // Allocate gate using Banker's algorithm
    Gate* allocate_gate(Flight* flight);
    
    // Release gate
    void release_gate(int gate_id);
    
    // Get gate by ID
    Gate* get_gate(int id);
    
    // Statistics
    int get_available_gate_count();
    int get_available_gate_count_by_type(GateType type);
};

#endif // GATE_MANAGER_H
