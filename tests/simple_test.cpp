#include "../src/core/SimulationEngine.h"
#include "../src/core/FlightEvents.h"
#include "../src/core/Logger.h"
#include "../src/airport/Aircraft.h"
#include "../src/airport/Flight.h"
#include <iostream>
#include <vector>
#include <unistd.h>

using namespace std;

int main() {
    cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    cout << "║          Simple pthread Test - Airport Infrastructure         ║\n";
    cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";
    
    // Initialize simulation engine
    SimulationEngine* engine = new SimulationEngine();
    engine->initialize();
    
    Logger* logger = Logger::get_instance();
    logger->log_event("=== Simple Test Started ===");
    
    cout << "Creating test flights...\n";
    
    // Create 5 test flights with different aircraft types
    vector<Flight*> test_flights;
    
    // Flight 1: A380 (Heavy, needs large gate, international)
    Aircraft* ac1 = new Aircraft(A380);
    Flight* f1 = new Flight("AA100", ac1, INTERNATIONAL, 100, 5000);
    test_flights.push_back(f1);
    
    // Flight 2: B777 (Heavy, needs heavy/large gate, international)
    Aircraft* ac2 = new Aircraft(B777);
    Flight* f2 = new Flight("UA200", ac2, INTERNATIONAL, 105, 5100);
    test_flights.push_back(f2);
    
    // Flight 3: B737 (Medium, domestic)
    Aircraft* ac3 = new Aircraft(B737);
    Flight* f3 = new Flight("DL300", ac3, DOMESTIC, 110, 4500);
    test_flights.push_back(f3);
    
    // Flight 4: A320 (Medium, domestic)
    Aircraft* ac4 = new Aircraft(A320);
    Flight* f4 = new Flight("SW400", ac4, DOMESTIC, 115, 4600);
    test_flights.push_back(f4);
    
    // Flight 5: G650 (Light, private, domestic)
    Aircraft* ac5 = new Aircraft(G650);
    Flight* f5 = new Flight("PVT500", ac5, DOMESTIC, 120, 4200);
    test_flights.push_back(f5);
    
    cout << "Created " << test_flights.size() << " test flights\n";
    cout << "  - " << f1->flight_id << " (" << ac1->get_type_name() << ")\n";
    cout << "  - " << f2->flight_id << " (" << ac2->get_type_name() << ")\n";
    cout << "  - " << f3->flight_id << " (" << ac3->get_type_name() << ")\n";
    cout << "  - " << f4->flight_id << " (" << ac4->get_type_name() << ")\n";
    cout << "  - " << f5->flight_id << " (" << ac5->get_type_name() << ")\n\n";
    
    // Create FlightArrivalEvents and add to event queue
    cout << "Scheduling arrival events...\n";
    
    for (size_t i = 0; i < test_flights.size(); i++) {
        Flight* flight = test_flights[i];
        long long arrival_time = 100 + (i * 5); // Stagger arrivals by 5 time units
        
        FlightArrivalEvent* event = new FlightArrivalEvent(flight, engine, arrival_time);
        engine->get_event_queue()->push(event);
        
        cout << "  - Scheduled " << flight->flight_id << " at time " << arrival_time << "\n";
    }
    
    cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    cout << "║                    Starting Simulation                        ║\n";
    cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";
    
    cout << "Watch logs/ directory for detailed output...\n";
    cout << "  - logs/events.log (all events)\n";
    cout << "  - logs/scheduling.log (scheduling decisions)\n\n";
    
    // Start simulation engine (will create threads)
    pthread_t sim_thread;
    pthread_create(&sim_thread, nullptr, [](void* arg) -> void* {
        SimulationEngine* eng = static_cast<SimulationEngine*>(arg);
        eng->run();
        return nullptr;
    }, engine);
    
    // Let simulation run for 60 seconds
    cout << "Running simulation for 60 seconds...\n\n";
    sleep(60);
    
    // Stop simulation
    cout << "\nStopping simulation...\n";
    engine->stop();
    pthread_join(sim_thread, nullptr);
    
    cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    cout << "║                    Simulation Complete                        ║\n";
    cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";
    
    // Print summary
    cout << "Test Summary:\n";
    cout << "  Total Flights: " << test_flights.size() << "\n";
    
    int completed = 0;
    for (Flight* f : test_flights) {
        if (f->status == DEPARTED) {
            completed++;
        }
        cout << "  - " << f->flight_id << ": " << f->get_status_string() << "\n";
    }
    
    cout << "\n  Completed: " << completed << "/" << test_flights.size() << "\n";
    
    // Cleanup
    delete engine;
    for (Flight* f : test_flights) {
        delete f->aircraft;
        delete f;
    }
    
    cout << "\nCheck logs/ directory for detailed pthread behavior!\n";
    cout << "\n✓ Test finished successfully\n\n";
    
    return 0;
}
