#include "RunwayManager.h"
#include "../core/Logger.h"
#include <sstream>

using namespace std;

RunwayManager::RunwayManager() {
    pthread_mutex_init(&manager_mutex, nullptr);
}

RunwayManager::~RunwayManager() {
    pthread_mutex_destroy(&manager_mutex);
    
    for (Runway* runway : runways) {
        delete runway;
    }
}

void RunwayManager::add_runway(Runway* runway) {
    pthread_mutex_lock(&manager_mutex);
    runways.push_back(runway);
    pthread_mutex_unlock(&manager_mutex);
}

Runway* RunwayManager::allocate_runway(Flight* flight, long long current_time) {
    // Phase 1: Find available runway (coarse-grained lock for search)
    pthread_mutex_lock(&manager_mutex);
    
    Runway* chosen_runway = nullptr;
    
    // Try to find an available runway
    for (Runway* runway : runways) {
        if (runway->is_available()) {
            chosen_runway = runway;
            break;
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
    // END search phase
    
    if (chosen_runway == nullptr) {
        return nullptr; // No runway available
    }
    
    // Phase 2: Try to reserve chosen runway (fine-grained lock in Runway class)
    if (chosen_runway->try_reserve(flight, current_time)) {
        // Log allocation
        std::ostringstream log_msg;
        log_msg << "[RunwayManager] Flight " << flight->flight_id 
                << " allocated runway " << chosen_runway->get_name();
        Logger::get_instance()->log_event(log_msg.str());
        
        return chosen_runway;
    }
    
    // Runway became unavailable or wake turbulence wait failed
    return nullptr;
}

void RunwayManager::release_runway(int runway_id, long long current_time) {
    pthread_mutex_lock(&manager_mutex);
    
    for (Runway* runway : runways) {
        if (runway->get_id() == runway_id) {
            pthread_mutex_unlock(&manager_mutex);
            
            // Release with runway's fine-grained lock
            runway->release(current_time);
            
            std::ostringstream log_msg;
            log_msg << "[RunwayManager] Runway " << runway->get_name() << " released";
            Logger::get_instance()->log_event(log_msg.str());
            
            return;
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
}

Runway* RunwayManager::get_runway(int id) {
    pthread_mutex_lock(&manager_mutex);
    
    for (Runway* runway : runways) {
        if (runway->get_id() == id) {
            pthread_mutex_unlock(&manager_mutex);
            return runway;
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
    return nullptr;
}

std::vector<Runway*> RunwayManager::get_all_runways() {
    pthread_mutex_lock(&manager_mutex);
    std::vector<Runway*> copy = runways;
    pthread_mutex_unlock(&manager_mutex);
    return copy;
}

int RunwayManager::get_available_runway_count() {
    pthread_mutex_lock(&manager_mutex);
    
    int count = 0;
    for (Runway* runway : runways) {
        if (runway->is_available()) {
            count++;
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
    return count;
}
