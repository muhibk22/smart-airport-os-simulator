#include "ResourceManager.h"
#include <algorithm>

using namespace std;

ResourceManager::ResourceManager() {
    num_resource_types = 7;
    num_flights = 0;
    available.resize(num_resource_types, 0);
    pthread_mutex_init(&manager_mutex, nullptr);
    pthread_cond_init(&resource_available, nullptr);
}

ResourceManager::~ResourceManager() {
    pthread_mutex_destroy(&manager_mutex);
    pthread_cond_destroy(&resource_available);
    
    // Clean up resources
    for (auto& pair : resource_pools) {
        for (Resource* r : pair.second) {
            delete r;
        }
    }
}

void ResourceManager::initialize(int fuel_trucks, int catering, int baggage,
                                  int cleaning, int buses, int tugs, int power_units) {
    pthread_mutex_lock(&manager_mutex);
    
    // Create fuel trucks
    for (int i = 0; i < fuel_trucks; i++) {
        resource_pools[RES_FUEL_TRUCK].push_back(
            new Resource(i, RES_FUEL_TRUCK, "FuelTruck-" + to_string(i)));
    }
    available[RES_FUEL_TRUCK] = fuel_trucks;
    
    // Create catering vehicles
    for (int i = 0; i < catering; i++) {
        resource_pools[RES_CATERING_VEHICLE].push_back(
            new Resource(i, RES_CATERING_VEHICLE, "Catering-" + to_string(i)));
    }
    available[RES_CATERING_VEHICLE] = catering;
    
    // Create baggage carts
    for (int i = 0; i < baggage; i++) {
        resource_pools[RES_BAGGAGE_CART].push_back(
            new Resource(i, RES_BAGGAGE_CART, "Baggage-" + to_string(i)));
    }
    available[RES_BAGGAGE_CART] = baggage;
    
    // Create cleaning crews
    for (int i = 0; i < cleaning; i++) {
        resource_pools[RES_CLEANING_CREW].push_back(
            new Resource(i, RES_CLEANING_CREW, "Cleaning-" + to_string(i)));
    }
    available[RES_CLEANING_CREW] = cleaning;
    
    // Create passenger buses
    for (int i = 0; i < buses; i++) {
        resource_pools[RES_PASSENGER_BUS].push_back(
            new Resource(i, RES_PASSENGER_BUS, "Bus-" + to_string(i)));
    }
    available[RES_PASSENGER_BUS] = buses;
    
    // Create aircraft tugs
    for (int i = 0; i < tugs; i++) {
        resource_pools[RES_AIRCRAFT_TUG].push_back(
            new Resource(i, RES_AIRCRAFT_TUG, "Tug-" + to_string(i)));
    }
    available[RES_AIRCRAFT_TUG] = tugs;
    
    // Create ground power units
    for (int i = 0; i < power_units; i++) {
        resource_pools[RES_GROUND_POWER_UNIT].push_back(
            new Resource(i, RES_GROUND_POWER_UNIT, "GPU-" + to_string(i)));
    }
    available[RES_GROUND_POWER_UNIT] = power_units;
    
    pthread_mutex_unlock(&manager_mutex);
}

int ResourceManager::register_flight(const vector<int>& max_demand) {
    pthread_mutex_lock(&manager_mutex);
    
    int flight_index = num_flights++;
    
    max_need.push_back(max_demand);
    allocation.push_back(vector<int>(num_resource_types, 0));
    need.push_back(max_demand);  // Initially need = max
    
    pthread_mutex_unlock(&manager_mutex);
    return flight_index;
}

void ResourceManager::unregister_flight(int flight_index) {
    pthread_mutex_lock(&manager_mutex);
    
    if (flight_index >= 0 && flight_index < (int)allocation.size()) {
        // Release all allocated resources
        for (int j = 0; j < num_resource_types; j++) {
            available[j] += allocation[flight_index][j];
            allocation[flight_index][j] = 0;
            need[flight_index][j] = 0;
            max_need[flight_index][j] = 0;
        }
    }
    
    pthread_cond_broadcast(&resource_available);
    pthread_mutex_unlock(&manager_mutex);
}

bool ResourceManager::is_safe_state() {
    // Banker's Algorithm safety check
    vector<int> work = available;
    vector<bool> finish(num_flights, false);
    
    int count = 0;
    while (count < num_flights) {
        bool found = false;
        
        for (int i = 0; i < num_flights; i++) {
            if (!finish[i]) {
                // Check if need[i] <= work
                bool can_finish = true;
                for (int j = 0; j < num_resource_types; j++) {
                    if (need[i][j] > work[j]) {
                        can_finish = false;
                        break;
                    }
                }
                
                if (can_finish) {
                    // Simulate completion
                    for (int j = 0; j < num_resource_types; j++) {
                        work[j] += allocation[i][j];
                    }
                    finish[i] = true;
                    found = true;
                    count++;
                }
            }
        }
        
        if (!found) {
            // No progress possible - unsafe state
            return false;
        }
    }
    
    return true;
}

bool ResourceManager::can_satisfy(int flight_index, const vector<int>& request) {
    // Check if request <= need and request <= available
    for (int j = 0; j < num_resource_types; j++) {
        if (request[j] > need[flight_index][j] || request[j] > available[j]) {
            return false;
        }
    }
    return true;
}

bool ResourceManager::request_resources(int flight_index, ResourceType type, int count) {
    pthread_mutex_lock(&manager_mutex);
    
    if (flight_index < 0 || flight_index >= (int)allocation.size()) {
        pthread_mutex_unlock(&manager_mutex);
        return false;
    }
    
    vector<int> request(num_resource_types, 0);
    request[type] = count;
    
    if (!can_satisfy(flight_index, request)) {
        pthread_mutex_unlock(&manager_mutex);
        return false;
    }
    
    // Temporarily allocate
    available[type] -= count;
    allocation[flight_index][type] += count;
    need[flight_index][type] -= count;
    
    // Check if safe
    if (!is_safe_state()) {
        // Rollback
        available[type] += count;
        allocation[flight_index][type] -= count;
        need[flight_index][type] += count;
        pthread_mutex_unlock(&manager_mutex);
        return false;
    }
    
    pthread_mutex_unlock(&manager_mutex);
    return true;
}

void ResourceManager::release_resources(int flight_index, ResourceType type, int count) {
    pthread_mutex_lock(&manager_mutex);
    
    if (flight_index >= 0 && flight_index < (int)allocation.size()) {
        int actual = min(count, allocation[flight_index][type]);
        allocation[flight_index][type] -= actual;
        available[type] += actual;
        need[flight_index][type] += actual;
        
        pthread_cond_broadcast(&resource_available);
    }
    
    pthread_mutex_unlock(&manager_mutex);
}

Resource* ResourceManager::allocate_resource(ResourceType type, int flight_id,
                                              long long current_time, long long duration) {
    pthread_mutex_lock(&manager_mutex);
    
    auto it = resource_pools.find(type);
    if (it == resource_pools.end()) {
        pthread_mutex_unlock(&manager_mutex);
        return nullptr;
    }
    
    for (Resource* r : it->second) {
        if (r->try_acquire(flight_id, current_time, duration)) {
            available[type]--;
            pthread_mutex_unlock(&manager_mutex);
            return r;
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
    return nullptr;
}

void ResourceManager::release_resource(Resource* resource) {
    if (resource == nullptr) return;
    
    pthread_mutex_lock(&manager_mutex);
    
    ResourceType type = resource->get_type();
    resource->release();
    available[type]++;
    
    pthread_cond_broadcast(&resource_available);
    pthread_mutex_unlock(&manager_mutex);
}

int ResourceManager::get_available_count(ResourceType type) {
    pthread_mutex_lock(&manager_mutex);
    int count = available[type];
    pthread_mutex_unlock(&manager_mutex);
    return count;
}

int ResourceManager::get_total_count(ResourceType type) {
    pthread_mutex_lock(&manager_mutex);
    auto it = resource_pools.find(type);
    int count = (it != resource_pools.end()) ? it->second.size() : 0;
    pthread_mutex_unlock(&manager_mutex);
    return count;
}

double ResourceManager::get_utilization(ResourceType type) {
    int total = get_total_count(type);
    if (total == 0) return 0.0;
    int avail = get_available_count(type);
    return 1.0 - ((double)avail / total);
}
