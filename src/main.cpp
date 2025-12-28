#include "core/SimulationEngine.h"
#include <iostream>
#include <cstdlib>
#include <csignal>

using namespace std;

// Global pointer for signal handler cleanup
static SimulationEngine* g_engine = nullptr;
static volatile sig_atomic_t g_shutdown_requested = 0;

// Signal handler for graceful shutdown (prevents zombie processes)
void signal_handler(int signum) {
    cout << "\n[SIGNAL] Received signal " << signum << " - shutting down gracefully..." << endl;
    g_shutdown_requested = 1;
    if (g_engine) {
        g_engine->stop();
    }
}

int main() {
    // Register signal handlers for clean shutdown
    signal(SIGINT, signal_handler);   // Ctrl+C
    signal(SIGTERM, signal_handler);  // Terminal close
    #ifdef _WIN32
    signal(SIGBREAK, signal_handler); // Windows console close
    #endif
    
    cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    cout << "║       Smart Airport OS Simulator - Starting Up               ║\n";
    cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";
    
    // Initialize simulation engine
    SimulationEngine* engine = new SimulationEngine();
    g_engine = engine;  // Set global pointer for signal handler
    
    try {
        engine->initialize();
        
        cout << "Press Enter to start simulation (Ctrl+C to exit)...\n";
        cin.get();
        
        if (!g_shutdown_requested) {
            engine->run();
        }
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        g_engine = nullptr;
        delete engine;
        return 1;
    }
    
    g_engine = nullptr;
    delete engine;
    
    cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    cout << "║       Simulation Complete - Check logs/ directory             ║\n";
    cout << "╚═══════════════════════════════════════════════════════════════╝\n";
    
    return 0;
}

