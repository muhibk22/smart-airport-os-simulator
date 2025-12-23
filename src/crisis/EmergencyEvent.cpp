#include "EmergencyEvent.h"

using namespace std;

EmergencyEvent::EmergencyEvent(int id, EmergencyType t, int flight_id, long long time) {
    event_id = id;
    type = t;
    affected_flight_id = flight_id;
    reported_time = time;
    resolved_time = 0;
    is_resolved = false;
    affected_gate_id = -1;
    affected_runway_id = -1;
    
    // Set default priority based on type
    switch (type) {
        case EMER_FIRE:
        case EMER_EVACUATION:
            priority = EMER_PRIORITY_CRITICAL;
            break;
        case EMER_SECURITY:
        case EMER_FUEL:
        case EMER_MECHANICAL:
            priority = EMER_PRIORITY_HIGH;
            break;
        case EMER_MEDICAL:
        case EMER_BIRD_STRIKE:
            priority = EMER_PRIORITY_MEDIUM;
            break;
        default:
            priority = EMER_PRIORITY_LOW;
    }
}

EmergencyEvent::~EmergencyEvent() {}

void EmergencyEvent::resolve(long long time) {
    is_resolved = true;
    resolved_time = time;
}

void EmergencyEvent::escalate() {
    if (priority > EMER_PRIORITY_CRITICAL) {
        priority = (EmergencyPriority)(priority - 1);
    }
}

void EmergencyEvent::set_priority(EmergencyPriority p) {
    priority = p;
}

bool EmergencyEvent::requires_runway_closure() {
    return type == EMER_FIRE || type == EMER_BIRD_STRIKE || 
           type == EMER_MECHANICAL;
}

bool EmergencyEvent::requires_gate_closure() {
    return type == EMER_FIRE || type == EMER_SECURITY || 
           type == EMER_EVACUATION;
}

bool EmergencyEvent::affects_other_flights() {
    return priority <= EMER_PRIORITY_HIGH;
}

int EmergencyEvent::get_estimated_resolution_minutes() {
    switch (type) {
        case EMER_MEDICAL: return 30;
        case EMER_SECURITY: return 60;
        case EMER_MECHANICAL: return 45;
        case EMER_FUEL: return 20;
        case EMER_BIRD_STRIKE: return 90;
        case EMER_FIRE: return 120;
        case EMER_EVACUATION: return 180;
        default: return 60;
    }
}

string EmergencyEvent::type_to_string(EmergencyType type) {
    switch (type) {
        case EMER_MEDICAL: return "Medical Emergency";
        case EMER_SECURITY: return "Security Threat";
        case EMER_MECHANICAL: return "Mechanical Issue";
        case EMER_FUEL: return "Low Fuel Emergency";
        case EMER_BIRD_STRIKE: return "Bird Strike";
        case EMER_FIRE: return "Fire";
        case EMER_EVACUATION: return "Evacuation";
        default: return "Unknown";
    }
}
