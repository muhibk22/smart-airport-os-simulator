#include "CrewManager.h"
#include <algorithm>

using namespace std;

CrewManager::CrewManager() {
    pthread_mutex_init(&manager_mutex, nullptr);
}

CrewManager::~CrewManager() {
    pthread_mutex_destroy(&manager_mutex);
    for (Crew* c : all_crew) {
        delete c;
    }
}

void CrewManager::initialize(int pilots, int co_pilots, int attendants,
                              int technicians, int handlers, int agents, int fuel_techs) {
    pthread_mutex_lock(&manager_mutex);
    
    int id = 0;
    
    // Create pilots
    for (int i = 0; i < pilots; i++) {
        Crew* c = new Crew(id++, "Pilot-" + to_string(i), CREW_PILOT);
        all_crew.push_back(c);
        available_by_role[CREW_PILOT].push_back(c);
    }
    
    // Create co-pilots
    for (int i = 0; i < co_pilots; i++) {
        Crew* c = new Crew(id++, "CoPilot-" + to_string(i), CREW_CO_PILOT);
        all_crew.push_back(c);
        available_by_role[CREW_CO_PILOT].push_back(c);
    }
    
    // Create flight attendants
    for (int i = 0; i < attendants; i++) {
        Crew* c = new Crew(id++, "Attendant-" + to_string(i), CREW_FLIGHT_ATTENDANT);
        all_crew.push_back(c);
        available_by_role[CREW_FLIGHT_ATTENDANT].push_back(c);
    }
    
    // Create ground technicians
    for (int i = 0; i < technicians; i++) {
        Crew* c = new Crew(id++, "Technician-" + to_string(i), CREW_GROUND_TECHNICIAN);
        all_crew.push_back(c);
        available_by_role[CREW_GROUND_TECHNICIAN].push_back(c);
    }
    
    // Create baggage handlers
    for (int i = 0; i < handlers; i++) {
        Crew* c = new Crew(id++, "Handler-" + to_string(i), CREW_BAGGAGE_HANDLER);
        all_crew.push_back(c);
        available_by_role[CREW_BAGGAGE_HANDLER].push_back(c);
    }
    
    // Create gate agents
    for (int i = 0; i < agents; i++) {
        Crew* c = new Crew(id++, "Agent-" + to_string(i), CREW_GATE_AGENT);
        all_crew.push_back(c);
        available_by_role[CREW_GATE_AGENT].push_back(c);
    }
    
    // Create fuel technicians
    for (int i = 0; i < fuel_techs; i++) {
        Crew* c = new Crew(id++, "FuelTech-" + to_string(i), CREW_FUEL_TECHNICIAN);
        all_crew.push_back(c);
        available_by_role[CREW_FUEL_TECHNICIAN].push_back(c);
    }
    
    pthread_mutex_unlock(&manager_mutex);
}

Crew* CrewManager::assign_crew(CrewRole role, int flight_id, long long current_time) {
    pthread_mutex_lock(&manager_mutex);
    
    auto& available = available_by_role[role];
    
    // LRU: take from front (least recently used)
    for (auto it = available.begin(); it != available.end(); ++it) {
        Crew* c = *it;
        if (c->is_fit_for_duty() && c->assign_to_flight(flight_id, current_time)) {
            available.erase(it);
            pthread_mutex_unlock(&manager_mutex);
            return c;
        }
    }
    
    pthread_mutex_unlock(&manager_mutex);
    return nullptr;
}

void CrewManager::release_crew(Crew* crew, long long current_time) {
    if (crew == nullptr) return;
    
    pthread_mutex_lock(&manager_mutex);
    
    crew->release_from_duty(current_time);
    
    // Add back to end of list (most recently used)
    if (crew->is_fit_for_duty()) {
        available_by_role[crew->get_role()].push_back(crew);
    }
    
    pthread_mutex_unlock(&manager_mutex);
}

int CrewManager::get_available_count(CrewRole role) {
    pthread_mutex_lock(&manager_mutex);
    int count = available_by_role[role].size();
    pthread_mutex_unlock(&manager_mutex);
    return count;
}

vector<Crew*> CrewManager::get_fatigued_crew() {
    vector<Crew*> fatigued;
    
    pthread_mutex_lock(&manager_mutex);
    for (Crew* c : all_crew) {
        if (c->get_fatigue_score() > 0.7) {
            fatigued.push_back(c);
        }
    }
    pthread_mutex_unlock(&manager_mutex);
    
    return fatigued;
}

double CrewManager::get_average_fatigue() {
    if (all_crew.empty()) return 0.0;
    
    pthread_mutex_lock(&manager_mutex);
    double total = 0.0;
    for (Crew* c : all_crew) {
        total += c->get_fatigue_score();
    }
    pthread_mutex_unlock(&manager_mutex);
    
    return total / all_crew.size();
}
