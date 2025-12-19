#include "Flight.h"

Flight::Flight(const std::string& id, Aircraft* ac, FlightType ft,
               long long arr_time, long long dep_time)
    : flight_id(id), aircraft(ac), flight_type(ft),
      status(SCHEDULED),
      scheduled_arrival_time(arr_time),
      actual_arrival_time(0),
      scheduled_departure_time(dep_time),
      actual_departure_time(0),
      assigned_runway_id(-1),
      assigned_gate_id(-1),
      is_delayed(false),
      delay_minutes(0) {
    
    // Set passenger count
    passenger_count = aircraft->passenger_capacity * (70 + rand() % 30) / 100; // 70-100% full
    connecting_passengers = passenger_count * 15 / 100; // 15% connecting
    
    // Set reserve fuel (30-60 minutes)
    reserve_fuel_minutes = 30 + rand() % 31;
    
    // Set priority based on aircraft type
    if (aircraft->is_emergency) {
        priority = 0; // Emergency priority
    } else if (aircraft->is_cargo) {
        priority = 60; // Normal-low priority
    } else if (aircraft->is_private) {
        priority = 80; // Low priority
    } else {
        priority = 50; // Normal priority
    }
}

bool Flight::is_emergency() const {
    return aircraft->is_emergency || reserve_fuel_minutes < 15;
}

bool Flight::needs_international_gate() const {
    return flight_type == INTERNATIONAL;
}

std::string Flight::get_status_string() const {
    switch(status) {
        case SCHEDULED: return "SCHEDULED";
        case APPROACHING: return "APPROACHING";
        case LANDING: return "LANDING";
        case TAXIING_TO_GATE: return "TAXIING_TO_GATE";
        case AT_GATE: return "AT_GATE";
        case SERVICING: return "SERVICING";
        case BOARDING: return "BOARDING";
        case TAXIING_TO_RUNWAY: return "TAXIING_TO_RUNWAY";
        case DEPARTING: return "DEPARTING";
        case DEPARTED: return "DEPARTED";
        default: return "UNKNOWN";
    }
}
