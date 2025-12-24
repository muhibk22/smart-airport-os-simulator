#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include "TimeManager.h"
#include "EventQueue.h"
#include "Logger.h"
#include "Dashboard.h"
#include "../airport/RunwayManager.h"
#include "../airport/GateManager.h"
#include "../airport/TaxiwayGraph.h"
#include "../scheduling/HMFQQueue.h"
#include "../memory/PageTable.h"
#include "../memory/TLB.h"
#include "../memory/WorkingSetManager.h"
#include "../memory/ClockReplacer.h"
#include "../memory/ThrashingDetector.h"
#include "../resources/ResourceManager.h"
#include "../crisis/CrisisManager.h"
#include "../memory/Prefetcher.h"
#include "../crew/CrewManager.h"
#include "../airport/Aircraft.h"
#include "../airport/Flight.h"
#include "FlightEvents.h"
#include "../finance/CostModel.h"
#include "../finance/RevenueModel.h"
#include <pthread.h>
#include <atomic>

using namespace std;

// Forward declarations
class HMFQQueue;
class PageTable;
class TLB;
class WorkingSetManager;
class ClockReplacer;
class ThrashingDetector;
class ResourceManager;
class CrisisManager;
class Prefetcher;
class CrewManager;

class SimulationEngine {
private:
    TimeManager* time_manager;
    EventQueue* event_queue;
    Logger* logger;
    Dashboard* dashboard;
    
    RunwayManager* runway_manager;
    GateManager* gate_manager;
    TaxiwayGraph* taxiway_graph;
    HMFQQueue* scheduler;  // HMFQ-PPRA Scheduler
    
    // Memory Manager components
    TLB* tlb;
    WorkingSetManager* working_set_manager;
    ClockReplacer* clock_replacer;
    ThrashingDetector* thrashing_detector;
    
    // Resource Manager (Banker's Algorithm)
    ResourceManager* resource_manager;
    
    // Crisis Manager (Weather and Emergencies)
    CrisisManager* crisis_manager;
    
    // Prefetcher for predictive memory access
    Prefetcher* prefetcher;
    
    // Crew Manager (LRU assignment with fatigue)
    CrewManager* crew_manager;
    
    // Finance tracking
    CostModel* cost_model;
    RevenueModel* revenue_model;
    
    // Performance tracking
    atomic<long long> total_turnaround_time;
    atomic<int> on_time_flights;
    atomic<int> delayed_flights;
    
    // Control threads
    pthread_t event_dispatcher_thread;
    pthread_t dashboard_updater_thread;
    pthread_t crisis_monitor_thread;
    pthread_t flight_generator_thread;  // Continuous flight generation
    
    // Flight generation
    atomic<int> next_flight_id;
    
    atomic<bool> simulation_running;
    long long simulation_duration;
    
    // Flight tracking for dashboard
    atomic<int> active_flight_count;
    atomic<int> flights_landing;
    atomic<int> flights_at_gates;
    atomic<int> flights_departing;
    atomic<int> total_flights_handled;
    
    // Thread functions
    static void* event_dispatcher_func(void* arg);
    static void* dashboard_updater_func(void* arg);
    static void* crisis_monitor_func(void* arg);
    static void* flight_generator_func(void* arg);  // Generates new flights
    
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
    HMFQQueue* get_scheduler() { return scheduler; }
    TLB* get_tlb() { return tlb; }
    ThrashingDetector* get_thrashing_detector() { return thrashing_detector; }
    ResourceManager* get_resource_manager() { return resource_manager; }
    CrisisManager* get_crisis_manager() { return crisis_manager; }
    Prefetcher* get_prefetcher() { return prefetcher; }
    WorkingSetManager* get_working_set_manager() { return working_set_manager; }
    CrewManager* get_crew_manager() { return crew_manager; }
    CostModel* get_cost_model() { return cost_model; }
    RevenueModel* get_revenue_model() { return revenue_model; }
    
    // Flight tracking updates
    void increment_active_flights() { active_flight_count++; }
    void decrement_active_flights() { active_flight_count--; }
    void increment_flights_landing() { flights_landing++; }
    void decrement_flights_landing() { flights_landing--; }
    void increment_flights_at_gates() { flights_at_gates++; }
    void decrement_flights_at_gates() { flights_at_gates--; }
    void increment_flights_departing() { flights_departing++; }
    void decrement_flights_departing() { flights_departing--; }
    void increment_total_handled() { total_flights_handled++; }
    
    // Performance tracking
    void record_turnaround(long long time_ms) { total_turnaround_time += time_ms; }
    void record_on_time() { on_time_flights++; }
    void record_delayed() { delayed_flights++; }
    double get_avg_turnaround() { return total_flights_handled > 0 ? (double)total_turnaround_time / total_flights_handled : 0; }
    double get_on_time_rate() { int total = on_time_flights + delayed_flights; return total > 0 ? (double)on_time_flights / total * 100 : 0; }
    
    // REQ-5: ATC (Air Traffic Controller) clearance system
    static constexpr int TOTAL_ATC = 4;          // 4 ATCs available
    static constexpr int ATC_SHIFT_HOURS = 8;    // 8-hour shifts
    bool acquire_atc_clearance();                // Returns true if ATC available
    void release_atc_clearance();                // Release ATC after landing/takeoff
    int get_available_atc() const;
    
private:
    atomic<int> available_atc;                   // REQ-5: Available ATC count
};

#endif // SIMULATION_ENGINE_H
