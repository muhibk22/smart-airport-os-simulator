#include "CostModel.h"

using namespace std;

CostModel::CostModel() {
    reset();
}

CostModel::~CostModel() {}

void CostModel::reset() {
    total_fuel_cost = 0.0;
    total_crew_cost = 0.0;
    total_facility_cost = 0.0;
    total_delay_cost = 0.0;
    total_emergency_cost = 0.0;
}

double CostModel::calculate_fuel_cost(double gallons) {
    return gallons * FUEL_COST_PER_GALLON;
}

double CostModel::calculate_crew_cost(double hours, int crew_count) {
    return hours * CREW_COST_PER_HOUR * crew_count;
}

double CostModel::calculate_gate_cost(double hours) {
    return hours * GATE_COST_PER_HOUR;
}

double CostModel::calculate_delay_cost(int delay_minutes, int passengers) {
    // More passengers = higher delay cost (passenger compensation)
    double passenger_factor = 1.0 + (passengers / 100.0);
    return delay_minutes * DELAY_COST_PER_MINUTE * passenger_factor;
}

double CostModel::calculate_emergency_cost(int severity) {
    // severity 1-5, higher = more expensive
    return EMERGENCY_COST_BASE * severity;
}

double CostModel::calculate_turnaround_cost(int services_count) {
    return services_count * GROUND_SERVICE_COST;
}

void CostModel::record_fuel(double gallons) {
    total_fuel_cost += calculate_fuel_cost(gallons);
}

void CostModel::record_crew(double hours, int count) {
    total_crew_cost += calculate_crew_cost(hours, count);
}

void CostModel::record_gate(double hours) {
    total_facility_cost += calculate_gate_cost(hours);
}

void CostModel::record_delay(int minutes, int passengers) {
    total_delay_cost += calculate_delay_cost(minutes, passengers);
}

void CostModel::record_emergency(int severity) {
    total_emergency_cost += calculate_emergency_cost(severity);
}

double CostModel::get_total_cost() const {
    return total_fuel_cost + total_crew_cost + total_facility_cost + 
           total_delay_cost + total_emergency_cost;
}
