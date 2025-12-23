#ifndef CRISIS_MANAGER_H
#define CRISIS_MANAGER_H

#include "WeatherEvent.h"
#include "EmergencyEvent.h"
#include <vector>
#include <queue>
#include <pthread.h>

using namespace std;

// Threat Level System (5 levels as per spec)
// Level 1 (Normal): Standard operations
// Level 2 (Elevated): Enhanced screening (30% slower)
// Level 3 (High): All bags screened (50% slower)
// Level 4 (Severe): Only critical flights
// Level 5 (Critical): Complete shutdown
enum ThreatLevel {
    THREAT_LEVEL_1 = 1,  // Normal operations
    THREAT_LEVEL_2 = 2,  // Elevated - 30% slower screening
    THREAT_LEVEL_3 = 3,  // High - 50% slower screening
    THREAT_LEVEL_4 = 4,  // Severe - critical flights only
    THREAT_LEVEL_5 = 5   // Critical - complete shutdown
};

// CrisisManager handles all crisis events with priority-based handling

class CrisisManager {
private:
    vector<WeatherEvent*> weather_events;
    vector<EmergencyEvent*> emergency_events;
    
    // Priority queue for emergency processing (higher priority = process first)
    struct EmergencyCompare {
        bool operator()(EmergencyEvent* a, EmergencyEvent* b) {
            return a->get_priority() > b->get_priority();  // Lower number = higher priority
        }
    };
    priority_queue<EmergencyEvent*, vector<EmergencyEvent*>, EmergencyCompare> emergency_queue;
    
    bool ground_stop_active;
    double current_capacity;  // 0.0-1.0
    
    // Threat level system
    ThreatLevel current_threat_level;
    long long threat_level_change_time;
    int security_incident_count;
    
    pthread_mutex_t crisis_mutex;
    
    // Auto-escalation based on conditions
    void check_threat_escalation();
    
public:
    CrisisManager();
    ~CrisisManager();
    
    // Weather management
    void add_weather_event(WeatherEvent* event);
    void update_weather(long long current_time);
    WeatherEvent* get_active_weather();
    
    // Emergency management
    void report_emergency(EmergencyEvent* event);
    EmergencyEvent* get_next_emergency();
    void resolve_emergency(int event_id, long long time);
    
    // Threat Level Management
    void set_threat_level(ThreatLevel level, long long current_time);
    ThreatLevel get_threat_level() const { return current_threat_level; }
    double get_screening_multiplier() const;  // Returns time multiplier for screening
    bool is_flight_allowed(bool is_critical) const;  // Check if flight can operate
    void report_security_incident(long long current_time);  // Auto-escalate threat
    static const char* threat_level_to_string(ThreatLevel level);
    
    // Status
    bool is_ground_stop() const { return ground_stop_active; }
    double get_operational_capacity() const { return current_capacity; }
    
    // Update overall status
    void update_status(long long current_time);
    
    // Statistics
    int get_active_emergency_count();
    int get_total_emergencies() const { return emergency_events.size(); }
    int get_security_incident_count() const { return security_incident_count; }
};

#endif // CRISIS_MANAGER_H
