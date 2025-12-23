#include "ServiceExecutor.h"
#include "../core/Logger.h"
#include <sstream>
#include <unistd.h>

using namespace std;

ServiceExecutor::ServiceExecutor(ServiceDependencyGraph* graph, ResourceManager* resources) {
    dep_graph = graph;
    resource_manager = resources;
    is_running = false;
    pthread_mutex_init(&executor_mutex, nullptr);
    pthread_cond_init(&service_ready, nullptr);
}

ServiceExecutor::~ServiceExecutor() {
    reset();
    pthread_mutex_destroy(&executor_mutex);
    pthread_cond_destroy(&service_ready);
}

void ServiceExecutor::create_turnaround_services(int flight_id, int gate_id) {
    pthread_mutex_lock(&executor_mutex);
    
    int id = 0;
    vector<ServiceType> order = dep_graph->get_execution_order();
    
    for (ServiceType type : order) {
        GroundService* service = new GroundService(id++, type, flight_id, gate_id);
        
        // Add dependencies from graph
        vector<ServiceType> deps = dep_graph->get_dependencies(type);
        for (ServiceType dep : deps) {
            service->add_dependency(dep);
        }
        
        all_services.push_back(service);
    }
    
    pthread_mutex_unlock(&executor_mutex);
}

void ServiceExecutor::update_ready_queue() {
    // Find services with all dependencies met
    for (GroundService* svc : all_services) {
        if (svc->get_status() == SVC_PENDING) {
            if (svc->dependencies_met(completed_services)) {
                ready_queue.push(svc);
            }
        }
    }
}

GroundService* ServiceExecutor::get_next_ready() {
    pthread_mutex_lock(&executor_mutex);
    
    update_ready_queue();
    
    GroundService* next = nullptr;
    if (!ready_queue.empty()) {
        next = ready_queue.front();
        ready_queue.pop();
    }
    
    pthread_mutex_unlock(&executor_mutex);
    return next;
}

bool ServiceExecutor::execute_service(GroundService* service, long long current_time) {
    if (service == nullptr) return false;
    
    Logger* logger = Logger::get_instance();
    
    // Start service
    if (!service->start(current_time)) {
        return false;
    }
    
    ostringstream msg;
    msg << "[SERVICE] Started " << service->get_name() 
        << " for flight " << service->get_flight_id();
    logger->log_event(msg.str());
    
    // Simulate work (scaled down)
    int duration = service->get_duration();
    usleep(duration * 10000);  // 1 min = 10ms
    
    // Complete service
    service->complete(current_time + duration);
    
    msg.str("");
    msg << "[SERVICE] Completed " << service->get_name()
        << " for flight " << service->get_flight_id();
    logger->log_event(msg.str());
    
    // Mark as completed
    mark_completed(service);
    
    return true;
}

void ServiceExecutor::mark_completed(GroundService* service) {
    pthread_mutex_lock(&executor_mutex);
    
    completed_services.push_back(service);
    pthread_cond_broadcast(&service_ready);
    
    pthread_mutex_unlock(&executor_mutex);
}

void ServiceExecutor::execute_all(long long start_time) {
    is_running = true;
    long long current = start_time;
    
    while ((int)completed_services.size() < (int)all_services.size()) {
        GroundService* next = get_next_ready();
        
        if (next != nullptr) {
            execute_service(next, current);
            current += next->get_duration();
        } else {
            // Wait for a service to complete
            usleep(10000);
        }
    }
    
    is_running = false;
}

void ServiceExecutor::reset() {
    pthread_mutex_lock(&executor_mutex);
    
    for (GroundService* svc : all_services) {
        delete svc;
    }
    all_services.clear();
    completed_services.clear();
    
    while (!ready_queue.empty()) {
        ready_queue.pop();
    }
    
    pthread_mutex_unlock(&executor_mutex);
}
