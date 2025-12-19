#include "SimulationEngine.h"
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <iostream>

SimulationEngine::SimulationEngine() {
    time_manager = new TimeManager();
    event_queue = new EventQueue();
    logger = Logger::get_instance();
    dashboard = new Dashboard();
    
    runway_manager = new RunwayManager();
    gate_manager = new GateManager();
    taxiway_graph = new TaxiwayGraph();
    
    simulation_running = false;
    simulation_duration = 86400; // 24 hours default
}

SimulationEngine::~SimulationEngine() {
    if (simulation_running) {
        stop();
    }
    
    delete time_manager;
    delete event_queue;
    delete dashboard;
    delete runway_manager;
    delete gate_manager;
    delete taxiway_graph;
}

void SimulationEngine::load_configuration() {
    // In full implementation, would parse JSON config files
    // For now, use defaults
    
    // Set random seed for deterministic simulation
    srand(42);
    
    logger->log_event("[SimulationEngine] Configuration loaded - seed: 42");
}

void SimulationEngine::initialize_airport() {
    // Create runways
    runway_manager->add_runway(new Runway(0, "27L"));
    runway_manager->add_runway(new Runway(1, "27R"));
    runway_manager->add_runway(new Runway(2, "09L"));
    runway_manager->add_runway(new Runway(3, "09R"));
    
    logger->log_event("[SimulationEngine] Created 4 runways");
    
    // Create gates
    gate_manager->add_gate(new Gate(0, GATE_INTERNATIONAL, GATE_LARGE, true));
    gate_manager->add_gate(new Gate(1, GATE_INTERNATIONAL, GATE_LARGE, true));
    gate_manager->add_gate(new Gate(2, GATE_INTERNATIONAL, GATE_HEAVY, true));
    gate_manager->add_gate(new Gate(3, GATE_INTERNATIONAL, GATE_HEAVY, true));
    gate_manager->add_gate(new Gate(4, GATE_INTERNATIONAL, GATE_HEAVY, true));
    
    for (int i = 5; i < 10; i++) {
        gate_manager->add_gate(new Gate(i, GATE_INTERNATIONAL, GATE_MEDIUM, true));
    }
    
    for (int i = 10; i < 15; i++) {
        gate_manager->add_gate(new Gate(i, GATE_DOMESTIC, GATE_MEDIUM, true));
    }
    
    for (int i = 15; i < 20; i++) {
        gate_manager->add_gate(new Gate(i, GATE_DOMESTIC, GATE_SMALL, false));
    }
    
    logger->log_event("[SimulationEngine] Created 20 gates");
    
    // Create taxiway graph (simplified)
    for (int i = 0; i < 10; i++) {
        taxiway_graph->add_node(i, "Taxiway_" + std::to_string(i));
    }
    
    // Add edges (simplified network)
    for (int i = 0; i < 9; i++) {
        taxiway_graph->add_edge(i, i + 1, 60); // 60 seconds per segment
    }
    
    logger->log_event("[SimulationEngine] Created taxiway graph");
}

void SimulationEngine::generate_initial_flights() {
    // In full implementation, would generate realistic flight schedule
    // For now, create a few test flights
    
    logger->log_event("[SimulationEngine] Flight generation placeholder");
    
    // Flights will be created by scheduler/main loop in full implementation
}

void* SimulationEngine::event_dispatcher_func(void* arg) {
    SimulationEngine* engine = static_cast<SimulationEngine*>(arg);
    
    while (engine->simulation_running) {
        Event* event = engine->event_queue->peek();
        
        if (event == nullptr) {
            usleep(100000); // 100ms
            continue;
        }
        
        long long current_time = engine->time_manager->get_current_time();
        
        if (event->get_time() <= current_time) {
            // Process event
            event = engine->event_queue->pop();
            
            if (event) {
                event->process();
                
                engine->logger->log_event("[EventDispatcher] Processed: " + 
                                         event->get_description());
                
                delete event;
            }
        } else {
            // Event in future, wait
            usleep(10000); // 10ms
        }
    }
    
    return nullptr;
}

void* SimulationEngine::dashboard_updater_func(void* arg) {
    SimulationEngine* engine = static_cast<SimulationEngine*>(arg);
    
    while (engine->simulation_running) {
        // Update dashboard every 1 second
        sleep(1);
        
        // Collect metrics
        DashboardMetrics metrics;
        metrics.current_sim_time = engine->time_manager->get_current_time();
        metrics.active_flights = 0; // Would count from flight list
        metrics.available_runways = engine->runway_manager->get_available_runway_count();
        metrics.available_gates = engine->gate_manager->get_available_gate_count();
        metrics.runway_utilization = 0.0; // Would calculate
        metrics.gate_utilization = 0.0; // Would calculate
        metrics.total_flights_handled = 0;
        metrics.average_turnaround_time = 0.0;
        metrics.on_time_performance = 0.0;
        metrics.page_fault_count = 0;
        metrics.page_fault_rate = 0.0;
        
        engine->dashboard->update_metrics(metrics);
        engine->dashboard->display();
    }
    
    return nullptr;
}

void* SimulationEngine::crisis_monitor_func(void* arg) {
    SimulationEngine* engine = static_cast<SimulationEngine*>(arg);
    
    while (engine->simulation_running) {
        // Monitor for crisis conditions
        sleep(5); // Check every 5 seconds
        
        // Would implement weather monitoring, gridlock detection etc.
    }
    
    return nullptr;
}

void SimulationEngine::initialize() {
    logger->log_event("[SimulationEngine] Initializing simulation...");
    
    load_configuration();
    initialize_airport();
    generate_initial_flights();
    
    logger->log_event("[SimulationEngine] Initialization complete");
}

void SimulationEngine::run() {
    simulation_running = true;
    
    logger->log_event("[SimulationEngine] Starting simulation threads...");
    
    // Create control threads
    pthread_create(&event_dispatcher_thread, nullptr, event_dispatcher_func, this);
    pthread_create(&dashboard_updater_thread, nullptr, dashboard_updater_func, this);
    pthread_create(&crisis_monitor_thread, nullptr, crisis_monitor_func, this);
    
    logger->log_event("[SimulationEngine] All threads started");
    
    // Main simulation loop
    long long start_time = time_manager->get_current_time();
    
    while (simulation_running) {
        long long current_time = time_manager->get_current_time();
        
        if (current_time - start_time >= simulation_duration) {
            logger->log_event("[SimulationEngine] Simulation duration reached");
            break;
        }
        
        // Advance time
        time_manager->advance_time(1);
        
        sleep(0); // Yield to other threads
    }
    
    stop();
}

void SimulationEngine::stop() {
    logger->log_event("[SimulationEngine] Stopping simulation...");
    
    simulation_running = false;
    
    // Wait for threads to finish
    pthread_join(event_dispatcher_thread, nullptr);
    pthread_join(dashboard_updater_thread, nullptr);
    pthread_join(crisis_monitor_thread, nullptr);
    
    logger->log_event("[SimulationEngine] All threads stopped");
    logger->flush_all();
}
