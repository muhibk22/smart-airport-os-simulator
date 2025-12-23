#ifndef GROUND_SERVICE_H
#define GROUND_SERVICE_H

#include <string>
#include <vector>
#include <pthread.h>
#include <random>

using namespace std;

// Ground service types (15+ from README)
enum ServiceType {
    SVC_DOCKING,
    SVC_POWER_CONNECT,
    SVC_UNLOADING_PASSENGERS,
    SVC_UNLOADING_BAGGAGE,
    SVC_REFUELING,
    SVC_CLEANING,
    SVC_CATERING,
    SVC_WATER_SERVICE,
    SVC_WASTE_SERVICE,
    SVC_CARGO_UNLOAD,
    SVC_CARGO_LOAD,
    SVC_LOADING_BAGGAGE,
    SVC_BOARDING,
    SVC_POWER_DISCONNECT,
    SVC_PUSHBACK
};

enum ServiceStatus {
    SVC_PENDING,
    SVC_IN_PROGRESS,
    SVC_COMPLETED,
    SVC_BLOCKED,
    SVC_FAILED,
    SVC_EQUIPMENT_FAILURE  // New status for equipment failures
};

class GroundService {
private:
    int service_id;
    ServiceType type;
    string name;
    ServiceStatus status;
    
    int flight_id;
    int gate_id;
    
    vector<ServiceType> dependencies;  // Must complete before this
    int estimated_duration;            // Minutes
    long long start_time;
    long long end_time;
    
    // Equipment failure simulation (5% probability as per spec)
    static constexpr double EQUIPMENT_FAILURE_PROBABILITY = 0.05;  // 5%
    bool equipment_failed;
    int failure_recovery_time;  // Minutes to recover from failure
    int retry_count;
    static constexpr int MAX_RETRIES = 3;
    
    // Random number generator for failure simulation
    static mt19937 rng;
    static bool rng_initialized;
    
    pthread_mutex_t service_mutex;
    pthread_cond_t service_complete;
    
public:
    GroundService(int id, ServiceType t, int flight, int gate);
    ~GroundService();
    
    // Lifecycle
    bool start(long long current_time);
    void complete(long long current_time);
    void block();
    void fail(const string& reason);
    
    // Equipment failure simulation
    bool check_equipment_failure();  // Returns true if equipment failed
    bool attempt_recovery();         // Try to recover from failure
    int get_recovery_time() const { return failure_recovery_time; }
    bool has_equipment_failed() const { return equipment_failed; }
    int get_retry_count() const { return retry_count; }
    
    // Dependencies
    void add_dependency(ServiceType dep);
    bool dependencies_met(const vector<GroundService*>& completed);
    
    // Wait for completion
    void wait_for_completion();
    void signal_completion();
    
    // Getters
    int get_id() const { return service_id; }
    ServiceType get_type() const { return type; }
    ServiceStatus get_status() const { return status; }
    string get_name() const { return name; }
    int get_flight_id() const { return flight_id; }
    int get_duration() const { return estimated_duration; }
    const vector<ServiceType>& get_dependencies() const { return dependencies; }
    
    static string type_to_string(ServiceType type);
    static int get_default_duration(ServiceType type);
};

#endif // GROUND_SERVICE_H
