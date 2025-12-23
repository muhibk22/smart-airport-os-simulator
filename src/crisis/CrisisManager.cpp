#include "CrisisManager.h"
#include <algorithm>

using namespace std;

CrisisManager::CrisisManager() {
    ground_stop_active = false;
    current_capacity = 1.0;
    
    // Initialize threat level system
    current_threat_level = THREAT_LEVEL_1;
    threat_level_change_time = 0;
    security_incident_count = 0;
    
    pthread_mutex_init(&crisis_mutex, nullptr);
}

CrisisManager::~CrisisManager() {
    pthread_mutex_destroy(&crisis_mutex);
    for (WeatherEvent* w : weather_events) delete w;
    for (EmergencyEvent* e : emergency_events) delete e;
}

void CrisisManager::add_weather_event(WeatherEvent* event) {
    pthread_mutex_lock(&crisis_mutex);
    weather_events.push_back(event);
    event->activate();
    update_status(0);
    pthread_mutex_unlock(&crisis_mutex);
}

void CrisisManager::update_weather(long long current_time) {
    pthread_mutex_lock(&crisis_mutex);
    
    for (WeatherEvent* w : weather_events) {
        w->check_expired(current_time);
    }
    
    update_status(current_time);
    pthread_mutex_unlock(&crisis_mutex);
}

WeatherEvent* CrisisManager::get_active_weather() {
    pthread_mutex_lock(&crisis_mutex);
    
    WeatherEvent* worst = nullptr;
    for (WeatherEvent* w : weather_events) {
        if (w->get_active()) {
            if (worst == nullptr || w->get_severity() > worst->get_severity()) {
                worst = w;
            }
        }
    }
    
    pthread_mutex_unlock(&crisis_mutex);
    return worst;
}

void CrisisManager::report_emergency(EmergencyEvent* event) {
    pthread_mutex_lock(&crisis_mutex);
    emergency_events.push_back(event);
    emergency_queue.push(event);
    update_status(0);
    pthread_mutex_unlock(&crisis_mutex);
}

EmergencyEvent* CrisisManager::get_next_emergency() {
    pthread_mutex_lock(&crisis_mutex);
    
    EmergencyEvent* next = nullptr;
    if (!emergency_queue.empty()) {
        next = emergency_queue.top();
        if (!next->is_active()) {
            emergency_queue.pop();
            next = nullptr;
        }
    }
    
    pthread_mutex_unlock(&crisis_mutex);
    return next;
}

void CrisisManager::resolve_emergency(int event_id, long long time) {
    pthread_mutex_lock(&crisis_mutex);
    
    for (EmergencyEvent* e : emergency_events) {
        if (e->get_id() == event_id) {
            e->resolve(time);
            break;
        }
    }
    
    update_status(time);
    pthread_mutex_unlock(&crisis_mutex);
}

void CrisisManager::update_status(long long current_time) {
    // Calculate operational capacity based on active crises
    current_capacity = 1.0;
    ground_stop_active = false;
    
    // Check weather impact
    for (WeatherEvent* w : weather_events) {
        if (w->get_active()) {
            current_capacity -= w->get_capacity_reduction();
            if (w->requires_ground_stop()) {
                ground_stop_active = true;
            }
        }
    }
    
    // Check emergency impact
    for (EmergencyEvent* e : emergency_events) {
        if (e->is_active()) {
            if (e->get_priority() == EMER_PRIORITY_CRITICAL) {
                current_capacity -= 0.3;
            } else if (e->get_priority() == EMER_PRIORITY_HIGH) {
                current_capacity -= 0.1;
            }
        }
    }
    
    current_capacity = max(0.0, min(1.0, current_capacity));
    
    if (current_capacity < 0.1) {
        ground_stop_active = true;
    }
}

int CrisisManager::get_active_emergency_count() {
    pthread_mutex_lock(&crisis_mutex);
    
    int count = 0;
    for (EmergencyEvent* e : emergency_events) {
        if (e->is_active()) count++;
    }
    
    pthread_mutex_unlock(&crisis_mutex);
    return count;
}

// ============================================================================
// THREAT LEVEL MANAGEMENT SYSTEM
// Implements 5-level security system as per spec
// ============================================================================

void CrisisManager::set_threat_level(ThreatLevel level, long long current_time) {
    pthread_mutex_lock(&crisis_mutex);
    
    current_threat_level = level;
    threat_level_change_time = current_time;
    
    // Level 5 = complete shutdown
    if (level == THREAT_LEVEL_5) {
        ground_stop_active = true;
        current_capacity = 0.0;
    }
    // Level 4 = critical flights only
    else if (level == THREAT_LEVEL_4) {
        current_capacity = 0.2;  // Only 20% capacity for critical flights
    }
    // Level 3 = 50% slower
    else if (level == THREAT_LEVEL_3) {
        current_capacity = 0.5;
    }
    // Level 2 = 30% slower
    else if (level == THREAT_LEVEL_2) {
        current_capacity = 0.7;
    }
    // Level 1 = normal
    else {
        current_capacity = 1.0;
    }
    
    pthread_mutex_unlock(&crisis_mutex);
}

double CrisisManager::get_screening_multiplier() const {
    // Return time multiplier for security screening based on threat level
    // Higher multiplier = longer screening times
    switch (current_threat_level) {
        case THREAT_LEVEL_1:
            return 1.0;   // Normal operations (100% speed)
        case THREAT_LEVEL_2:
            return 1.3;   // Enhanced screening (30% slower)
        case THREAT_LEVEL_3:
            return 1.5;   // All bags screened (50% slower)
        case THREAT_LEVEL_4:
            return 2.0;   // Extensive screening (100% slower)
        case THREAT_LEVEL_5:
            return 999.0; // Complete shutdown (no screening possible)
        default:
            return 1.0;
    }
}

bool CrisisManager::is_flight_allowed(bool is_critical) const {
    switch (current_threat_level) {
        case THREAT_LEVEL_1:
        case THREAT_LEVEL_2:
        case THREAT_LEVEL_3:
            return true;  // All flights allowed
        case THREAT_LEVEL_4:
            return is_critical;  // Only critical flights
        case THREAT_LEVEL_5:
            return false; // Complete shutdown - no flights
        default:
            return true;
    }
}

void CrisisManager::report_security_incident(long long current_time) {
    pthread_mutex_lock(&crisis_mutex);
    
    security_incident_count++;
    
    // Auto-escalate based on incident count
    check_threat_escalation();
    
    pthread_mutex_unlock(&crisis_mutex);
}

void CrisisManager::check_threat_escalation() {
    // Auto-escalate threat level based on security incident count
    // This is called with mutex already held
    
    ThreatLevel new_level = current_threat_level;
    
    if (security_incident_count >= 5) {
        new_level = THREAT_LEVEL_5;
    } else if (security_incident_count >= 4) {
        new_level = THREAT_LEVEL_4;
    } else if (security_incident_count >= 3) {
        new_level = THREAT_LEVEL_3;
    } else if (security_incident_count >= 2) {
        new_level = THREAT_LEVEL_2;
    } else {
        new_level = THREAT_LEVEL_1;
    }
    
    // Only escalate, never auto-de-escalate
    if (new_level > current_threat_level) {
        current_threat_level = new_level;
        
        // Update capacity based on new level
        if (new_level == THREAT_LEVEL_5) {
            ground_stop_active = true;
            current_capacity = 0.0;
        } else if (new_level == THREAT_LEVEL_4) {
            current_capacity = 0.2;
        } else if (new_level == THREAT_LEVEL_3) {
            current_capacity = 0.5;
        } else if (new_level == THREAT_LEVEL_2) {
            current_capacity = 0.7;
        }
    }
}

const char* CrisisManager::threat_level_to_string(ThreatLevel level) {
    switch (level) {
        case THREAT_LEVEL_1: return "Level 1 (Normal)";
        case THREAT_LEVEL_2: return "Level 2 (Elevated)";
        case THREAT_LEVEL_3: return "Level 3 (High)";
        case THREAT_LEVEL_4: return "Level 4 (Severe)";
        case THREAT_LEVEL_5: return "Level 5 (Critical)";
        default: return "Unknown";
    }
}
