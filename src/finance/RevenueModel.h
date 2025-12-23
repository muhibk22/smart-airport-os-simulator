#ifndef REVENUE_MODEL_H
#define REVENUE_MODEL_H

#include <string>

using namespace std;

// RevenueModel calculates airport revenue

class RevenueModel {
private:
    // Revenue rates ($/unit)
    static constexpr double LANDING_FEE_BASE = 500.0;
    static constexpr double LANDING_FEE_PER_TON = 10.0;
    static constexpr double GATE_FEE_PER_HOUR = 200.0;
    static constexpr double PASSENGER_FEE = 5.0;
    static constexpr double FUEL_SURCHARGE = 0.50;  // $/gallon
    static constexpr double CARGO_FEE_PER_TON = 50.0;
    static constexpr double PARKING_FEE_PER_HOUR = 25.0;
    
    double total_landing_revenue;
    double total_gate_revenue;
    double total_passenger_revenue;
    double total_fuel_revenue;
    double total_cargo_revenue;
    
public:
    RevenueModel();
    ~RevenueModel();
    
    // Calculate revenue
    double calculate_landing_fee(double weight_tons, bool is_international);
    double calculate_gate_fee(double hours, bool is_international);
    double calculate_passenger_fee(int passengers);
    double calculate_fuel_fee(double gallons);
    double calculate_cargo_fee(double tons);
    
    // Record revenue
    void record_landing(double weight_tons, bool is_international);
    void record_gate(double hours, bool is_international);
    void record_passengers(int count);
    void record_fuel(double gallons);
    void record_cargo(double tons);
    
    // Get totals
    double get_total_revenue() const;
    double get_landing_revenue() const { return total_landing_revenue; }
    double get_passenger_revenue() const { return total_passenger_revenue; }
    
    // Profit calculation (needs cost model)
    double calculate_profit(double total_costs) const;
    
    // Reset
    void reset();
};

#endif // REVENUE_MODEL_H
