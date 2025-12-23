#include "CrisisManager.h"
#include <algorithm>

using namespace std;

CrisisManager::CrisisManager() {
    ground_stop_active = false;
    current_capacity = 1.0;
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
