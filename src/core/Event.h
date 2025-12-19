#ifndef EVENT_H
#define EVENT_H

#include <string>

enum EventType {
    FLIGHT_ARRIVAL,
    FLIGHT_DEPARTURE,
    SERVICE_START,
    SERVICE_END,
    RESOURCE_REQUEST,
    RESOURCE_RELEASE,
    PAGE_FAULT,
    WEATHER_EVENT,
    EMERGENCY_EVENT,
    CRISIS_RESOLVED
};

class Event {
protected:
    EventType type;
    long long event_time;
    int priority;
    std::string description;
    
public:
    Event(EventType t, long long time, int pri = 0);
    virtual ~Event() = default;
    
    EventType get_type() const { return type; }
    long long get_time() const { return event_time; }
    int get_priority() const { return priority; }
    std::string get_description() const { return description; }
    
    // For priority queue ordering (earlier time = higher priority)
    bool operator<(const Event& other) const {
        if (event_time != other.event_time) {
            return event_time > other.event_time; // Min-heap based on time
        }
        return priority < other.priority; // Higher priority value first if same time
    }
    
    virtual void process() = 0; // Subclasses implement specific behavior
};

#endif // EVENT_H
