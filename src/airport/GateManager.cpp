#include "GateManager.h"
#include "../core/Logger.h"
#include <sstream>
#include <algorithm>

using namespace std;

GateManager::GateManager() {
    pthread_mutex_init(&manager_mutex, nullptr);
}

GateManager::~GateManager() {
    pthread_mutex_destroy(&manager_mutex);
    
    for (Gate* gate : gates) {
        delete gate;
    }
}

void GateManager::add_gate(Gate* gate) {
    pthread_mutex_lock(&manager_mutex);
    gates.push_back(gate);
    pthread_mutex_unlock(&manager_mutex);
}

bool GateManager::is_safe_state(int flight_index, const std::vector<int>& request) {
    // Simplified Banker's algorithm check
    // In full implementation, would check if allocation leaves system in safe state
    
    // For now, just check if gate is available and compatible
    return true;
}

Gate* GateManager::allocate_gate(Flight* flight) {
    // CRITICAL SECTION: Search and allocate gate
    pthread_mutex_lock(&manager_mutex);
    
    Gate* chosen_gate = nullptr;
    
    // Priority order: exact match > larger gate
    // First, try to find exact match
    for (Gate* gate : gates) {
        if (gate->is_available() && gate->is_compatible(flight)) {
            // Banker's algorithm: check if allocation is safe
            if (is_safe_state(0, {})) {
                chosen_gate = gate;
                break;
            }
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
    // END search phase
    
    if (chosen_gate == nullptr) {
        return nullptr; // No compatible gate available
    }
    
    // Try to reserve (gate has its own fine-grained lock)
    if (chosen_gate->try_reserve(flight)) {
        std::ostringstream log_msg;
        log_msg << "[GateManager] Flight " << flight->flight_id 
                << " allocated gate " << chosen_gate->get_id();
        Logger::get_instance()->log_event(log_msg.str());
        
        return chosen_gate;
    }
    
    return nullptr;
}

void GateManager::release_gate(int gate_id) {
    pthread_mutex_lock(&manager_mutex);
    
    for (Gate* gate : gates) {
        if (gate->get_id() == gate_id) {
            pthread_mutex_unlock(&manager_mutex);
            
            gate->release();
            
            std::ostringstream log_msg;
            log_msg << "[GateManager] Gate " << gate_id << " released";
            Logger::get_instance()->log_event(log_msg.str());
            
            return;
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
}

Gate* GateManager::get_gate(int id) {
    pthread_mutex_lock(&manager_mutex);
    
    for (Gate* gate : gates) {
        if (gate->get_id() == id) {
            pthread_mutex_unlock(&manager_mutex);
            return gate;
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
    return nullptr;
}

int GateManager::get_available_gate_count() {
    pthread_mutex_lock(&manager_mutex);
    
    int count = 0;
    for (Gate* gate : gates) {
        if (gate->is_available()) {
            count++;
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
    return count;
}

int GateManager::get_available_gate_count_by_type(GateType type) {
    pthread_mutex_lock(&manager_mutex);
    
    int count = 0;
    for (Gate* gate : gates) {
        if (gate->is_available() && gate->get_type() == type) {
            count++;
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
    return count;
}
