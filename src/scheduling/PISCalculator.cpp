#include "PISCalculator.h"
#include <cmath>
#include <algorithm>

using namespace std;

PISCalculator::PISCalculator() {
    // Default weights from README (sum = 1.0)
    alpha = 0.25;   // Delay propagation
    beta = 0.20;    // Connection risk
    gamma = 0.15;   // Resource utilization
    delta = 0.20;   // Weather risk
    epsilon = 0.20; // Fuel criticality
    
    // Initial system state
    total_flights = 50;
    total_connecting_passengers = 1000;
    total_resources = 100;
    weather_severity = 0.0;
    time_window_affected = 0;
    emergency_threshold_minutes = 30;
    
    pthread_mutex_init(&calc_mutex, nullptr);
}

PISCalculator::~PISCalculator() {
    pthread_mutex_destroy(&calc_mutex);
}

double PISCalculator::calculate_pis(Operation* op) {
    pthread_mutex_lock(&calc_mutex);
    
    // PIS = α × DPF + β × CRF + γ × RUI + δ × WRF + ε × FCF
    double pis = 0.0;
    
    pis += alpha * calculate_delay_propagation(op);
    pis += beta * calculate_connection_risk(op);
    pis += gamma * calculate_resource_utilization(op);
    pis += delta * calculate_weather_risk(op);
    pis += epsilon * calculate_fuel_criticality(op);
    
    pthread_mutex_unlock(&calc_mutex);
    
    return pis;
}

double PISCalculator::calculate_delay_propagation(Operation* op) {
    // Delay_Propagation_Factor = Number_of_Affected_Flights / Total_Flights
    if (op == nullptr || op->flight == nullptr) return 0.0;
    
    // Estimate affected flights based on flight type and delay
    int affected_flights = 0;
    
    if (op->flight->flight_type == INTERNATIONAL) {
        // International flights affect more connections
        affected_flights = 5 + (op->wait_time / 60); // More delay = more affected
    } else if (op->flight->flight_type == DOMESTIC) {
        affected_flights = 2 + (op->wait_time / 120);
    }
    
    // Emergency flights could affect many
    if (op->flight->is_emergency()) {
        affected_flights = 10;
    }
    
    if (total_flights <= 0) return 0.0;
    return min(1.0, (double)affected_flights / total_flights);
}

double PISCalculator::calculate_connection_risk(Operation* op) {
    // Connection_Risk_Factor = Number_Passengers_at_Risk / Total_Connecting_Passengers
    if (op == nullptr || op->flight == nullptr) return 0.0;
    
    int passengers_at_risk = 0;
    
    // Estimate based on passenger count and delay
    if (op->wait_time > 30) { // More than 30 time units waiting
        passengers_at_risk = op->flight->passenger_count / 4; // 25% at risk
    }
    if (op->wait_time > 60) {
        passengers_at_risk = op->flight->passenger_count / 2; // 50% at risk
    }
    if (op->wait_time > 90) {
        passengers_at_risk = op->flight->passenger_count; // All at risk
    }
    
    if (total_connecting_passengers <= 0) return 0.0;
    return min(1.0, (double)passengers_at_risk / total_connecting_passengers);
}

double PISCalculator::calculate_resource_utilization(Operation* op) {
    // Resource_Utilization_Impact = Resources_Blocked / Total_Resources
    if (op == nullptr) return 0.0;
    
    int resources_blocked = 0;
    
    // Estimate based on operation type
    switch (op->type) {
        case OP_LANDING:
        case OP_TAKEOFF:
            resources_blocked = 10; // Runway + support vehicles
            break;
        case OP_GATE_ARRIVAL:
        case OP_GATE_DEPARTURE:
            resources_blocked = 5; // Gate + ground crew
            break;
        case OP_REFUELING:
            resources_blocked = 3; // Fuel truck + crew
            break;
        case OP_CLEANING:
        case OP_CATERING:
            resources_blocked = 2;
            break;
        default:
            resources_blocked = 1;
    }
    
    if (total_resources <= 0) return 0.0;
    return min(1.0, (double)resources_blocked / total_resources);
}

double PISCalculator::calculate_weather_risk(Operation* op) {
    // Weather_Risk_Factor = (Weather_Severity × Time_Window_Affected) / Total_Resources
    if (op == nullptr) return 0.0;
    
    // Outdoor operations affected more by weather
    double weather_impact = weather_severity;
    
    if (op->type == OP_LANDING || op->type == OP_TAKEOFF || op->type == OP_TAXIING) {
        weather_impact *= 1.5; // Higher impact for runway operations
    }
    
    if (total_resources <= 0) return 0.0;
    return min(1.0, (weather_impact * time_window_affected) / total_resources);
}

double PISCalculator::calculate_fuel_criticality(Operation* op) {
    // Fuel_Criticality_Factor = (Reserve_Fuel_Minutes - Emergency_Threshold) / Reserve_Fuel_Minutes
    if (op == nullptr || op->flight == nullptr) return 0.0;
    
    int reserve_fuel = op->flight->reserve_fuel_minutes;
    
    // Convert fuel to approximate minutes (simplified)
    // Assume fuel consumption rate based on aircraft type
    int reserve_minutes = reserve_fuel / 100; // Simplified conversion
    
    if (reserve_minutes <= 0) return 1.0; // Maximum criticality
    
    double factor = (double)(reserve_minutes - emergency_threshold_minutes) / reserve_minutes;
    
    // Invert so low fuel = high score
    return max(0.0, 1.0 - factor);
}

void PISCalculator::update_weights(double new_alpha, double new_beta, double new_gamma,
                                   double new_delta, double new_epsilon) {
    pthread_mutex_lock(&calc_mutex);
    
    // Verify sum = 1.0 (with tolerance)
    double sum = new_alpha + new_beta + new_gamma + new_delta + new_epsilon;
    if (abs(sum - 1.0) < 0.01) {
        alpha = new_alpha;
        beta = new_beta;
        gamma = new_gamma;
        delta = new_delta;
        epsilon = new_epsilon;
    }
    
    pthread_mutex_unlock(&calc_mutex);
}

void PISCalculator::set_total_flights(int count) {
    pthread_mutex_lock(&calc_mutex);
    total_flights = max(1, count);
    pthread_mutex_unlock(&calc_mutex);
}

void PISCalculator::set_total_connecting_passengers(int count) {
    pthread_mutex_lock(&calc_mutex);
    total_connecting_passengers = max(1, count);
    pthread_mutex_unlock(&calc_mutex);
}

void PISCalculator::set_total_resources(int count) {
    pthread_mutex_lock(&calc_mutex);
    total_resources = max(1, count);
    pthread_mutex_unlock(&calc_mutex);
}

void PISCalculator::set_weather_severity(double severity, int time_affected) {
    pthread_mutex_lock(&calc_mutex);
    weather_severity = max(0.0, min(1.0, severity));
    time_window_affected = max(0, time_affected);
    pthread_mutex_unlock(&calc_mutex);
}
