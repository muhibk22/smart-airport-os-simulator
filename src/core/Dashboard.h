#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <pthread.h>
#include <string>

struct DashboardMetrics {
    int active_flights;
    int flights_at_gates;
    int flights_landing;
    int flights_departing;
    
    int available_runways;
    int available_gates;
    
    double runway_utilization;
    double gate_utilization;
    
    int total_flights_handled;
    double average_turnaround_time;
    double on_time_performance;
    
    int page_fault_count;
    double page_fault_rate;
    
    long long current_sim_time;
};

class Dashboard {
private:
    DashboardMetrics metrics;
    pthread_rwlock_t dashboard_lock;  // Reader-writer lock
    
    void clear_screen();
    
public:
    Dashboard();
    ~Dashboard();
    
    // Update metrics (write lock - exclusive)
    void update_metrics(const DashboardMetrics& new_metrics);
    
    // Read specific metric (read lock - shared)
    int get_active_flights();
    double get_runway_utilization();
    
    // Display dashboard (read lock - shared)
    void display();
    
    // Get full metrics snapshot (read lock - shared)
    DashboardMetrics get_metrics();
};

#endif // DASHBOARD_H
