#ifndef FLIGHT_EVENTS_H
#define FLIGHT_EVENTS_H

#include "Event.h"
#include "../airport/Flight.h"
#include "SimulationEngine.h"
#include <pthread.h>

// Forward declaration
class SimulationEngine;

// Flight lifecycle thread data
struct FlightThreadData {
    Flight* flight;
    SimulationEngine* engine;
    long long arrival_time;
};

// Thread function for flight lifecycle
void* flight_lifecycle_handler(void* arg);

// ========== Concrete Event Classes ==========

class FlightArrivalEvent : public Event {
private:
    Flight* flight;
    SimulationEngine* engine;
    
public:
    FlightArrivalEvent(Flight* f, SimulationEngine* eng, long long time);
    ~FlightArrivalEvent() override = default;
    
    void process() override;
};

class FlightDepartureEvent : public Event {
private:
    Flight* flight;
    SimulationEngine* engine;
    
public:
    FlightDepartureEvent(Flight* f, SimulationEngine* eng, long long time);
    ~FlightDepartureEvent() override = default;
    
    void process() override;
};

class ServiceStartEvent : public Event {
private:
    Flight* flight;
    std::string service_name;
    
public:
    ServiceStartEvent(Flight* f, const std::string& service, long long time);
    ~ServiceStartEvent() override = default;
    
    void process() override;
};

class ServiceEndEvent : public Event {
private:
    Flight* flight;
    std::string service_name;
    
public:
    ServiceEndEvent(Flight* f, const std::string& service, long long time);
    ~ServiceEndEvent() override = default;
    
    void process() override;
};

#endif // FLIGHT_EVENTS_H
