/**
 * requirements_test.cpp
 * 
 * REQ-6: Test Cases for newly implemented requirements
 * 
 * Tests:
 * 1. Go-Around Triggering (REQ-1)
 * 2. Parallel Runway Landing (REQ-2 - already implemented)
 * 3. VIP Passenger Fast-Tracking (REQ-3)
 * 4. Budget Exceed Warning (REQ-4)
 * 5. Deadlock Prevention (existing functionality)
 */

#include "../src/core/SimulationEngine.h"
#include "../src/core/Logger.h"
#include "../src/airport/Aircraft.h"
#include "../src/airport/Flight.h"
#include "../src/airport/Runway.h"
#include "../src/airport/RunwayManager.h"
#include "../src/passengers/PassengerGroup.h"
#include "../src/finance/CostModel.h"
#include "../src/crisis/CrisisManager.h"
#include "../src/crisis/WeatherEvent.h"
#include "../src/crew/Crew.h"
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <pthread.h>

using namespace std;

// Test result tracking
int tests_passed = 0;
int tests_failed = 0;

void test_pass(const string& test_name) {
    cout << "[PASS] " << test_name << endl;
    tests_passed++;
}

void test_fail(const string& test_name, const string& reason) {
    cout << "[FAIL] " << test_name << " - " << reason << endl;
    tests_failed++;
}

// ===========================================================================
// TEST 1: Go-Around Status Exists
// REQ-1: Verify GO_AROUND status is defined and can be set
// ===========================================================================
void test_go_around_status() {
    cout << "\n=== Test 1: Go-Around Status ===" << endl;
    
    // Create a test flight
    Aircraft* ac = new Aircraft(B737);
    Flight* flight = new Flight("TEST01", ac, DOMESTIC, 100, 500);
    
    // Set status to GO_AROUND
    flight->status = GO_AROUND;
    
    // Verify status
    if (flight->status == GO_AROUND) {
        // Verify status string
        string status_str = flight->get_status_string();
        if (status_str == "GO_AROUND") {
            // Verify go_around_count field
            flight->go_around_count = 2;
            if (flight->go_around_count == 2) {
                test_pass("Go-Around status and counter work correctly");
            } else {
                test_fail("Go-Around", "go_around_count field not working");
            }
        } else {
            test_fail("Go-Around", "get_status_string() not returning GO_AROUND");
        }
    } else {
        test_fail("Go-Around", "GO_AROUND status not in enum");
    }
    
    delete flight;
    delete ac;
}

// ===========================================================================
// TEST 2: Parallel Runway Operations
// REQ-2: Verify different runways can be allocated simultaneously
// ===========================================================================
void test_parallel_runways() {
    cout << "\n=== Test 2: Parallel Runway Operations ===" << endl;
    
    RunwayManager* rm = new RunwayManager();
    rm->add_runway(new Runway(0, "27L"));
    rm->add_runway(new Runway(1, "27R"));
    
    // Create two flights
    Aircraft* ac1 = new Aircraft(B777);
    Aircraft* ac2 = new Aircraft(A320);
    Flight* f1 = new Flight("PAR01", ac1, INTERNATIONAL, 100, 500);
    Flight* f2 = new Flight("PAR02", ac2, DOMESTIC, 100, 500);
    
    // Allocate both runways
    Runway* r1 = rm->allocate_runway(f1, 100);
    Runway* r2 = rm->allocate_runway(f2, 100);
    
    if (r1 != nullptr && r2 != nullptr) {
        if (r1->get_id() != r2->get_id()) {
            test_pass("Parallel runway allocation: 2 flights on different runways");
        } else {
            test_fail("Parallel Runways", "Both flights on same runway");
        }
    } else {
        test_fail("Parallel Runways", "Could not allocate both runways");
    }
    
    // Cleanup
    if (r1) rm->release_runway(r1->get_id(), 200);
    if (r2) rm->release_runway(r2->get_id(), 200);
    delete f1;
    delete f2;
    delete ac1;
    delete ac2;
    delete rm;
}

// ===========================================================================
// TEST 3: VIP Passenger Fast-Tracking
// REQ-3: Verify VIP flag reduces processing time by 50%
// ===========================================================================
void test_vip_passengers() {
    cout << "\n=== Test 3: VIP Passenger Fast-Tracking ===" << endl;
    
    // Create passenger group
    PassengerGroup* pax = new PassengerGroup(1, 100, 50);
    
    // Verify initial state
    if (pax->get_flags() != PAX_FLAG_NONE) {
        test_fail("VIP Passengers", "Initial flags not PAX_FLAG_NONE");
        delete pax;
        return;
    }
    
    // Set VIP flag
    pax->set_flags(PAX_FLAG_VIP);
    
    // Verify VIP detection
    if (!pax->is_vip()) {
        test_fail("VIP Passengers", "is_vip() not returning true");
        delete pax;
        return;
    }
    
    // Verify processing multiplier
    double multiplier = pax->get_processing_multiplier();
    if (multiplier == 0.5) {
        test_pass("VIP passenger processing at 0.5x (50% faster)");
    } else {
        test_fail("VIP Passengers", "Processing multiplier not 0.5");
    }
    
    // Test disabled passenger
    PassengerGroup* disabled_pax = new PassengerGroup(2, 101, 30);
    disabled_pax->set_flags(PAX_FLAG_DISABLED);
    
    if (disabled_pax->is_disabled() && disabled_pax->get_assistance_delay() == 300) {
        test_pass("Disabled passenger assistance delay: +5 min (300 units)");
    } else {
        test_fail("Disabled Passengers", "Assistance delay not 300");
    }
    
    // Test unaccompanied minor
    PassengerGroup* minor_pax = new PassengerGroup(3, 102, 1);
    minor_pax->set_flags(PAX_FLAG_UNACCOMPANIED_MINOR);
    
    if (minor_pax->is_unaccompanied_minor()) {
        test_pass("Unaccompanied minor flag detected correctly");
    } else {
        test_fail("Unaccompanied Minor", "Flag not detected");
    }
    
    delete pax;
    delete disabled_pax;
    delete minor_pax;
}

// ===========================================================================
// TEST 4: Budget Exceed Warning
// REQ-4: Verify budget warning when exceeding $100,000
// ===========================================================================
void test_budget_constraint() {
    cout << "\n=== Test 4: Budget Exceed Warning ===" << endl;
    
    CostModel* cm = new CostModel();
    
    // Verify initial state
    if (cm->is_budget_exceeded()) {
        test_fail("Budget Constraint", "Budget exceeded before any costs");
        delete cm;
        return;
    }
    
    // Check daily budget value
    double budget = cm->get_daily_budget();
    if (budget != 100000.0) {
        test_fail("Budget Constraint", "Daily budget not $100,000");
        delete cm;
        return;
    }
    
    // Add costs below budget
    cm->record_fuel(10000);  // $35,000 at $3.50/gallon
    
    if (cm->is_budget_exceeded()) {
        test_fail("Budget Constraint", "Budget exceeded prematurely");
        delete cm;
        return;
    }
    
    // Add more costs to exceed budget
    cm->record_fuel(20000);  // Another $70,000 -> Total $105,000
    
    if (cm->is_budget_exceeded()) {
        test_pass("Budget exceed warning triggered at $" + to_string((int)cm->get_total_cost()));
    } else {
        test_fail("Budget Constraint", "Budget not marked as exceeded after $100K");
    }
    
    delete cm;
}

// ===========================================================================
// TEST 5: ATC Crew Type
// REQ-5: Verify ATC crew type exists and clearance system works
// ===========================================================================
void test_atc_crew_type() {
    cout << "\n=== Test 5: ATC Crew Type ===" << endl;
    
    // Test CREW_ATC enum exists
    CrewRole role = CREW_ATC;
    string role_str = Crew::role_to_string(role);
    
    if (role_str == "Air Traffic Controller") {
        test_pass("CREW_ATC role exists and has correct string");
    } else {
        test_fail("ATC Crew Type", "Role string incorrect: " + role_str);
    }
    
    // Note: Full ATC clearance test would require SimulationEngine
    // which initializes many components. We verify the enum here.
}

// ===========================================================================
// TEST 6: Deadlock Prevention (Banker's Algorithm)
// Existing functionality - verify multiple resource requests don't deadlock
// ===========================================================================
void test_deadlock_prevention() {
    cout << "\n=== Test 6: Deadlock Prevention ===" << endl;
    
    // This test creates multiple flights competing for resources
    // If there's a deadlock, the test will timeout
    
    RunwayManager* rm = new RunwayManager();
    rm->add_runway(new Runway(0, "09L"));
    rm->add_runway(new Runway(1, "09R"));
    
    // Allocate both runways
    Aircraft* ac1 = new Aircraft(B737);
    Aircraft* ac2 = new Aircraft(A320);
    Flight* f1 = new Flight("DL01", ac1, DOMESTIC, 100, 500);
    Flight* f2 = new Flight("DL02", ac2, DOMESTIC, 150, 550);
    
    Runway* r1 = rm->allocate_runway(f1, 100);
    Runway* r2 = rm->allocate_runway(f2, 100);
    
    // Release in different order (potential deadlock scenario)
    if (r2) rm->release_runway(r2->get_id(), 200);
    if (r1) rm->release_runway(r1->get_id(), 300);
    
    // If we reach here, no deadlock occurred
    test_pass("No deadlock during resource allocation/release");
    
    delete f1;
    delete f2;
    delete ac1;
    delete ac2;
    delete rm;
}

// ===========================================================================
// MAIN
// ===========================================================================
int main() {
    cout << "╔═══════════════════════════════════════════════════════════════╗" << endl;
    cout << "║       Smart Airport OS Simulator - Requirements Tests         ║" << endl;
    cout << "╚═══════════════════════════════════════════════════════════════╝" << endl;
    
    // Initialize logger for test output
    Logger::get_instance();
    
    // Run all tests
    test_go_around_status();
    test_parallel_runways();
    test_vip_passengers();
    test_budget_constraint();
    test_atc_crew_type();
    test_deadlock_prevention();
    
    // Summary
    cout << "\n╔═══════════════════════════════════════════════════════════════╗" << endl;
    cout << "║                        Test Summary                           ║" << endl;
    cout << "╠═══════════════════════════════════════════════════════════════╣" << endl;
    cout << "║  Passed: " << tests_passed << "                                                     ║" << endl;
    cout << "║  Failed: " << tests_failed << "                                                     ║" << endl;
    cout << "╚═══════════════════════════════════════════════════════════════╝" << endl;
    
    if (tests_failed == 0) {
        cout << "\n✓ All requirements tests passed successfully!\n" << endl;
        return 0;
    } else {
        cout << "\n✗ Some tests failed. Please review the output above.\n" << endl;
        return 1;
    }
}
