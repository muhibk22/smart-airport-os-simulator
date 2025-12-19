#include "Gate.h"

Gate::Gate(int gate_id, GateType gt, GateSize gs, bool jetbridge)
    : id(gate_id), type(gt), size(gs), has_jetbridge(jetbridge),
      available(true), current_flight(nullptr) {
    
    pthread_mutex_init(&gate_mutex, nullptr);
    pthread_cond_init(&gate_free, nullptr);
}

Gate::~Gate() {
    pthread_mutex_destroy(&gate_mutex);
    pthread_cond_destroy(&gate_free);
}

bool Gate::is_compatible(Flight* flight) {
    // Check flight type compatibility
    if (flight->needs_international_gate() && type != GATE_INTERNATIONAL) {
        return false;
    }
    
    // Check size compatibility
    AircraftType ac_type = flight->aircraft->type;
    
    switch(ac_type) {
        case A380:
            return size == GATE_LARGE;
            
        case B777:
        case B747F:
        case B777F:
            return size == GATE_LARGE || size == GATE_HEAVY;
            
        case B737:
        case A320:
            return size == GATE_HEAVY || size == GATE_MEDIUM;
            
        case G650:
        case FALCON_7X:
            return size == GATE_SMALL || size == GATE_REGIONAL;
            
        case EMERGENCY:
            return size == GATE_MEDIUM || size == GATE_HEAVY;
            
        default:
            return false;
    }
}

bool Gate::try_reserve(Flight* flight) {
    // CRITICAL SECTION: Reserve gate
    pthread_mutex_lock(&gate_mutex);
    
    if (!available || !is_compatible(flight)) {
        pthread_mutex_unlock(&gate_mutex);
        return false;
    }
    
    // Reserve gate
    available = false;
    current_flight = flight;
    
    pthread_mutex_unlock(&gate_mutex);
    // END CRITICAL SECTION
    
    return true;
}

void Gate::release() {
    // CRITICAL SECTION: Release gate
    pthread_mutex_lock(&gate_mutex);
    
    available = true;
    current_flight = nullptr;
    
    pthread_cond_signal(&gate_free);
    
    pthread_mutex_unlock(&gate_mutex);
    // END CRITICAL SECTION
}

bool Gate::is_available() {
    pthread_mutex_lock(&gate_mutex);
    bool avail = available;
    pthread_mutex_unlock(&gate_mutex);
    return avail;
}

Flight* Gate::get_current_flight() {
    pthread_mutex_lock(&gate_mutex);
    Flight* flight = current_flight;
    pthread_mutex_unlock(&gate_mutex);
    return flight;
}
