#include "SimulationEngine.h"
#include <unistd.h>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>

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
    
    // Initialize crisis manager for weather and emergencies
    crisis_manager = new CrisisManager();
    
    // Initialize prefetcher for predictive memory access
    prefetcher = new Prefetcher();
    
    // Initialize crew manager with LRU assignment
    crew_manager = new CrewManager();
    // Crew pool: 20 pilots, 20 co-pilots, 60 attendants, 15 technicians, 30 handlers, 10 agents, 10 fuel techs
    crew_manager->initialize(20, 20, 60, 15, 30, 10, 10);
    
    // Initialize finance tracking
    cost_model = new CostModel();
    revenue_model = new RevenueModel();
    
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
    next_flight_id = 100;  // Start flight IDs at 100
    available_atc = TOTAL_ATC;  // REQ-5: Initialize ATC count
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
    delete crisis_manager;
    delete prefetcher;
    delete crew_manager;
    delete cost_model;
    delete revenue_model;
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
    logger->log_event("[SimulationEngine] Generating initial flights...");
    
    // Create some initial aircraft
    vector<Aircraft*> aircraft = {
        new Aircraft(B777),      // Heavy international
        new Aircraft(A320),      // Medium domestic
        new Aircraft(B737),      // Medium domestic
        new Aircraft(G650),      // Private jet
        new Aircraft(A380)       // Heavy international
    };
    
    // Create initial flights
    vector<Flight*> flights = {
        new Flight("AA100", aircraft[0], INTERNATIONAL, 300, 1800),  // Arrive at 5 min, depart at 30 min
        new Flight("UA200", aircraft[1], DOMESTIC, 600, 2400),       // Arrive at 10 min, depart at 40 min
        new Flight("DL300", aircraft[2], DOMESTIC, 900, 3000),       // Arrive at 15 min, depart at 50 min
        new Flight("PVT500", aircraft[3], DOMESTIC, 1200, 3600),     // Arrive at 20 min, depart at 60 min
        new Flight("BA400", aircraft[4], INTERNATIONAL, 1500, 4200)  // Arrive at 25 min, depart at 70 min
    };
    
    // Schedule arrival events
    for (Flight* flight : flights) {
        FlightArrivalEvent* arrival_event = new FlightArrivalEvent(flight, this, flight->scheduled_arrival_time);
        event_queue->push(arrival_event);
        
        ostringstream log_msg;
        log_msg << "[SimulationEngine] Scheduled flight " << flight->flight_id 
                << " arrival at " << flight->scheduled_arrival_time << " seconds";
        logger->log_event(log_msg.str());
    }
    
    logger->log_event("[SimulationEngine] Initial flights generated");
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
            // Event in future, wait quietly
            usleep(50000); // 50ms
        }
    }
    
    engine->logger->log_event("[EventDispatcher] Stopped");
    return nullptr;
}

void* SimulationEngine::dashboard_updater_func(void* arg) {
    SimulationEngine* engine = static_cast<SimulationEngine*>(arg);
    
    // Track last logged times to prevent duplicates
    long long last_memory_log_time = -1;
    long long last_perf_log_time = -1;
    
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
        
        // Log memory stats every 10 seconds (only once per interval)
        long long memory_interval = metrics.current_sim_time / 10;
        if (memory_interval > last_memory_log_time) {
            last_memory_log_time = memory_interval;
            ostringstream mem_msg;
            mem_msg << "[MEMORY] TLB Hit Rate: " << (engine->tlb->get_hit_rate() * 100.0) << "%"
                    << " | Thrashing: " << (engine->thrashing_detector->is_in_thrashing_state() ? "YES" : "NO");
            engine->logger->log_memory(mem_msg.str());
        }
        
        // Log performance stats every 30 seconds (only once per interval)
        long long perf_interval = metrics.current_sim_time / 30;
        if (perf_interval > last_perf_log_time && metrics.total_flights_handled > 0) {
            last_perf_log_time = perf_interval;
            ostringstream perf_msg;
            perf_msg << "[PERF] Flights: " << metrics.total_flights_handled 
                     << " | Avg Turnaround: " << fixed << setprecision(2) << metrics.average_turnaround_time << " min"
                     << " | On-Time: " << fixed << setprecision(1) << (metrics.on_time_performance * 100) << "%";
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
    Logger* logger = Logger::get_instance();
    CrisisManager* crisis_mgr = engine->get_crisis_manager();
    
    int weather_event_id = 0;
    int emergency_event_id = 0;
    int check_cycle = 0;
    
    logger->log_event("[CrisisMonitor] Crisis monitoring thread started");
    
    while (engine->simulation_running) {
        long long current_time = engine->get_time_manager()->get_current_time();
        
        // Update weather status
        crisis_mgr->update_weather(current_time);
        
        // Random weather event generation (20% chance every 4 seconds)
        if (check_cycle % 2 == 0 && (rand() % 100) < 20) {
            WeatherType wtype = static_cast<WeatherType>(rand() % 6);
            WeatherSeverity wsev = static_cast<WeatherSeverity>((rand() % 4) + 1);
            long long duration = 60 + (rand() % 240);  // 1-5 minutes
            
            WeatherEvent* weather = new WeatherEvent(
                weather_event_id++, wtype, wsev, current_time, duration);
            crisis_mgr->add_weather_event(weather);
            
            ostringstream log_msg;
            log_msg << "[CRISIS] Weather event: " << WeatherEvent::type_to_string(wtype)
                    << " severity " << WeatherEvent::severity_to_string(wsev)
                    << " - capacity now " << (crisis_mgr->get_operational_capacity() * 100) << "%";
            logger->log_event(log_msg.str());
        }
        
        // Random emergency event generation (10% chance every 4 seconds)
        if (check_cycle % 2 == 1 && (rand() % 100) < 10) {
            EmergencyType etype = static_cast<EmergencyType>(rand() % 7);
            int affected_flight = rand() % 10;  // Random flight ID
            
            EmergencyEvent* emergency = new EmergencyEvent(
                emergency_event_id++, etype, affected_flight, current_time);
            crisis_mgr->report_emergency(emergency);
            
            ostringstream log_msg;
            log_msg << "[CRISIS] Emergency: " << EmergencyEvent::type_to_string(etype)
                    << " on flight " << affected_flight
                    << " priority " << emergency->get_priority();
            logger->log_event(log_msg.str());
        }
        
        // Process next emergency if available
        EmergencyEvent* next_emergency = crisis_mgr->get_next_emergency();
        if (next_emergency) {
            // Simulate emergency handling
            int resolution_time = next_emergency->get_estimated_resolution_minutes();
            
            ostringstream log_msg;
            log_msg << "[CRISIS] Handling emergency " << next_emergency->get_id()
                    << " type " << EmergencyEvent::type_to_string(next_emergency->get_type())
                    << " - ETA " << resolution_time << " minutes";
            logger->log_event(log_msg.str());
            
            // Auto-resolve after simulated time (immediate for simulation)
            crisis_mgr->resolve_emergency(next_emergency->get_id(), current_time + resolution_time);
            
            log_msg.str("");
            log_msg << "[CRISIS] Emergency " << next_emergency->get_id() << " resolved";
            logger->log_event(log_msg.str());
        }
        
        // Check for ground stop
        if (crisis_mgr->is_ground_stop()) {
            logger->log_event("[CRISIS] GROUND STOP IN EFFECT - All departures halted");
        }
        
        check_cycle++;
        sleep(2); // Check every 2 seconds for more frequent crisis events
    }
    
    logger->log_event("[CrisisMonitor] Crisis monitoring thread stopped");
    return nullptr;
}

void* SimulationEngine::flight_generator_func(void* arg) {
    SimulationEngine* engine = static_cast<SimulationEngine*>(arg);
    Logger* logger = Logger::get_instance();
    
    logger->log_event("[FlightGenerator] Flight generation thread started");
    
    // Aircraft types for random selection
    AircraftType aircraft_types[] = {A380, B777, B737, A320, B777F, G650, FALCON_7X};
    const char* airlines[] = {"AA", "UA", "DL", "BA", "LH", "AF", "EK", "SQ", "QF", "CX"};
    
    while (engine->simulation_running) {
        // Generate a new flight every 2-5 real seconds (20-50 sim seconds)
        int delay_seconds = 2 + (rand() % 4);  // 2-5 seconds
        sleep(delay_seconds);
        
        if (!engine->simulation_running) break;
        
        // Check if there's capacity (don't overload)
        int active = engine->active_flight_count.load();
        if (active >= 8) {  // Max 8 concurrent flights
            continue;
        }
        
        // Generate flight ID
        int flight_num = engine->next_flight_id++;
        string airline = airlines[rand() % 10];
        string flight_id = airline + to_string(flight_num);
        
        // Random aircraft type
        AircraftType type = aircraft_types[rand() % 7];
        Aircraft* aircraft = new Aircraft(type);
        
        // Random flight type
        FlightType ftype = (rand() % 2 == 0) ? DOMESTIC : INTERNATIONAL;
        
        // Schedule arrival in near future (5-30 seconds from now)
        long long current_time = engine->get_time_manager()->get_current_time();
        long long arrival_time = current_time + 5 + (rand() % 26);
        long long departure_time = arrival_time + 120 + (rand() % 180);  // 2-5 min turnaround
        
        // Create flight
        Flight* flight = new Flight(flight_id, aircraft, ftype, arrival_time, departure_time);
        
        // Schedule arrival event
        FlightArrivalEvent* arrival_event = new FlightArrivalEvent(flight, engine, arrival_time);
        engine->get_event_queue()->push(arrival_event);
        
        ostringstream log_msg;
        log_msg << "[FlightGenerator] Created flight " << flight_id 
                << " (" << aircraft->get_type_name() << ")"
                << " arriving at T+" << arrival_time;
        logger->log_event(log_msg.str());
    }
    
    logger->log_event("[FlightGenerator] Flight generation thread stopped");
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
    pthread_create(&flight_generator_thread, nullptr, flight_generator_func, this);
    
    logger->log_event("[SimulationEngine] All threads started (including flight generator)");
    
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
    pthread_join(flight_generator_thread, nullptr);
    
    logger->log_event("[SimulationEngine] All threads stopped");
    logger->flush_all();
}

// REQ-5: ATC clearance system implementation
bool SimulationEngine::acquire_atc_clearance() {
    int current = available_atc.load();
    while (current > 0) {
        if (available_atc.compare_exchange_weak(current, current - 1)) {
            ostringstream log_msg;
            log_msg << "[ATC] Clearance granted. Available ATCs: " << (current - 1) << "/" << TOTAL_ATC;
            logger->log_event(log_msg.str());
            return true;
        }
    }
    logger->log_event("[ATC] Waiting for ATC clearance - all controllers busy");
    return false;
}

void SimulationEngine::release_atc_clearance() {
    int current = available_atc.load();
    while (current < TOTAL_ATC) {
        if (available_atc.compare_exchange_weak(current, current + 1)) {
            ostringstream log_msg;
            log_msg << "[ATC] Clearance released. Available ATCs: " << (current + 1) << "/" << TOTAL_ATC;
            logger->log_event(log_msg.str());
            return;
        }
    }
}

int SimulationEngine::get_available_atc() const {
    return available_atc.load();
}
