#include "GroundService.h"
#include <chrono>

using namespace std;

// Static random number generator for equipment failure simulation
mt19937 GroundService::rng;
bool GroundService::rng_initialized = false;

GroundService::GroundService(int id, ServiceType t, int flight, int gate) {
    service_id = id;
    type = t;
    name = type_to_string(t);
    status = SVC_PENDING;
    flight_id = flight;
    gate_id = gate;
    estimated_duration = get_default_duration(t);
    start_time = 0;
    end_time = 0;
    
    // Initialize equipment failure tracking
    equipment_failed = false;
    failure_recovery_time = 0;
    retry_count = 0;
    
    // Initialize RNG if not already done
    if (!rng_initialized) {
        rng.seed(chrono::steady_clock::now().time_since_epoch().count());
        rng_initialized = true;
    }
    
    pthread_mutex_init(&service_mutex, nullptr);
    pthread_cond_init(&service_complete, nullptr);
}

GroundService::~GroundService() {
    pthread_mutex_destroy(&service_mutex);
    pthread_cond_destroy(&service_complete);
}

bool GroundService::start(long long current_time) {
    pthread_mutex_lock(&service_mutex);
    
    if (status != SVC_PENDING && status != SVC_EQUIPMENT_FAILURE) {
        pthread_mutex_unlock(&service_mutex);
        return false;
    }
    
    // Check for equipment failure (5% probability as per spec)
    if (check_equipment_failure()) {
        status = SVC_EQUIPMENT_FAILURE;
        equipment_failed = true;
        retry_count++;
        
        // Recovery time varies by service type (5-15 minutes)
        uniform_int_distribution<int> recovery_dist(5, 15);
        failure_recovery_time = recovery_dist(rng);
        
        pthread_mutex_unlock(&service_mutex);
        return false;  // Service cannot start due to equipment failure
    }
    
    status = SVC_IN_PROGRESS;
    start_time = current_time;
    equipment_failed = false;
    
    pthread_mutex_unlock(&service_mutex);
    return true;
}

void GroundService::complete(long long current_time) {
    pthread_mutex_lock(&service_mutex);
    
    status = SVC_COMPLETED;
    end_time = current_time;
    
    pthread_cond_broadcast(&service_complete);
    pthread_mutex_unlock(&service_mutex);
}

void GroundService::block() {
    pthread_mutex_lock(&service_mutex);
    status = SVC_BLOCKED;
    pthread_mutex_unlock(&service_mutex);
}

void GroundService::fail(const string& reason) {
    pthread_mutex_lock(&service_mutex);
    status = SVC_FAILED;
    pthread_mutex_unlock(&service_mutex);
}

void GroundService::add_dependency(ServiceType dep) {
    dependencies.push_back(dep);
}

bool GroundService::dependencies_met(const vector<GroundService*>& completed) {
    for (ServiceType dep : dependencies) {
        bool found = false;
        for (GroundService* svc : completed) {
            if (svc->get_type() == dep && svc->get_status() == SVC_COMPLETED) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    return true;
}

void GroundService::wait_for_completion() {
    pthread_mutex_lock(&service_mutex);
    while (status != SVC_COMPLETED && status != SVC_FAILED) {
        pthread_cond_wait(&service_complete, &service_mutex);
    }
    pthread_mutex_unlock(&service_mutex);
}

void GroundService::signal_completion() {
    pthread_mutex_lock(&service_mutex);
    pthread_cond_broadcast(&service_complete);
    pthread_mutex_unlock(&service_mutex);
}

string GroundService::type_to_string(ServiceType type) {
    switch (type) {
        case SVC_DOCKING: return "Docking";
        case SVC_POWER_CONNECT: return "Power Connect";
        case SVC_UNLOADING_PASSENGERS: return "Passenger Unloading";
        case SVC_UNLOADING_BAGGAGE: return "Baggage Unloading";
        case SVC_REFUELING: return "Refueling";
        case SVC_CLEANING: return "Cleaning";
        case SVC_CATERING: return "Catering";
        case SVC_WATER_SERVICE: return "Water Service";
        case SVC_WASTE_SERVICE: return "Waste Service";
        case SVC_CARGO_UNLOAD: return "Cargo Unload";
        case SVC_CARGO_LOAD: return "Cargo Load";
        case SVC_LOADING_BAGGAGE: return "Baggage Loading";
        case SVC_BOARDING: return "Boarding";
        case SVC_POWER_DISCONNECT: return "Power Disconnect";
        case SVC_PUSHBACK: return "Pushback";
        default: return "Unknown";
    }
}

int GroundService::get_default_duration(ServiceType type) {
    switch (type) {
        case SVC_DOCKING: return 5;
        case SVC_POWER_CONNECT: return 2;
        case SVC_UNLOADING_PASSENGERS: return 15;
        case SVC_UNLOADING_BAGGAGE: return 20;
        case SVC_REFUELING: return 25;
        case SVC_CLEANING: return 30;
        case SVC_CATERING: return 20;
        case SVC_WATER_SERVICE: return 10;
        case SVC_WASTE_SERVICE: return 10;
        case SVC_CARGO_UNLOAD: return 30;
        case SVC_CARGO_LOAD: return 30;
        case SVC_LOADING_BAGGAGE: return 25;
        case SVC_BOARDING: return 20;
        case SVC_POWER_DISCONNECT: return 2;
        case SVC_PUSHBACK: return 5;
        default: return 10;
    }
}

// ============================================================================
// EQUIPMENT FAILURE SIMULATION
// Implements 5% random equipment failure probability as per spec
// ============================================================================

bool GroundService::check_equipment_failure() {
    // Generate random number between 0 and 1
    uniform_real_distribution<double> dist(0.0, 1.0);
    double roll = dist(rng);
    
    // 5% chance of equipment failure
    return roll < EQUIPMENT_FAILURE_PROBABILITY;
}

bool GroundService::attempt_recovery() {
    pthread_mutex_lock(&service_mutex);
    
    if (status != SVC_EQUIPMENT_FAILURE) {
        pthread_mutex_unlock(&service_mutex);
        return true;  // Not in failure state
    }
    
    if (retry_count >= MAX_RETRIES) {
        // Max retries exceeded, permanent failure
        status = SVC_FAILED;
        pthread_mutex_unlock(&service_mutex);
        return false;
    }
    
    // Reset to pending for retry
    status = SVC_PENDING;
    equipment_failed = false;
    
    pthread_mutex_unlock(&service_mutex);
    return true;
}
