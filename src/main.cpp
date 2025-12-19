#include "core/SimulationEngine.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv) {
    std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║       Smart Airport OS Simulator - Starting Up               ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n\n";
    
    // Initialize simulation engine
    SimulationEngine* engine = new SimulationEngine();
    
    try {
        engine->initialize();
        
        std::cout << "Press Enter to start simulation...\n";
        std::cin.get();
        
        engine->run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        delete engine;
        return 1;
    }
    
    delete engine;
    
    std::cout << "\n╔═══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║       Simulation Complete - Check logs/ directory             ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";
    
    return 0;
}
