#include "Crew.h"
#include <algorithm>

using namespace std;

Crew::Crew(int id, const string& n, CrewRole r) {
    crew_id = id;
    name = n;
    role = r;
    status = CREW_AVAILABLE;
    assigned_flight_id = -1;
    fatigue_score = 0.0;
    duty_start_time = 0;
    total_duty_hours = 0;
    last_rest_time = 0;
    pthread_mutex_init(&crew_mutex, nullptr);
}

Crew::~Crew() {
    pthread_mutex_destroy(&crew_mutex);
}

bool Crew::assign_to_flight(int flight_id, long long current_time) {
    pthread_mutex_lock(&crew_mutex);
    
    if (status != CREW_AVAILABLE || !is_fit_for_duty()) {
        pthread_mutex_unlock(&crew_mutex);
        return false;
    }
    
    assigned_flight_id = flight_id;
    status = CREW_ON_DUTY;
    duty_start_time = current_time;
    
    pthread_mutex_unlock(&crew_mutex);
    return true;
}

void Crew::release_from_duty(long long current_time) {
    pthread_mutex_lock(&crew_mutex);
    
    if (status == CREW_ON_DUTY) {
        total_duty_hours += (current_time - duty_start_time);
        update_fatigue(current_time);
    }
    
    assigned_flight_id = -1;
    status = (fatigue_score > 0.7) ? CREW_FATIGUED : CREW_AVAILABLE;
    
    pthread_mutex_unlock(&crew_mutex);
}

void Crew::update_fatigue(long long current_time) {
    // Fatigue increases with duty time
    // Formula: fatigue += duty_hours / max_duty_hours
    long long duty_duration = current_time - duty_start_time;
    double fatigue_increase = duty_duration / 28800.0;  // 8 hours = max shift
    fatigue_score = min(1.0, fatigue_score + fatigue_increase);
}

void Crew::take_rest(long long duration) {
    pthread_mutex_lock(&crew_mutex);
    
    status = CREW_ON_BREAK;
    // Rest reduces fatigue
    double recovery = duration / 28800.0;  // 8 hours full recovery
    fatigue_score = max(0.0, fatigue_score - recovery);
    
    if (fatigue_score < 0.3) {
        status = CREW_AVAILABLE;
    }
    
    pthread_mutex_unlock(&crew_mutex);
}

bool Crew::is_fit_for_duty() {
    return fatigue_score < 0.7 && status != CREW_FATIGUED && status != CREW_OFF_DUTY;
}

string Crew::role_to_string(CrewRole role) {
    switch (role) {
        case CREW_PILOT: return "Pilot";
        case CREW_CO_PILOT: return "Co-Pilot";
        case CREW_FLIGHT_ATTENDANT: return "Flight Attendant";
        case CREW_GROUND_TECHNICIAN: return "Ground Technician";
        case CREW_BAGGAGE_HANDLER: return "Baggage Handler";
        case CREW_GATE_AGENT: return "Gate Agent";
        case CREW_FUEL_TECHNICIAN: return "Fuel Technician";
        default: return "Unknown";
    }
}
