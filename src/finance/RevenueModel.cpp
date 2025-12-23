#include "RevenueModel.h"

using namespace std;

RevenueModel::RevenueModel() {
    reset();
}

RevenueModel::~RevenueModel() {}

void RevenueModel::reset() {
    total_landing_revenue = 0.0;
    total_gate_revenue = 0.0;
    total_passenger_revenue = 0.0;
    total_fuel_revenue = 0.0;
    total_cargo_revenue = 0.0;
}

double RevenueModel::calculate_landing_fee(double weight_tons, bool is_international) {
    double fee = LANDING_FEE_BASE + (weight_tons * LANDING_FEE_PER_TON);
    if (is_international) {
        fee *= 1.5;  // 50% premium for international
    }
    return fee;
}

double RevenueModel::calculate_gate_fee(double hours, bool is_international) {
    double fee = hours * GATE_FEE_PER_HOUR;
    if (is_international) {
        fee *= 1.3;  // Premium for international gates
    }
    return fee;
}

double RevenueModel::calculate_passenger_fee(int passengers) {
    return passengers * PASSENGER_FEE;
}

double RevenueModel::calculate_fuel_fee(double gallons) {
    return gallons * FUEL_SURCHARGE;
}

double RevenueModel::calculate_cargo_fee(double tons) {
    return tons * CARGO_FEE_PER_TON;
}

void RevenueModel::record_landing(double weight_tons, bool is_international) {
    total_landing_revenue += calculate_landing_fee(weight_tons, is_international);
}

void RevenueModel::record_gate(double hours, bool is_international) {
    total_gate_revenue += calculate_gate_fee(hours, is_international);
}

void RevenueModel::record_passengers(int count) {
    total_passenger_revenue += calculate_passenger_fee(count);
}

void RevenueModel::record_fuel(double gallons) {
    total_fuel_revenue += calculate_fuel_fee(gallons);
}

void RevenueModel::record_cargo(double tons) {
    total_cargo_revenue += calculate_cargo_fee(tons);
}

double RevenueModel::get_total_revenue() const {
    return total_landing_revenue + total_gate_revenue + total_passenger_revenue +
           total_fuel_revenue + total_cargo_revenue;
}

double RevenueModel::calculate_profit(double total_costs) const {
    return get_total_revenue() - total_costs;
}
