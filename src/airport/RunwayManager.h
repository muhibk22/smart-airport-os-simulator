#ifndef RUNWAY_MANAGER_H
#define RUNWAY_MANAGER_H

#include "Runway.h"
#include <vector>
#include <pthread.h>

using namespace std;

class RunwayManager {
private:
    vector<Runway*> runways;
    pthread_mutex_t manager_mutex;
    
public:
    RunwayManager();
    ~RunwayManager();
    
    void add_runway(Runway* runway);
    
    // Allocate runway for flight
    Runway* allocate_runway(Flight* flight, long long current_time);
    
    // Release runway
    void release_runway(int runway_id, long long current_time);
    
    // Get runway by ID
    Runway* get_runway(int id);
    
    // Get all runways
    vector<Runway*> get_all_runways();
    
    // Statistics
    int get_available_runway_count();
};

#endif // RUNWAY_MANAGER_H
