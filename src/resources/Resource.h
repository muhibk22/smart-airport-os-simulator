#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <pthread.h>

using namespace std;

// Resource types from README (7 types)
enum ResourceType {
    RES_FUEL_TRUCK,
    RES_CATERING_VEHICLE,
    RES_BAGGAGE_CART,
    RES_CLEANING_CREW,
    RES_PASSENGER_BUS,
    RES_AIRCRAFT_TUG,
    RES_GROUND_POWER_UNIT
};

class Resource {
private:
    int resource_id;
    ResourceType type;
    string name;
    
    bool is_available;
    int assigned_flight_id;  // -1 if unassigned
    
    long long assignment_time;
    long long expected_duration;
    
    pthread_mutex_t resource_mutex;
    
public:
    Resource(int id, ResourceType t, const string& n);
    ~Resource();
    
    // Allocation
    bool try_acquire(int flight_id, long long current_time, long long duration);
    void release();
    
    // Getters
    int get_id() const { return resource_id; }
    ResourceType get_type() const { return type; }
    string get_name() const { return name; }
    bool get_available() const { return is_available; }
    int get_assigned_flight() const { return assigned_flight_id; }
    long long get_assignment_time() const { return assignment_time; }
    long long get_expected_duration() const { return expected_duration; }
    
    static string type_to_string(ResourceType type);
};

#endif // RESOURCE_H
