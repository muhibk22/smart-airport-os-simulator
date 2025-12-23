#ifndef COST_MODEL_H
#define COST_MODEL_H

#include <string>

using namespace std;

// CostModel calculates operational costs

class CostModel {
private:
    // Cost rates ($/unit)
    static constexpr double FUEL_COST_PER_GALLON = 3.50;
    static constexpr double CREW_COST_PER_HOUR = 75.0;
    static constexpr double GATE_COST_PER_HOUR = 150.0;
    static constexpr double RUNWAY_COST_PER_OP = 50.0;
    static constexpr double GROUND_SERVICE_COST = 500.0;
    static constexpr double DELAY_COST_PER_MINUTE = 1.0;  // Reduced from $100 for realistic balance
    static constexpr double EMERGENCY_COST_BASE = 5000.0;
    
    double total_fuel_cost;
    double total_crew_cost;
    double total_facility_cost;
    double total_delay_cost;
    double total_emergency_cost;
    
public:
    CostModel();
    ~CostModel();
    
    // Calculate costs
    double calculate_fuel_cost(double gallons);
    double calculate_crew_cost(double hours, int crew_count);
    double calculate_gate_cost(double hours);
    double calculate_delay_cost(int delay_minutes, int passengers);
    double calculate_emergency_cost(int severity);
    double calculate_turnaround_cost(int services_count);
    
    // Record costs
    void record_fuel(double gallons);
    void record_crew(double hours, int count);
    void record_gate(double hours);
    void record_delay(int minutes, int passengers);
    void record_emergency(int severity);
    
    // Get totals
    double get_total_cost() const;
    double get_fuel_cost() const { return total_fuel_cost; }
    double get_crew_cost() const { return total_crew_cost; }
    double get_delay_cost() const { return total_delay_cost; }
    
    // Reset
    void reset();
};

#endif // COST_MODEL_H
