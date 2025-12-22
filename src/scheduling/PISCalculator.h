#ifndef PIS_CALCULATOR_H
#define PIS_CALCULATOR_H

#include "Operation.h"
#include <pthread.h>

using namespace std;

// PIS (Priority Index Score) Calculator
// Implements formula from README:
// PIS = α × Delay_Propagation_Factor 
//     + β × Connection_Risk_Factor
//     + γ × Resource_Utilization_Impact
//     + δ × Weather_Risk_Factor
//     + ε × Fuel_Criticality_Factor
// Constraint: α + β + γ + δ + ε = 1

class PISCalculator {
private:
    // Weights (must sum to 1.0)
    double alpha;   // Delay propagation weight (default 0.25)
    double beta;    // Connection risk weight (default 0.20)
    double gamma;   // Resource utilization weight (default 0.15)
    double delta;   // Weather risk weight (default 0.20)
    double epsilon; // Fuel criticality weight (default 0.20)
    
    // System state for calculations
    int total_flights;
    int total_connecting_passengers;
    int total_resources;
    double weather_severity;        // 0.0 to 1.0
    int time_window_affected;       // Time units affected by weather
    int emergency_threshold_minutes; // Fuel emergency threshold
    
    pthread_mutex_t calc_mutex;
    
public:
    PISCalculator();
    ~PISCalculator();
    
    // Calculate PIS for an operation
    double calculate_pis(Operation* op);
    
    // Individual factor calculations
    double calculate_delay_propagation(Operation* op);
    double calculate_connection_risk(Operation* op);
    double calculate_resource_utilization(Operation* op);
    double calculate_weather_risk(Operation* op);
    double calculate_fuel_criticality(Operation* op);
    
    // Update weights (for adaptive learning)
    void update_weights(double new_alpha, double new_beta, double new_gamma,
                       double new_delta, double new_epsilon);
    
    // Update system state
    void set_total_flights(int count);
    void set_total_connecting_passengers(int count);
    void set_total_resources(int count);
    void set_weather_severity(double severity, int time_affected);
    
    // Getters
    double get_alpha() const { return alpha; }
    double get_beta() const { return beta; }
    double get_gamma() const { return gamma; }
    double get_delta() const { return delta; }
    double get_epsilon() const { return epsilon; }
};

#endif // PIS_CALCULATOR_H
