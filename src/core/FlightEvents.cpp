#include "FlightEvents.h"
#include "Logger.h"
#include "../airport/RunwayManager.h"
#include "../airport/GateManager.h"
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
    
    std::ostringstream log_msg;
    log_msg << "[FLIGHT_THREAD] Flight " << flight->flight_id << " thread started";
    logger->log_event(log_msg.str());
    
    // Track this flight as active
    engine->increment_active_flights();
    
    // ===== PHASE 1: ARRIVAL & RUNWAY REQUEST =====
    flight->status = APPROACHING;
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " approaching, requesting runway";
    logger->log_event(log_msg.str());
    
    Runway* runway = nullptr;
    int attempts = 0;
    const int MAX_ATTEMPTS = 30;
    
    while (runway == nullptr && attempts < MAX_ATTEMPTS) {
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
    
    if (runway == nullptr) {
        log_msg.str("");
        log_msg << "[FLIGHT] " << flight->flight_id << " FAILED to get runway after " 
                << MAX_ATTEMPTS << " attempts";
        logger->log_event(log_msg.str());
        engine->decrement_active_flights();
        delete data;
        return nullptr;
    }
    
    flight->assigned_runway_id = runway->get_id();
    
    // ===== PHASE 2: LANDING (runway reserved, using it) =====
    flight->status = LANDING;
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " landing on runway " << runway->get_name();
    logger->log_event(log_msg.str());
    
    sleep(5); // Simulate landing - longer so dashboard can see it
    
    flight->actual_arrival_time = engine->get_time_manager()->get_current_time();
    
    // ===== PHASE 3: RELEASE RUNWAY =====
    engine->get_runway_manager()->release_runway(
        runway->get_id(),
        engine->get_time_manager()->get_current_time()
    );
    
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " cleared runway " << runway->get_name();
    logger->log_event(log_msg.str());
    
    // ===== PHASE 4: TAXIING TO GATE =====
    flight->status = TAXIING_TO_GATE;
    sleep(3); // Simulate taxi time - longer so dashboard can see it
    
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
    
    // ===== PHASE 6: AT GATE & SERVICING =====
    flight->status = AT_GATE;
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " at gate " << gate->get_id();
    logger->log_event(log_msg.str());
    
    flight->status = SERVICING;
    int service_time = flight->aircraft->service_time_minutes;
    sleep(service_time / 30); // Scale down for testing (30 mins -> 1 sec)
    
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " servicing complete";
    logger->log_event(log_msg.str());
    
    // ===== PHASE 7: RELEASE GATE & DEPARTURE =====
    engine->get_gate_manager()->release_gate(gate->get_id());
    
    flight->status = DEPARTING;
    flight->actual_departure_time = engine->get_time_manager()->get_current_time();
    
    log_msg.str("");
    log_msg << "[FLIGHT] " << flight->flight_id << " departed. Total time: " 
            << (flight->actual_departure_time - flight->actual_arrival_time) << " time units";
    logger->log_event(log_msg.str());
    
    flight->status = DEPARTED;
    
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
