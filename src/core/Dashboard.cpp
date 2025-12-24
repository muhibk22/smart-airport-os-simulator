#include "Dashboard.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>

using namespace std;

// ANSI Color Codes
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"

Dashboard::Dashboard() {
    pthread_rwlock_init(&dashboard_lock, nullptr);
    
    // Initialize all metrics to zero
    metrics.active_flights = 0;
    metrics.flights_at_gates = 0;
    metrics.flights_landing = 0;
    metrics.flights_departing = 0;
    metrics.available_runways = 0;
    metrics.available_gates = 0;
    metrics.runway_utilization = 0.0;
    metrics.gate_utilization = 0.0;
    metrics.total_flights_handled = 0;
    metrics.average_turnaround_time = 0.0;
    metrics.on_time_performance = 0.0;
    metrics.page_fault_count = 0;
    metrics.page_fault_rate = 0.0;
    metrics.current_sim_time = 0;
}

Dashboard::~Dashboard() {
    pthread_rwlock_destroy(&dashboard_lock);
}

void Dashboard::update_metrics(const DashboardMetrics& new_metrics) {
    pthread_rwlock_wrlock(&dashboard_lock);
    metrics = new_metrics;
    pthread_rwlock_unlock(&dashboard_lock);
}

int Dashboard::get_active_flights() {
    pthread_rwlock_rdlock(&dashboard_lock);
    int count = metrics.active_flights;
    pthread_rwlock_unlock(&dashboard_lock);
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
    // Use ANSI escape to clear and move cursor to top
    std::cout << "\033[2J\033[H";
#endif
}

// Helper function to create a progress bar
string create_progress_bar(double percent, int width = 20) {
    int filled = (int)(percent / 100.0 * width);
    if (filled > width) filled = width;
    if (filled < 0) filled = 0;
    
    stringstream bar;
    bar << "[";
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            if (percent > 80) bar << RED << "█" << RESET;
            else if (percent > 50) bar << YELLOW << "█" << RESET;
            else bar << GREEN << "█" << RESET;
        } else {
            bar << DIM << "░" << RESET;
        }
    }
    bar << "]";
    return bar.str();
}

// Helper to get color based on value
const char* get_status_color(int value, int threshold_warn = 3, int threshold_good = 1) {
    if (value >= threshold_warn) return GREEN;
    if (value >= threshold_good) return YELLOW;
    return RED;
}

void Dashboard::display() {
    pthread_rwlock_rdlock(&dashboard_lock);
    
    clear_screen();
    
    // Header with color
    std::cout << BOLD << CYAN;
    std::cout << "╔═══════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║" << WHITE << "     ✈️  Smart Airport OS Simulator - Real-Time Dashboard  ✈️           " << CYAN << "║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════════╝\n" << RESET;
    
    // Time display
    int hours = metrics.current_sim_time / 3600;
    int mins = (metrics.current_sim_time % 3600) / 60;
    int secs = metrics.current_sim_time % 60;
    
    std::cout << "\n  " << BOLD << "⏱  Simulation Time: " << CYAN;
    std::cout << std::setfill('0') << std::setw(2) << hours << ":"
              << std::setw(2) << mins << ":"
              << std::setw(2) << secs << RESET;
    std::cout << std::setfill(' ') << "  (" << metrics.current_sim_time << " seconds)\n\n";
    
    // Flight Operations Section
    std::cout << BOLD << BLUE << "  ┌─────────────────────── Flight Operations ───────────────────────┐\n" << RESET;
    
    std::cout << "  │  " << BOLD << "Active Flights:  " << RESET 
              << get_status_color(metrics.active_flights, 3, 1)
              << std::setw(3) << metrics.active_flights << RESET;
    
    // Visual flight indicators
    std::cout << "   ";
    for (int i = 0; i < metrics.active_flights && i < 10; i++) {
        std::cout << "✈ ";
    }
    std::cout << "\n";
    
    std::cout << "  │  " << YELLOW << "🛬 Landing:     " << RESET << std::setw(3) << metrics.flights_landing
              << "    " << GREEN << "🚪 At Gates:    " << RESET << std::setw(3) << metrics.flights_at_gates
              << "    " << CYAN << "🛫 Departing:   " << RESET << std::setw(3) << metrics.flights_departing << "\n";
    
    std::cout << BOLD << BLUE << "  └────────────────────────────────────────────────────────────────┘\n\n" << RESET;
    
    // Resource Status Section
    std::cout << BOLD << MAGENTA << "  ┌─────────────────────── Resource Status ─────────────────────────┐\n" << RESET;
    
    double runway_pct = metrics.runway_utilization * 100;
    double gate_pct = metrics.gate_utilization * 100;
    
    std::cout << "  │  " << BOLD << "Runways:  " << RESET 
              << std::setw(2) << (4 - metrics.available_runways) << "/4 in use   "
              << create_progress_bar(runway_pct, 15) << " "
              << std::fixed << std::setprecision(1) << std::setw(5) << runway_pct << "%\n";
    
    std::cout << "  │  " << BOLD << "Gates:    " << RESET 
              << std::setw(2) << (8 - metrics.available_gates) << "/8 in use   "
              << create_progress_bar(gate_pct, 15) << " "
              << std::fixed << std::setprecision(1) << std::setw(5) << gate_pct << "%\n";
    
    std::cout << BOLD << MAGENTA << "  └────────────────────────────────────────────────────────────────┘\n\n" << RESET;
    
    // Performance Metrics Section
    std::cout << BOLD << GREEN << "  ┌─────────────────────── Performance Metrics ────────────────────┐\n" << RESET;
    
    double otp = metrics.on_time_performance * 100;
    const char* otp_color = (otp >= 80) ? GREEN : (otp >= 50) ? YELLOW : RED;
    
    std::cout << "  │  " << BOLD << "Total Flights Handled:  " << RESET 
              << CYAN << std::setw(4) << metrics.total_flights_handled << RESET << "\n";
    
    std::cout << "  │  " << BOLD << "Avg Turnaround Time:    " << RESET
              << std::fixed << std::setprecision(2) << std::setw(6) 
              << metrics.average_turnaround_time << " min\n";
    
    std::cout << "  │  " << BOLD << "On-Time Performance:    " << RESET
              << otp_color << std::fixed << std::setprecision(1) << std::setw(5) << otp << "%" << RESET
              << "  " << create_progress_bar(otp, 15) << "\n";
    
    std::cout << BOLD << GREEN << "  └────────────────────────────────────────────────────────────────┘\n\n" << RESET;
    
    // Memory/OS Statistics Section
    std::cout << BOLD << YELLOW << "  ┌─────────────────────── OS/Memory Statistics ───────────────────┐\n" << RESET;
    
    double fault_pct = metrics.page_fault_rate * 100;
    const char* fault_color = (fault_pct < 25) ? GREEN : (fault_pct < 50) ? YELLOW : RED;
    double hit_rate = 100.0 - fault_pct;
    
    std::cout << "  │  " << BOLD << "Page Faults:     " << RESET 
              << std::setw(4) << metrics.page_fault_count
              << "    " << BOLD << "TLB Hit Rate:  " << RESET
              << fault_color << std::fixed << std::setprecision(1) << std::setw(5) << hit_rate << "%" << RESET << "\n";
    
    std::cout << "  │  " << BOLD << "Page Fault Rate: " << RESET
              << create_progress_bar(fault_pct, 20) << " "
              << fault_color << std::fixed << std::setprecision(2) << std::setw(6) << fault_pct << "%" << RESET << "\n";
    
    // Thrashing indicator
    if (fault_pct > 25) {
        std::cout << "  │  " << BG_RED << WHITE << BOLD << " ⚠ THRASHING DETECTED " << RESET << "\n";
    }
    
    std::cout << BOLD << YELLOW << "  └────────────────────────────────────────────────────────────────┘\n" << RESET;
    
    pthread_rwlock_unlock(&dashboard_lock);
    
    std::cout << "\n" << DIM << "  Press Ctrl+C to stop simulation..." << RESET << std::endl;
}
