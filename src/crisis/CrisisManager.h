#ifndef CRISIS_MANAGER_H
#define CRISIS_MANAGER_H

#include "WeatherEvent.h"
#include "EmergencyEvent.h"
#include <vector>
#include <queue>
#include <pthread.h>

using namespace std;

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
    
    pthread_mutex_t crisis_mutex;
    
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
    
    // Status
    bool is_ground_stop() const { return ground_stop_active; }
    double get_operational_capacity() const { return current_capacity; }
    
    // Update overall status
    void update_status(long long current_time);
    
    // Statistics
    int get_active_emergency_count();
    int get_total_emergencies() const { return emergency_events.size(); }
};

#endif // CRISIS_MANAGER_H
