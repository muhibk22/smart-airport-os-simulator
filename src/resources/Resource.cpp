#include "Resource.h"

using namespace std;

Resource::Resource(int id, ResourceType t, const string& n) {
    resource_id = id;
    type = t;
    name = n;
    is_available = true;
    assigned_flight_id = -1;
    assignment_time = 0;
    expected_duration = 0;
    pthread_mutex_init(&resource_mutex, nullptr);
}

Resource::~Resource() {
    pthread_mutex_destroy(&resource_mutex);
}

bool Resource::try_acquire(int flight_id, long long current_time, long long duration) {
    pthread_mutex_lock(&resource_mutex);
    
    if (!is_available) {
        pthread_mutex_unlock(&resource_mutex);
        return false;
    }
    
    is_available = false;
    assigned_flight_id = flight_id;
    assignment_time = current_time;
    expected_duration = duration;
    
    pthread_mutex_unlock(&resource_mutex);
    return true;
}

void Resource::release() {
    pthread_mutex_lock(&resource_mutex);
    
    is_available = true;
    assigned_flight_id = -1;
    assignment_time = 0;
    expected_duration = 0;
    
    pthread_mutex_unlock(&resource_mutex);
}

string Resource::type_to_string(ResourceType type) {
    switch (type) {
        case RES_FUEL_TRUCK: return "Fuel Truck";
        case RES_CATERING_VEHICLE: return "Catering Vehicle";
        case RES_BAGGAGE_CART: return "Baggage Cart";
        case RES_CLEANING_CREW: return "Cleaning Crew";
        case RES_PASSENGER_BUS: return "Passenger Bus";
        case RES_AIRCRAFT_TUG: return "Aircraft Tug";
        case RES_GROUND_POWER_UNIT: return "Ground Power Unit";
        default: return "Unknown";
    }
}
