#include "SimulationEngine.h"
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace std;

SimulationEngine::SimulationEngine() {
    time_manager = new TimeManager();
    event_queue = new EventQueue();
    logger = Logger::get_instance();
    dashboard = new Dashboard();
    
    runway_manager = new RunwayManager();
    gate_manager = new GateManager();
    taxiway_graph = new TaxiwayGraph();
    scheduler = new HMFQQueue();  // HMFQ-PPRA Scheduler
    
    // Initialize memory manager components
    tlb = new TLB(64);  // 64-entry TLB
    working_set_manager = new WorkingSetManager();
    clock_replacer = new ClockReplacer(256);  // 256 frames
    thrashing_detector = new ThrashingDetector();
    
    // Initialize resource manager with Banker's algorithm
    resource_manager = new ResourceManager();
    // Resource pool sizes: 8 fuel trucks, 6 catering, 10 baggage, 4 cleaning, 5 buses, 6 tugs, 8 GPUs
    resource_manager->initialize(8, 6, 10, 4, 5, 6, 8);
    
    simulation_running = false;
    simulation_duration = 86400; // 24 hours default
    
    // Initialize flight counters
    active_flight_count = 0;
    flights_landing = 0;
    flights_at_gates = 0;
    flights_departing = 0;
    total_flights_handled = 0;
    
    // Initialize performance counters
    total_turnaround_time = 0;
    on_time_flights = 0;
    delayed_flights = 0;
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
    delete scheduler;
    delete tlb;
    delete working_set_manager;
    delete clock_replacer;
    delete thrashing_detector;
    delete resource_manager;
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
    
    engine->logger->log_event("[EventDispatcher] Started");
    
    while (engine->simulation_running) {
        Event* event = engine->event_queue->peek();
        
        if (event == nullptr) {
            usleep(100000); // 100ms
            continue;
        }
        
        long long current_time = engine->time_manager->get_current_time();
        long long event_time = event->get_time();
        
        engine->logger->log_event("[EventDispatcher] Checking event at time " + 
                                  std::to_string(event_time) + 
                                  ", current time: " + std::to_string(current_time));
        
        if (event_time <= current_time) {
            // Process event
            event = engine->event_queue->pop();
            
            if (event) {
                engine->logger->log_event("[EventDispatcher] Processing: " + 
                                         event->get_description());
                
                event->process();
                
                engine->logger->log_event("[EventDispatcher] Completed: " + 
                                         event->get_description());
                
                delete event;
            }
        } else {
            // Event in future, wait
            usleep(50000); // 50ms
        }
    }
    
    engine->logger->log_event("[EventDispatcher] Stopped");
    return nullptr;
}

void* SimulationEngine::dashboard_updater_func(void* arg) {
    SimulationEngine* engine = static_cast<SimulationEngine*>(arg);
    
    while (engine->simulation_running) {
        // Collect metrics from atomic counters
        DashboardMetrics metrics = {};
        metrics.current_sim_time = engine->time_manager->get_current_time();
        metrics.active_flights = engine->active_flight_count.load();
        metrics.flights_at_gates = engine->flights_at_gates.load();
        metrics.flights_landing = engine->flights_landing.load();
        metrics.flights_departing = engine->flights_departing.load();
        metrics.available_runways = engine->runway_manager->get_available_runway_count();
        metrics.available_gates = engine->gate_manager->get_available_gate_count();
        
        // Calculate utilization (0.0 to 1.0 - dashboard multiplies by 100)
        metrics.runway_utilization = (4.0 - metrics.available_runways) / 4.0;
        metrics.gate_utilization = (20.0 - metrics.available_gates) / 20.0;
        
        metrics.total_flights_handled = engine->total_flights_handled.load();
        
        // Performance metrics (0.0 to 1.0 - dashboard multiplies by 100)
        metrics.average_turnaround_time = engine->get_avg_turnaround() / 60.0;  // Convert to minutes
        metrics.on_time_performance = engine->get_on_time_rate() / 100.0;  // Convert to 0-1 range
        
        // Memory metrics from TLB and thrashing detector (0.0 to 1.0)
        metrics.page_fault_count = engine->tlb->get_misses();
        double total_accesses = engine->tlb->get_hits() + engine->tlb->get_misses();
        metrics.page_fault_rate = total_accesses > 0 ? 
            ((double)engine->tlb->get_misses() / total_accesses) : 0.0;
        
        // Log memory stats periodically
        if (metrics.current_sim_time % 10 == 0) {  // Every 10 time units
            ostringstream mem_msg;
            mem_msg << "[MEMORY] TLB Hit Rate: " << (engine->tlb->get_hit_rate() * 100.0) << "%"
                    << " | Thrashing: " << (engine->thrashing_detector->is_in_thrashing_state() ? "YES" : "NO");
            engine->logger->log_memory(mem_msg.str());
        }
        
        // Log performance stats
        if (metrics.current_sim_time % 30 == 0 && metrics.total_flights_handled > 0) {
            ostringstream perf_msg;
            perf_msg << "[PERF] Flights: " << metrics.total_flights_handled 
                     << " | Avg Turnaround: " << metrics.average_turnaround_time << " min"
                     << " | On-Time: " << metrics.on_time_performance << "%";
            engine->logger->log_performance(perf_msg.str());
        }
        
        engine->dashboard->update_metrics(metrics);
        engine->dashboard->display();
        
        // Update every 500ms for more responsive display
        usleep(500000);
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
    
    // Main simulation loop - advance time with real delays
    // 1 simulation time unit = 100ms real time (for testing)
    while (simulation_running) {
        // Advance simulation time
        time_manager->advance_time(1);
        
        // Wait 100ms between time advances (10 sim-seconds per real second)
        usleep(100000);  // 100ms
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
