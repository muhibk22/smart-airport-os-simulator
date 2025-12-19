#ifndef RUNWAY_H
#define RUNWAY_H

#include "Flight.h"
#include <string>
#include <pthread.h>

class Runway {
private:
    int id;
    std::string name;
    bool available;
    Flight* current_flight;
    
    // Wake turbulence tracking
    long long last_departure_time;
    AircraftClass last_aircraft_class;
    
    // Per-runway mutex (fine-grained locking)
    pthread_mutex_t runway_mutex;
    pthread_cond_t runway_clear;
    
public:
    Runway(int runway_id, const std::string& runway_name);
    ~Runway();
    
    // Reserve runway (short critical section)
    bool try_reserve(Flight* flight, long long current_time);
    
    // Release runway (short critical section)
    void release(long long current_time);
    
    // Check wake turbulence separation requirement
    int get_wake_separation_time(AircraftClass trailing_class);
    
    // Getters
    int get_id() const { return id; }
    std::string get_name() const { return name; }
    bool is_available();
    Flight* get_current_flight();
};

#endif // RUNWAY_H
