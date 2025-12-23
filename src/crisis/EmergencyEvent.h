#ifndef EMERGENCY_EVENT_H
#define EMERGENCY_EVENT_H

#include <string>

using namespace std;

enum EmergencyType {
    EMER_MEDICAL,           // Passenger medical
    EMER_SECURITY,          // Security threat
    EMER_MECHANICAL,        // Aircraft issue
    EMER_FUEL,              // Low fuel
    EMER_BIRD_STRIKE,       // Bird impact
    EMER_FIRE,              // Fire on aircraft/airport
    EMER_EVACUATION         // Terminal evacuation
};

enum EmergencyPriority {
    EMER_PRIORITY_CRITICAL,   // Immediate action
    EMER_PRIORITY_HIGH,       // Urgent
    EMER_PRIORITY_MEDIUM,     // Important
    EMER_PRIORITY_LOW         // Advisory
};

class EmergencyEvent {
private:
    int event_id;
    EmergencyType type;
    EmergencyPriority priority;
    string description;
    
    int affected_flight_id;
    int affected_gate_id;
    int affected_runway_id;
    
    long long reported_time;
    long long resolved_time;
    bool is_resolved;
    
public:
    EmergencyEvent(int id, EmergencyType t, int flight_id, long long time);
    ~EmergencyEvent();
    
    // Lifecycle
    void resolve(long long time);
    bool is_active() const { return !is_resolved; }
    
    // Priority escalation
    void escalate();
    void set_priority(EmergencyPriority p);
    
    // Impact
    bool requires_runway_closure();
    bool requires_gate_closure();
    bool affects_other_flights();
    int get_estimated_resolution_minutes();
    
    // Getters
    int get_id() const { return event_id; }
    EmergencyType get_type() const { return type; }
    EmergencyPriority get_priority() const { return priority; }
    int get_affected_flight() const { return affected_flight_id; }
    
    static string type_to_string(EmergencyType type);
};

#endif // EMERGENCY_EVENT_H
