#include "Runway.h"
#include <unistd.h>

// Wake turbulence separation matrix (in seconds)
// [Leading][Trailing] = separation time
static const int WAKE_SEPARATION[3][3] = {
    // Leading: HEAVY    MEDIUM  LIGHT
    /*HEAVY*/   {90,      120,    180},
    /*MEDIUM*/  {60,      60,     90},
    /*LIGHT*/   {60,      60,     60}
};

Runway::Runway(int runway_id, const std::string& runway_name)
    : id(runway_id), name(runway_name), available(true),
      current_flight(nullptr), last_departure_time(0),
      last_aircraft_class(LIGHT) {
    
    pthread_mutex_init(&runway_mutex, nullptr);
    pthread_cond_init(&runway_clear, nullptr);
}

Runway::~Runway() {
    pthread_mutex_destroy(&runway_mutex);
    pthread_cond_destroy(&runway_clear);
}

bool Runway::try_reserve(Flight* flight, long long current_time) {
    // CRITICAL SECTION: Reserve runway
    pthread_mutex_lock(&runway_mutex);
    
    if (!available) {
        pthread_mutex_unlock(&runway_mutex);
        return false; // Runway occupied
    }
    
    // Check wake turbulence separation
    int required_separation = get_wake_separation_time(flight->aircraft->weight_class);
    long long time_since_last = current_time - last_departure_time;
    
    if (time_since_last < required_separation) {
        // Must wait for wake turbulence to dissipate
        struct timespec timeout;
        timeout.tv_sec = (required_separation - time_since_last) / 1000;
        timeout.tv_nsec = ((required_separation - time_since_last) % 1000) * 1000000;
        
        int result = pthread_cond_timedwait(&runway_clear, &runway_mutex, &timeout);
        
        if (result != 0 || !available) {
            pthread_mutex_unlock(&runway_mutex);
            return false;
        }
    }
    
    // Reserve runway
    available = false;
    current_flight = flight;
    
    pthread_mutex_unlock(&runway_mutex);
    // END CRITICAL SECTION - mutex released!
    
    return true;
}

void Runway::release(long long current_time) {
    // CRITICAL SECTION: Release runway
    pthread_mutex_lock(&runway_mutex);
    
    if (current_flight) {
        last_aircraft_class = current_flight->aircraft->weight_class;
    }
    
    available = true;
    current_flight = nullptr;
    last_departure_time = current_time;
    
    // Wake up waiting flights
    pthread_cond_broadcast(&runway_clear);
    
    pthread_mutex_unlock(&runway_mutex);
    // END CRITICAL SECTION
}

int Runway::get_wake_separation_time(AircraftClass trailing_class) {
    // Called with mutex held
    return WAKE_SEPARATION[last_aircraft_class][trailing_class];
}

bool Runway::is_available() {
    pthread_mutex_lock(&runway_mutex);
    bool avail = available;
    pthread_mutex_unlock(&runway_mutex);
    return avail;
}

Flight* Runway::get_current_flight() {
    pthread_mutex_lock(&runway_mutex);
    Flight* flight = current_flight;
    pthread_mutex_unlock(&runway_mutex);
    return flight;
}
