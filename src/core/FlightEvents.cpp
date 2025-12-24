#include "FlightEvents.h"
#include "Logger.h"
#include "../airport/RunwayManager.h"
#include "../airport/GateManager.h"
#include "../scheduling/HMFQQueue.h"
#include "../memory/TLB.h"
#include "../memory/Prefetcher.h"
#include "../memory/WorkingSetManager.h"
#include "../memory/ThrashingDetector.h"
#include "../resources/ResourceManager.h"
#include "../resources/Resource.h"
#include "../crew/CrewManager.h"
#include "../crew/Crew.h"
#include "../finance/CostModel.h"
#include "../finance/RevenueModel.h"
#include "../crisis/CrisisManager.h"      // REQ-1: Go-around weather check
#include "../crisis/WeatherEvent.h"       // REQ-1: Weather severity check
#include <sstream>
#include <unistd.h>
#include <cstdlib>

using namespace std;

// ========== Flight Lifecycle pthread Handler ==========

void* flight_lifecycle_handler(void* arg) {
    FlightThreadData* data = static_cast<FlightThreadData*>(arg);
    Flight* flight = data->flight;
    SimulationEngine* engine = data->engine;
    long long arrival_time = data->arrival_time;
    
    Logger* logger = Logger::get_instance();
    TLB* tlb = engine->get_tlb();
    
    ostringstream log_msg;
    log_msg << "[FLIGHT_THREAD] Flight " << flight->flight_id << " thread started";
    logger->log_event(log_msg.str());
    
    // Track this flight as active
    engine->increment_active_flights();
    
    // ===== AWSC-PPC Memory Simulation =====
    int flight_id_hash = abs((int)(flight->flight_id[0] + flight->flight_id[1] * 256));
    Prefetcher* prefetcher = engine->get_prefetcher();
    ThrashingDetector* thrash_detector = engine->get_thrashing_detector();
    WorkingSetManager* ws_manager = engine->get_working_set_manager();
    
    // Calculate working set window based on phase (initialization)
    double fault_rate = thrash_detector->is_in_thrashing_state() ? 0.25 : 0.05;
    int ws_window = ws_manager->calculate_window(PHASE_INIT, fault_rate);
    
    log_msg.str("");
    log_msg << "[MEMORY] Flight " << flight->flight_id << " working set window: " << ws_window;
    logger->log_memory(log_msg.str());
    
    // Simulate memory access for flight data (passenger manifest, baggage, etc.)
    for (int page = 0; page < 5; page++) {  // Each flight accesses ~5 pages
        // Record access for prefetcher pattern detection
        prefetcher->record_access(flight_id_hash, page);
        
        int frame = tlb->lookup(flight_id_hash, page);
        if (frame < 0) {
            // TLB miss - simulate page fault
            thrash_detector->record_fault();
            tlb->insert(flight_id_hash, page, page + flight_id_hash % 100);
            
            log_msg.str("");
            log_msg << "[MEMORY] Flight " << flight->flight_id << " TLB miss on page " << page;
            logger->log_memory(log_msg.str());
        } else {
            // TLB hit
            thrash_detector->record_hit();
        }
    }
    
    // Prefetch predicted pages
    vector<int> prefetch_candidates = prefetcher->get_prefetch_candidates(flight_id_hash);
    for (int pred_page : prefetch_candidates) {
        if (tlb->lookup(flight_id_hash, pred_page) < 0) {
            tlb->insert(flight_id_hash, pred_page, pred_page + flight_id_hash % 100);
            log_msg.str("");
            log_msg << "[MEMORY] Flight " << flight->flight_id << " prefetched page " << pred_page;
            logger->log_memory(log_msg.str());
        }
    }
    
    // Create scheduler operation for this flight
    HMFQQueue* scheduler = engine->get_scheduler();
    Operation* landing_op = scheduler->create_operation(flight, OP_LANDING, arrival_time);
    scheduler->enqueue(landing_op);
    
    // ===== PHASE 1: ARRIVAL & RUNWAY REQUEST WITH GO-AROUND =====
    // REQ-1: Go-around procedure implementation
    static const int GO_AROUND_DELAY_SECONDS = 2;  // Reduced delay for faster turnaround
    static const int MAX_GO_AROUNDS = 3;
    static const double GO_AROUND_FUEL_COST = 500.0;  // Extra fuel cost per go-around
    
    CrisisManager* crisis_mgr = engine->get_crisis_manager();
    
go_around_retry:
    flight->status = APPROACHING;
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " approaching, requesting runway";
    logger->log_event(log_msg.str());
    
    // REQ-1: Check weather safety before landing
    bool weather_unsafe = false;
    if (crisis_mgr) {
        WeatherEvent* active_weather = crisis_mgr->get_active_weather();
        if (active_weather && active_weather->get_severity() >= SEV_SEVERE) {
            weather_unsafe = true;
        }
        if (crisis_mgr->is_ground_stop()) {
            weather_unsafe = true;
        }
    }
    
    Runway* runway = nullptr;
    int attempts = 0;
    const int MAX_ATTEMPTS = 30;
    
    while (runway == nullptr && attempts < MAX_ATTEMPTS && !weather_unsafe) {
        runway = engine->get_runway_manager()->allocate_runway(
            flight, 
            engine->get_time_manager()->get_current_time()
        );
        
        if (runway == nullptr) {
            log_msg.str("");
            log_msg << "[FLIGHT] " << flight->flight_id << " waiting for runway (attempt " 
                    << (attempts + 1) << ")";
            logger->log_event(log_msg.str());
            sleep(1); // Wait 1 second before retry
            attempts++;
        }
    }
    
    // REQ-1: Trigger go-around if runway unavailable or weather unsafe
    if (runway == nullptr || weather_unsafe) {
        if (flight->go_around_count < MAX_GO_AROUNDS) {
            flight->go_around_count++;
            flight->status = GO_AROUND;
            
            log_msg.str("");
            log_msg << "[GO-AROUND] Flight " << flight->flight_id 
                    << " go-around #" << flight->go_around_count
                    << " - " << (weather_unsafe ? "weather unsafe" : "runway unavailable");
            logger->log_event(log_msg.str());
            
            // Add fuel cost for go-around
            CostModel* cost_model = engine->get_cost_model();
            if (cost_model) {
                cost_model->record_fuel(GO_AROUND_FUEL_COST / 3.50);  // Convert to gallons
                log_msg.str("");
                log_msg << "[GO-AROUND] Flight " << flight->flight_id 
                        << " extra fuel cost: $" << GO_AROUND_FUEL_COST;
                logger->log_event(log_msg.str());
            }
            
            // Wait go-around delay then retry
            sleep(GO_AROUND_DELAY_SECONDS);
            goto go_around_retry;
        } else {
            // Max go-arounds exceeded - diversion
            log_msg.str("");
            log_msg << "[DIVERSION] Flight " << flight->flight_id 
                    << " diverted after " << MAX_GO_AROUNDS << " go-arounds";
            logger->log_event(log_msg.str());
            engine->decrement_active_flights();
            delete data;
            return nullptr;
        }
    }
    
    flight->assigned_runway_id = runway->get_id();
    
    // ===== PHASE 2: LANDING (runway reserved, using it) =====
    flight->status = LANDING;
    engine->increment_flights_landing();  // Track landing
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " landing on runway " << runway->get_name();
    logger->log_event(log_msg.str());
    
    usleep(500000); // 0.5s landing - faster turnaround
    
    flight->actual_arrival_time = engine->get_time_manager()->get_current_time();
    
    // ===== PHASE 3: RELEASE RUNWAY =====
    engine->get_runway_manager()->release_runway(
        runway->get_id(),
        engine->get_time_manager()->get_current_time()
    );
    
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " cleared runway " << runway->get_name();
    logger->log_event(log_msg.str());
    
    engine->decrement_flights_landing();  // No longer landing
    
    // ===== PHASE 4: TAXIING TO GATE =====
    flight->status = TAXIING_TO_GATE;
    usleep(300000); // 0.3s taxi time - faster turnaround
    
    // ===== PHASE 5: GATE REQUEST =====
    Gate* gate = nullptr;
    attempts = 0;
    
    while (gate == nullptr && attempts < MAX_ATTEMPTS) {
        gate = engine->get_gate_manager()->allocate_gate(flight);
        
        if (gate == nullptr) {
            log_msg.str("");
            log_msg << "[FLIGHT] " << flight->flight_id << " waiting for gate (attempt " 
                    << (attempts + 1) << ")";
            logger->log_event(log_msg.str());
            sleep(1);
            attempts++;
        }
    }
    
    if (gate == nullptr) {
        log_msg.str("");
        log_msg << "[FLIGHT] " << flight->flight_id << " FAILED to get gate";
        logger->log_event(log_msg.str());
        engine->decrement_active_flights();
        delete data;
        return nullptr;
    }
    
    flight->assigned_gate_id = gate->get_id();
    
    // ===== PHASE 6: AT GATE & SERVICING WITH RESOURCE ALLOCATION =====
    flight->status = AT_GATE;
    engine->increment_flights_at_gates();  // Track at gate
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " at gate " << gate->get_id();
    logger->log_event(log_msg.str());
    
    flight->status = SERVICING;
    ResourceManager* res_mgr = engine->get_resource_manager();
    long long current_time = engine->get_time_manager()->get_current_time();
    
    // ===== GROUND SERVICE: GPU (Ground Power Unit) =====
    log_msg.str("");
    log_msg << "[RESOURCE] " << flight->flight_id << " requesting GPU";
    logger->log_resource(log_msg.str());
    
    Resource* gpu = res_mgr->allocate_resource(RES_GROUND_POWER_UNIT, gate->get_id(), current_time, 30);
    if (gpu) {
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " acquired " << gpu->get_name();
        logger->log_resource(log_msg.str());
    }
    
    // ===== GROUND SERVICE: FUEL TRUCK =====
    log_msg.str("");
    log_msg << "[RESOURCE] " << flight->flight_id << " requesting fuel truck";
    logger->log_resource(log_msg.str());
    
    Resource* fuel_truck = nullptr;
    attempts = 0;
    while (fuel_truck == nullptr && attempts < 10) {
        fuel_truck = res_mgr->allocate_resource(RES_FUEL_TRUCK, gate->get_id(), current_time, 15);
        if (fuel_truck == nullptr) {
            usleep(500000); // Wait 500ms before retry
            attempts++;
        }
    }
    
    if (fuel_truck) {
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " acquired " << fuel_truck->get_name() << " - refueling";
        logger->log_resource(log_msg.str());
        usleep(200000); // 0.2s refueling
        res_mgr->release_resource(fuel_truck);
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " released fuel truck - refueling complete";
        logger->log_resource(log_msg.str());
    }
    
    // ===== GROUND SERVICE: CATERING =====
    log_msg.str("");
    log_msg << "[RESOURCE] " << flight->flight_id << " requesting catering vehicle";
    logger->log_resource(log_msg.str());
    
    Resource* catering = res_mgr->allocate_resource(RES_CATERING_VEHICLE, gate->get_id(), current_time, 10);
    if (catering) {
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " acquired " << catering->get_name() << " - catering";
        logger->log_resource(log_msg.str());
        usleep(100000); // 0.1s catering
        res_mgr->release_resource(catering);
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " released catering - complete";
        logger->log_resource(log_msg.str());
    }
    
    // ===== GROUND SERVICE: CLEANING CREW =====
    log_msg.str("");
    log_msg << "[RESOURCE] " << flight->flight_id << " requesting cleaning crew";
    logger->log_resource(log_msg.str());
    
    Resource* cleaning = res_mgr->allocate_resource(RES_CLEANING_CREW, gate->get_id(), current_time, 20);
    if (cleaning) {
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " acquired " << cleaning->get_name() << " - cleaning";
        logger->log_resource(log_msg.str());
        usleep(200000); // 0.2s cleaning
        res_mgr->release_resource(cleaning);
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " released cleaning crew - complete";
        logger->log_resource(log_msg.str());
    }
    
    // ===== GROUND SERVICE: BAGGAGE HANDLING =====
    log_msg.str("");
    log_msg << "[RESOURCE] " << flight->flight_id << " requesting baggage cart";
    logger->log_resource(log_msg.str());
    
    Resource* baggage = res_mgr->allocate_resource(RES_BAGGAGE_CART, gate->get_id(), current_time, 15);
    if (baggage) {
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " acquired " << baggage->get_name() << " - loading baggage";
        logger->log_resource(log_msg.str());
        usleep(100000); // 0.1s baggage
        res_mgr->release_resource(baggage);
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " released baggage cart - complete";
        logger->log_resource(log_msg.str());
    }
    
    // ===== GROUND SERVICE: TUG FOR PUSHBACK =====
    log_msg.str("");
    log_msg << "[RESOURCE] " << flight->flight_id << " requesting aircraft tug for pushback";
    logger->log_resource(log_msg.str());
    
    Resource* tug = res_mgr->allocate_resource(RES_AIRCRAFT_TUG, gate->get_id(), current_time, 5);
    
    // Release GPU before departure
    if (gpu) {
        res_mgr->release_resource(gpu);
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " released GPU";
        logger->log_resource(log_msg.str());
    }
    
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " servicing complete";
    logger->log_event(log_msg.str());
    
    // ===== PHASE 7: RELEASE TUG, GATE & DEPARTURE =====
    if (tug) {
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " using " << tug->get_name() << " for pushback";
        logger->log_resource(log_msg.str());
    }
    
    engine->get_gate_manager()->release_gate(gate->get_id());
    engine->decrement_flights_at_gates();  // No longer at gate
    
    // Release tug after pushback
    if (tug) {
        usleep(100000); // 0.1s pushback
        res_mgr->release_resource(tug);
        log_msg.str("");
        log_msg << "[RESOURCE] " << flight->flight_id << " released tug - pushback complete";
        logger->log_resource(log_msg.str());
    }
    
    flight->status = DEPARTING;
    engine->increment_flights_departing();  // Track departing
    flight->actual_departure_time = engine->get_time_manager()->get_current_time();
    
    // Calculate turnaround time and record performance
    long long turnaround = flight->actual_departure_time - flight->actual_arrival_time;
    engine->record_turnaround(turnaround);
    
    // Check if on-time (allow 120 time units for turnaround - realistic target)
    long long scheduled_departure = flight->actual_arrival_time + 120;
    if (flight->actual_departure_time <= scheduled_departure) {
        engine->record_on_time();
    } else {
        engine->record_delayed();
    }
    
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " departed. Turnaround: " 
            << turnaround << " time units";
    logger->log_event(log_msg.str());
    
    flight->status = DEPARTED;
    engine->decrement_flights_departing();  // No longer departing
    
    // Complete scheduler operation
    scheduler->complete(landing_op);
    
    // ===== FINANCIAL TRACKING =====
    CostModel* cost_model = engine->get_cost_model();
    RevenueModel* revenue_model = engine->get_revenue_model();
    
    // Record costs (scale fuel to reasonable amount - 500-2000 gallons typical refuel)
    double fuel_gallons = (flight->aircraft->fuel_capacity_gallons / 20.0);  // ~5% refuel for short turn
    cost_model->record_fuel(fuel_gallons);
    cost_model->record_gate((turnaround / 3600.0));  // Gate time in hours (turnaround in seconds)
    if (flight->actual_departure_time > scheduled_departure) {
        int delay_minutes = (flight->actual_departure_time - scheduled_departure) / 60;
        cost_model->record_delay(delay_minutes, flight->passenger_count);
    }
    
    // Record revenue (use cargo_capacity_kg as proxy for weight)
    revenue_model->record_landing(flight->aircraft->cargo_capacity_kg / 1000.0, 
                                   flight->flight_type == INTERNATIONAL);
    revenue_model->record_gate((turnaround / 3600.0), flight->flight_type == INTERNATIONAL);
    revenue_model->record_passengers(flight->passenger_count);
    
    log_msg.str("");
    log_msg << "[FINANCE] Flight " << flight->flight_id 
            << " - Revenue: $" << revenue_model->get_total_revenue()
            << " Cost: $" << cost_model->get_total_cost();
    logger->log_performance(log_msg.str());
    
    // Update counters
    engine->decrement_active_flights();
    engine->increment_total_handled();
    
    delete data;
    return nullptr;
}

// ========== FlightArrivalEvent Implementation ==========

FlightArrivalEvent::FlightArrivalEvent(Flight* f, SimulationEngine* eng, long long time)
    : Event(FLIGHT_ARRIVAL, time, f->priority), flight(f), engine(eng) {
    
    std::ostringstream desc;
    desc << "FlightArrival:" << flight->flight_id;
    description = desc.str();
}

void FlightArrivalEvent::process() {
    Logger* logger = Logger::get_instance();
    
    std::ostringstream log_msg;
    log_msg << "[EVENT] Processing FlightArrivalEvent for " << flight->flight_id;
    logger->log_event(log_msg.str());
    
    // Create thread data
    FlightThreadData* thread_data = new FlightThreadData{
        flight,
        engine,
        event_time
    };
    
    // Create pthread for flight lifecycle
    pthread_t flight_thread;
    int result = pthread_create(&flight_thread, nullptr, flight_lifecycle_handler, thread_data);
    
    if (result != 0) {
        log_msg.str("");
        log_msg << "[EVENT] ERROR: Failed to create thread for flight " << flight->flight_id;
        logger->log_event(log_msg.str());
        delete thread_data;
        return;
    }
    
    // Detach thread (fire and forget)
    pthread_detach(flight_thread);
    
    log_msg.str("");
    log_msg << "[EVENT] Flight thread created for " << flight->flight_id;
    logger->log_event(log_msg.str());
}

// ========== FlightDepartureEvent Implementation ==========

FlightDepartureEvent::FlightDepartureEvent(Flight* f, SimulationEngine* eng, long long time)
    : Event(FLIGHT_DEPARTURE, time, f->priority), flight(f), engine(eng) {
    
    std::ostringstream desc;
    desc << "FlightDeparture:" << flight->flight_id;
    description = desc.str();
}

void FlightDepartureEvent::process() {
    Logger* logger = Logger::get_instance();
    
    std::ostringstream log_msg;
    log_msg << "[EVENT] Flight " << flight->flight_id << " departure event processed";
    logger->log_event(log_msg.str());
    
    // In full implementation, would handle departure sequence
    // For now, just log
}

// ========== ServiceStartEvent Implementation ==========

ServiceStartEvent::ServiceStartEvent(Flight* f, const std::string& service, long long time)
    : Event(SERVICE_START, time, 50), flight(f), service_name(service) {
    
    std::ostringstream desc;
    desc << "ServiceStart:" << service_name << ":" << flight->flight_id;
    description = desc.str();
}

void ServiceStartEvent::process() {
    Logger* logger = Logger::get_instance();
    
    std::ostringstream log_msg;
    log_msg << "[EVENT] Service " << service_name << " started for flight " << flight->flight_id;
    logger->log_event(log_msg.str());
}

// ========== ServiceEndEvent Implementation ==========

ServiceEndEvent::ServiceEndEvent(Flight* f, const std::string& service, long long time)
    : Event(SERVICE_END, time, 50), flight(f), service_name(service) {
    
    std::ostringstream desc;
    desc << "ServiceEnd:" << service_name << ":" << flight->flight_id;
    description = desc.str();
}

void ServiceEndEvent::process() {
    Logger* logger = Logger::get_instance();
    
    std::ostringstream log_msg;
    log_msg << "[EVENT] Service " << service_name << " completed for flight " << flight->flight_id;
    logger->log_event(log_msg.str());
}
