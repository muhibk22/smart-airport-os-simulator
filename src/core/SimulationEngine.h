#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include "TimeManager.h"
#include "EventQueue.h"
#include "Logger.h"
#include "Dashboard.h"
#include "../airport/RunwayManager.h"
#include "../airport/GateManager.h"
#include "../airport/TaxiwayGraph.h"
#include <pthread.h>
#include <atomic>

class SimulationEngine {
private:
    TimeManager* time_manager;
    EventQueue* event_queue;
    Logger* logger;
    Dashboard* dashboard;
    
    RunwayManager* runway_manager;
    GateManager* gate_manager;
    TaxiwayGraph* taxiway_graph;
    
    // Control threads
    pthread_t event_dispatcher_thread;
    pthread_t dashboard_updater_thread;
    pthread_t crisis_monitor_thread;
    
    std::atomic<bool> simulation_running;
    long long simulation_duration;
    
    // Thread functions
    static void* event_dispatcher_func(void* arg);
    static void* dashboard_updater_func(void* arg);
    static void* crisis_monitor_func(void* arg);
    
    // Initialization
    void load_configuration();
    void initialize_airport();
    void generate_initial_flights();
    
public:
    SimulationEngine();
    ~SimulationEngine();
    
    void initialize();
    void run();
    void stop();
    
    // Getters
    TimeManager* get_time_manager() { return time_manager; }
    EventQueue* get_event_queue() { return event_queue; }
    RunwayManager* get_runway_manager() { return runway_manager; }
    GateManager* get_gate_manager() { return gate_manager; }
    TaxiwayGraph* get_taxiway_graph() { return taxiway_graph; }
};

#endif // SIMULATION_ENGINE_H
