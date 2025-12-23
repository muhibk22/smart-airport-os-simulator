#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Resource.h"
#include <vector>
#include <unordered_map>
#include <pthread.h>

using namespace std;

// ResourceManager implements Banker's Algorithm for deadlock-free allocation
// Manages 7 resource types from README

class ResourceManager {
private:
    // Resource pools by type
    unordered_map<ResourceType, vector<Resource*>> resource_pools;
    
    // Banker's Algorithm matrices
    vector<int> available;          // Available[j] = # of resource type j available
    vector<vector<int>> max_need;   // Max[i][j] = max demand of flight i for type j
    vector<vector<int>> allocation; // Allocation[i][j] = currently allocated
    vector<vector<int>> need;       // Need[i][j] = Max[i][j] - Allocation[i][j]
    
    int num_resource_types;
    int num_flights;
    
    pthread_mutex_t manager_mutex;
    pthread_cond_t resource_available;
    
    // Banker's algorithm helpers
    bool is_safe_state();
    bool can_satisfy(int flight_index, const vector<int>& request);
    
public:
    ResourceManager();
    ~ResourceManager();
    
    // Initialize resource pools from config
    void initialize(int fuel_trucks, int catering, int baggage, 
                   int cleaning, int buses, int tugs, int power_units);
    
    // Register flight for Banker's algorithm
    int register_flight(const vector<int>& max_demand);
    void unregister_flight(int flight_index);
    
    // Request resources (Banker's algorithm)
    bool request_resources(int flight_index, ResourceType type, int count);
    
    // Release resources
    void release_resources(int flight_index, ResourceType type, int count);
    
    // Simple allocation (without Banker's for single resources)
    Resource* allocate_resource(ResourceType type, int flight_id, 
                                 long long current_time, long long duration);
    void release_resource(Resource* resource);
    
    // Statistics
    int get_available_count(ResourceType type);
    int get_total_count(ResourceType type);
    double get_utilization(ResourceType type);
};

#endif // RESOURCE_MANAGER_H
