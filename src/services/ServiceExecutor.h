#ifndef SERVICE_EXECUTOR_H
#define SERVICE_EXECUTOR_H

#include "GroundService.h"
#include "ServiceDependencyGraph.h"
#include "../resources/ResourceManager.h"
#include <vector>
#include <queue>
#include <pthread.h>

using namespace std;

// ServiceExecutor runs services in topological order using pthread_cond

class ServiceExecutor {
private:
    ServiceDependencyGraph* dep_graph;
    ResourceManager* resource_manager;
    
    vector<GroundService*> all_services;
    vector<GroundService*> completed_services;
    queue<GroundService*> ready_queue;
    
    pthread_mutex_t executor_mutex;
    pthread_cond_t service_ready;
    
    bool is_running;
    
public:
    ServiceExecutor(ServiceDependencyGraph* graph, ResourceManager* resources);
    ~ServiceExecutor();
    
    // Create services for a flight turnaround
    void create_turnaround_services(int flight_id, int gate_id);
    
    // Execute all services (blocking)
    void execute_all(long long start_time);
    
    // Execute single service (called by worker thread)
    bool execute_service(GroundService* service, long long current_time);
    
    // Check which services are ready to run
    void update_ready_queue();
    
    // Get next ready service
    GroundService* get_next_ready();
    
    // Mark service complete and update ready queue
    void mark_completed(GroundService* service);
    
    // Statistics
    int get_completed_count() const { return completed_services.size(); }
    int get_total_count() const { return all_services.size(); }
    
    // Cleanup
    void reset();
};

#endif // SERVICE_EXECUTOR_H
