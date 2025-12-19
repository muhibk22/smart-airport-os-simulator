#include "Aircraft.h"

Aircraft::Aircraft(AircraftType t) : type(t) {
    is_cargo = false;
    is_private = false;
    is_emergency = false;
    
    switch(type) {
        case A380:
            type_name = "A380";
            weight_class = HEAVY;
            passenger_capacity = 520;
            fuel_capacity_gallons = 84500;
            cargo_capacity_kg = 25000;
            service_time_minutes = 90;
            break;
            
        case B777:
            type_name = "B777";
            weight_class = HEAVY;
            passenger_capacity = 360;
            fuel_capacity_gallons = 45220;
            cargo_capacity_kg = 21000;
            service_time_minutes = 75;
            break;
            
        case B747F:
            type_name = "B747F";
            weight_class = HEAVY;
            passenger_capacity = 0;
            fuel_capacity_gallons = 48450;
            cargo_capacity_kg = 124000;
            service_time_minutes = 120;
            is_cargo = true;
            break;
            
        case B777F:
            type_name = "B777F";
            weight_class = HEAVY;
            passenger_capacity = 0;
            fuel_capacity_gallons = 47890;
            cargo_capacity_kg = 102000;
            service_time_minutes = 100;
            is_cargo = true;
            break;
            
        case B737:
            type_name = "B737";
            weight_class = MEDIUM;
            passenger_capacity = 175;
            fuel_capacity_gallons = 6875;
            cargo_capacity_kg = 4000;
            service_time_minutes = 45;
            break;
            
        case A320:
            type_name = "A320";
            weight_class = MEDIUM;
            passenger_capacity = 150;
            fuel_capacity_gallons = 6400;
            cargo_capacity_kg = 3500;
            service_time_minutes = 40;
            break;
            
        case G650:
            type_name = "G650";
            weight_class = LIGHT;
            passenger_capacity = 19;
            fuel_capacity_gallons = 7000;
            cargo_capacity_kg = 500;
            service_time_minutes = 20;
            is_private = true;
            break;
            
        case FALCON_7X:
            type_name = "Falcon 7X";
            weight_class = LIGHT;
            passenger_capacity = 16;
            fuel_capacity_gallons = 7000;
            cargo_capacity_kg = 400;
            service_time_minutes = 18;
            is_private = true;
            break;
            
        case EMERGENCY:
            type_name = "EMERGENCY";
            weight_class = MEDIUM;
            passenger_capacity = 100;
            fuel_capacity_gallons = 5000;
            cargo_capacity_kg = 1000;
            service_time_minutes = 30;
            is_emergency = true;
            break;
    }
}
