#include "ServiceDependencyGraph.h"
#include <algorithm>

using namespace std;

ServiceDependencyGraph::ServiceDependencyGraph() {
    // Initialize adjacency for all service types
    for (int i = SVC_DOCKING; i <= SVC_PUSHBACK; i++) {
        adjacency[(ServiceType)i] = vector<ServiceType>();
    }
}

ServiceDependencyGraph::~ServiceDependencyGraph() {
}

void ServiceDependencyGraph::build_standard_turnaround() {
    // Standard turnaround DAG from README
    // Docking first
    add_dependency(SVC_POWER_CONNECT, SVC_DOCKING);
    add_dependency(SVC_UNLOADING_PASSENGERS, SVC_DOCKING);
    add_dependency(SVC_UNLOADING_BAGGAGE, SVC_DOCKING);
    
    // After power connect
    add_dependency(SVC_REFUELING, SVC_POWER_CONNECT);
    
    // After passenger/baggage unload
    add_dependency(SVC_CLEANING, SVC_UNLOADING_PASSENGERS);
    add_dependency(SVC_CATERING, SVC_CLEANING);
    add_dependency(SVC_WATER_SERVICE, SVC_CLEANING);
    add_dependency(SVC_WASTE_SERVICE, SVC_CLEANING);
    
    // Cargo
    add_dependency(SVC_CARGO_UNLOAD, SVC_UNLOADING_BAGGAGE);
    add_dependency(SVC_CARGO_LOAD, SVC_CARGO_UNLOAD);
    add_dependency(SVC_LOADING_BAGGAGE, SVC_CARGO_LOAD);
    
    // Boarding after services complete
    add_dependency(SVC_BOARDING, SVC_CATERING);
    add_dependency(SVC_BOARDING, SVC_LOADING_BAGGAGE);
    add_dependency(SVC_BOARDING, SVC_REFUELING);
    
    // Pushback last
    add_dependency(SVC_POWER_DISCONNECT, SVC_BOARDING);
    add_dependency(SVC_PUSHBACK, SVC_POWER_DISCONNECT);
}

void ServiceDependencyGraph::add_dependency(ServiceType after, ServiceType before) {
    adjacency[after].push_back(before);
}

vector<ServiceType> ServiceDependencyGraph::get_dependencies(ServiceType service) {
    auto it = adjacency.find(service);
    if (it != adjacency.end()) {
        return it->second;
    }
    return vector<ServiceType>();
}

void ServiceDependencyGraph::dfs_topo(ServiceType node, unordered_set<ServiceType>& visited,
                                       vector<ServiceType>& result) {
    visited.insert(node);
    
    for (ServiceType dep : adjacency[node]) {
        if (!visited.count(dep)) {
            dfs_topo(dep, visited, result);
        }
    }
    
    result.push_back(node);
}

vector<ServiceType> ServiceDependencyGraph::get_execution_order() {
    vector<ServiceType> result;
    unordered_set<ServiceType> visited;
    
    // Run DFS from all nodes
    for (int i = SVC_DOCKING; i <= SVC_PUSHBACK; i++) {
        ServiceType svc = (ServiceType)i;
        if (!visited.count(svc)) {
            dfs_topo(svc, visited, result);
        }
    }
    
    // Result is already in correct order (dependencies first)
    return result;
}

bool ServiceDependencyGraph::dfs_cycle(ServiceType node, unordered_set<ServiceType>& visited,
                                        unordered_set<ServiceType>& rec_stack) {
    visited.insert(node);
    rec_stack.insert(node);
    
    for (ServiceType dep : adjacency[node]) {
        if (rec_stack.count(dep)) {
            return true;
        }
        if (!visited.count(dep)) {
            if (dfs_cycle(dep, visited, rec_stack)) {
                return true;
            }
        }
    }
    
    rec_stack.erase(node);
    return false;
}

bool ServiceDependencyGraph::has_cycle() {
    unordered_set<ServiceType> visited;
    unordered_set<ServiceType> rec_stack;
    
    for (int i = SVC_DOCKING; i <= SVC_PUSHBACK; i++) {
        ServiceType svc = (ServiceType)i;
        if (!visited.count(svc)) {
            if (dfs_cycle(svc, visited, rec_stack)) {
                return true;
            }
        }
    }
    return false;
}
