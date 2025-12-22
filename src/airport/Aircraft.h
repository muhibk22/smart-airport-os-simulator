#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <string>

using namespace std;

enum AircraftType {
    A380,      // Heavy, 500+ passengers
    B777,      // Heavy, 350+ passengers
    B747F,     // Cargo Heavy
    B777F,     // Cargo Heavy
    B737,      // Medium, 150-180 passengers
    A320,      // Medium, 150-180 passengers
    G650,      // Private Jet
    FALCON_7X, // Private Jet
    EMERGENCY  // Emergency aircraft
};

enum AircraftClass {
    HEAVY,
    MEDIUM,
    LIGHT
};

class Aircraft {
public:
    AircraftType type;
    AircraftClass weight_class;
    std::string type_name;
    int passenger_capacity;
    int fuel_capacity_gallons;
    int cargo_capacity_kg;
    int service_time_minutes;
    bool is_cargo;
    bool is_private;
    bool is_emergency;
    
    Aircraft(AircraftType t);
    
    std::string get_type_name() const { return type_name; }
    AircraftClass get_weight_class() const { return weight_class; }
};

#endif // AIRCRAFT_H
