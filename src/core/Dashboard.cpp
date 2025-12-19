#include "Dashboard.h"
#include <iostream>
#include <iomanip>
#include <cstring>

Dashboard::Dashboard() {
    pthread_rwlock_init(&dashboard_lock, nullptr);
    
    // Initialize metrics
    memset(&metrics, 0, sizeof(DashboardMetrics));
}

Dashboard::~Dashboard() {
    pthread_rwlock_destroy(&dashboard_lock);
}

void Dashboard::update_metrics(const DashboardMetrics& new_metrics) {
    // WRITE LOCK (exclusive)
    pthread_rwlock_wrlock(&dashboard_lock);
    
    metrics = new_metrics;
    
    pthread_rwlock_unlock(&dashboard_lock);
    // END WRITE LOCK
}

int Dashboard::get_active_flights() {
    // READ LOCK (shared)
    pthread_rwlock_rdlock(&dashboard_lock);
    
    int count = metrics.active_flights;
    
    pthread_rwlock_unlock(&dashboard_lock);
    // END READ LOCK
    
    return count;
}

double Dashboard::get_runway_utilization() {
    pthread_rwlock_rdlock(&dashboard_lock);
    double util = metrics.runway_utilization;
    pthread_rwlock_unlock(&dashboard_lock);
    return util;
}

DashboardMetrics Dashboard::get_metrics() {
    pthread_rwlock_rdlock(&dashboard_lock);
    DashboardMetrics copy = metrics;
    pthread_rwlock_unlock(&dashboard_lock);
    return copy;
}

void Dashboard::clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void Dashboard::display() {
    pthread_rwlock_rdlock(&dashboard_lock);
    // READ LOCK - can have multiple readers simultaneously
    
    clear_screen();
    
    std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║       Smart Airport OS Simulator - Real-Time Dashboard       ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "Simulation Time: " << metrics.current_sim_time << " seconds\n\n";
    
    std::cout << "┌─────────────── Flight Operations ───────────────┐\n";
    std::cout << "│ Active Flights:      " << std::setw(4) << metrics.active_flights << "                    │\n";
    std::cout << "│ At Gates:            " << std::setw(4) << metrics.flights_at_gates << "                    │\n";
    std::cout << "│ Landing:             " << std::setw(4) << metrics.flights_landing << "                    │\n";
    std::cout << "│ Departing:           " << std::setw(4) << metrics.flights_departing << "                    │\n";
    std::cout << "└──────────────────────────────────────────────────┘\n\n";
    
    std::cout << "┌─────────────── Resource Status ─────────────────┐\n";
    std::cout << "│ Available Runways:   " << std::setw(4) << metrics.available_runways << "                    │\n";
    std::cout << "│ Available Gates:     " << std::setw(4) << metrics.available_gates << "                    │\n";
    std::cout << "│ Runway Utilization:  " << std::fixed << std::setprecision(1) 
              << std::setw(5) << (metrics.runway_utilization * 100) << "%                 │\n";
    std::cout << "│ Gate Utilization:    " << std::fixed << std::setprecision(1) 
              << std::setw(5) << (metrics.gate_utilization * 100) << "%                 │\n";
    std::cout << "└──────────────────────────────────────────────────┘\n\n";
    
    std::cout << "┌─────────────── Performance Metrics ─────────────┐\n";
    std::cout << "│ Total Flights:       " << std::setw(4) << metrics.total_flights_handled << "                    │\n";
    std::cout << "│ Avg Turnaround:      " << std::fixed << std::setprecision(1) 
              << std::setw(5) << metrics.average_turnaround_time << " min              │\n";
    std::cout << "│ On-Time Performance: " << std::fixed << std::setprecision(1) 
              << std::setw(5) << (metrics.on_time_performance * 100) << "%                 │\n";
    std::cout << "└──────────────────────────────────────────────────┘\n\n";
    
    std::cout << "┌─────────────── Memory Statistics ───────────────┐\n";
    std::cout << "│ Page Faults:         " << std::setw(4) << metrics.page_fault_count << "                    │\n";
    std::cout << "│ Page Fault Rate:     " << std::fixed << std::setprecision(2) 
              << std::setw(5) << (metrics.page_fault_rate * 100) << "%                 │\n";
    std::cout << "└──────────────────────────────────────────────────┘\n";
    
    pthread_rwlock_unlock(&dashboard_lock);
    // END READ LOCK
    
    std::cout << std::flush;
}
