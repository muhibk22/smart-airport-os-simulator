#ifndef SERVICE_DEPENDENCY_GRAPH_H
#define SERVICE_DEPENDENCY_GRAPH_H

#include "GroundService.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;

// ServiceDependencyGraph implements DAG for service order
// Supports topological sort for execution order

class ServiceDependencyGraph {
private:
    unordered_map<ServiceType, vector<ServiceType>> adjacency;  // service -> dependencies
    
public:
    ServiceDependencyGraph();
    ~ServiceDependencyGraph();
    
    // Build default turnaround service DAG
    void build_standard_turnaround();
    
    // Add edge: 'before' must complete before 'after'
    void add_dependency(ServiceType after, ServiceType before);
    
    // Get dependencies for a service
    vector<ServiceType> get_dependencies(ServiceType service);
    
    // Topological sort - returns execution order
    vector<ServiceType> get_execution_order();
    
    // Check if graph has cycles (would be error)
    bool has_cycle();
    
private:
    bool dfs_cycle(ServiceType node, unordered_set<ServiceType>& visited,
                   unordered_set<ServiceType>& rec_stack);
    void dfs_topo(ServiceType node, unordered_set<ServiceType>& visited,
                  vector<ServiceType>& result);
};

#endif // SERVICE_DEPENDENCY_GRAPH_H
