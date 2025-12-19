#ifndef GATE_H
#define GATE_H

#include "Flight.h"
#include <string>
#include <pthread.h>

enum GateType {
    GATE_INTERNATIONAL,
    GATE_DOMESTIC
};

enum GateSize {
    GATE_LARGE,      // A380
    GATE_HEAVY,      // B777, B747
    GATE_MEDIUM,     // B737, A320
    GATE_REGIONAL,   // Small aircraft
    GATE_SMALL       // Private jets
};

class Gate {
private:
    int id;
    GateType type;
    GateSize size;
    bool has_jetbridge;
    bool available;
    Flight* current_flight;
    
    pthread_mutex_t gate_mutex;
    pthread_cond_t gate_free;
    
public:
    Gate(int gate_id, GateType gt, GateSize gs, bool jetbridge);
    ~Gate();
    
    // Reserve gate (short critical section)
    bool try_reserve(Flight* flight);
    
    // Release gate (short critical section)
    void release();
    
    // Check compatibility
    bool is_compatible(Flight* flight);
    
    // Getters
    int get_id() const { return id; }
    GateType get_type() const { return type; }
    GateSize get_size() const { return size; }
    bool is_available();
    Flight* get_current_flight();
};

#endif // GATE_H
