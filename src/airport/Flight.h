#ifndef FLIGHT_H
#define FLIGHT_H

#include "Aircraft.h"
#include <string>

using namespace std;

enum FlightType {
    DOMESTIC,
    INTERNATIONAL
};

enum FlightStatus {
    SCHEDULED,
    APPROACHING,
    GO_AROUND,      // REQ-1: Go-around when runway unavailable or weather unsafe
    LANDING,
    TAXIING_TO_GATE,
    AT_GATE,
    SERVICING,
    BOARDING,
    TAXIING_TO_RUNWAY,
    DEPARTING,
    DEPARTED
};

class Flight {
public:
    std::string flight_id;
    Aircraft* aircraft;
    FlightType flight_type;
    FlightStatus status;
    
    long long scheduled_arrival_time;
    long long actual_arrival_time;
    long long scheduled_departure_time;
    long long actual_departure_time;
    
    int passenger_count;
    int connecting_passengers;
    int reserve_fuel_minutes;
    bool is_delayed;
    int delay_minutes;
    int go_around_count;    // REQ-1: Track number of go-arounds (max 3)
    
    // Assigned resources
    int assigned_runway_id;
    int assigned_gate_id;
    
    // Priority (0=Emergency, 100=Low)
    int priority;
    
    Flight(const std::string& id, Aircraft* ac, FlightType ft, 
           long long arr_time, long long dep_time);
    
    bool is_emergency() const;
    bool needs_international_gate() const;
    std::string get_status_string() const;
};

#endif // FLIGHT_H
