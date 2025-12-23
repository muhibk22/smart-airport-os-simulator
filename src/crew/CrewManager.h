#ifndef CREW_MANAGER_H
#define CREW_MANAGER_H

#include "Crew.h"
#include <vector>
#include <unordered_map>
#include <list>
#include <pthread.h>

using namespace std;

// CrewManager with LRU replacement for crew assignment

class CrewManager {
private:
    vector<Crew*> all_crew;
    unordered_map<CrewRole, list<Crew*>> available_by_role;  // LRU order
    
    pthread_mutex_t manager_mutex;
    
public:
    CrewManager();
    ~CrewManager();
    
    // Initialize crew pool
    void initialize(int pilots, int co_pilots, int attendants, 
                   int technicians, int handlers, int agents, int fuel_techs);
    
    // Assign crew (LRU - least recently used first)
    Crew* assign_crew(CrewRole role, int flight_id, long long current_time);
    
    // Release crew back to pool
    void release_crew(Crew* crew, long long current_time);
    
    // Get available crew count by role
    int get_available_count(CrewRole role);
    
    // Get fatigued crew that need rest
    vector<Crew*> get_fatigued_crew();
    
    // Statistics
    double get_average_fatigue();
    int get_total_crew() const { return all_crew.size(); }
};

#endif // CREW_MANAGER_H
