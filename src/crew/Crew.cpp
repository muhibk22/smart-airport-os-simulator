#include "Crew.h"
#include <algorithm>
#include <chrono>

using namespace std;

// Static random generator for skill level assignment
mt19937 Crew::rng;
bool Crew::rng_initialized = false;

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
    
    // Initialize RNG if not already done
    if (!rng_initialized) {
        rng.seed(chrono::steady_clock::now().time_since_epoch().count());
        rng_initialized = true;
    }
    
    // Randomly assign skill level (weighted distribution)
    // 30% Novice, 40% Standard, 20% Experienced, 10% Expert
    uniform_int_distribution<int> dist(1, 100);
    int roll = dist(rng);
    
    if (roll <= 30) {
        skill_level = SKILL_NOVICE;
        skill_multiplier = 0.80;  // -20% efficiency
    } else if (roll <= 70) {
        skill_level = SKILL_STANDARD;
        skill_multiplier = 1.00;  // Normal efficiency
    } else if (roll <= 90) {
        skill_level = SKILL_EXPERIENCED;
        skill_multiplier = 1.10;  // +10% efficiency
    } else {
        skill_level = SKILL_EXPERT;
        skill_multiplier = 1.20;  // +20% efficiency
    }
    
    experience_points = 0;
    
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
        case CREW_ATC: return "Air Traffic Controller";  // REQ-5
        default: return "Unknown";
    }
}

// ============================================================================
// SKILL LEVEL MANAGEMENT
// Implements ±20% efficiency variation as per spec
// ============================================================================

void Crew::add_experience(int points) {
    pthread_mutex_lock(&crew_mutex);
    
    experience_points += points;
    
    // Auto-upgrade skill level based on experience thresholds
    if (experience_points >= 1000 && skill_level < SKILL_EXPERT) {
        skill_level = SKILL_EXPERT;
        skill_multiplier = 1.20;
    } else if (experience_points >= 500 && skill_level < SKILL_EXPERIENCED) {
        skill_level = SKILL_EXPERIENCED;
        skill_multiplier = 1.10;
    } else if (experience_points >= 100 && skill_level < SKILL_STANDARD) {
        skill_level = SKILL_STANDARD;
        skill_multiplier = 1.00;
    }
    
    pthread_mutex_unlock(&crew_mutex);
}

void Crew::set_skill_level(SkillLevel level) {
    pthread_mutex_lock(&crew_mutex);
    
    skill_level = level;
    
    switch (level) {
        case SKILL_NOVICE:
            skill_multiplier = 0.80;
            break;
        case SKILL_STANDARD:
            skill_multiplier = 1.00;
            break;
        case SKILL_EXPERIENCED:
            skill_multiplier = 1.10;
            break;
        case SKILL_EXPERT:
            skill_multiplier = 1.20;
            break;
    }
    
    pthread_mutex_unlock(&crew_mutex);
}

double Crew::calculate_service_duration(int base_duration) const {
    // Higher efficiency = shorter duration
    // Novice (0.80x efficiency) = takes 25% longer (1/0.80 = 1.25)
    // Expert (1.20x efficiency) = takes 17% less time (1/1.20 = 0.83)
    
    double adjusted_duration = base_duration / skill_multiplier;
    return adjusted_duration;
}

const char* Crew::skill_level_to_string(SkillLevel level) {
    switch (level) {
        case SKILL_NOVICE: return "Novice (80%)";
        case SKILL_STANDARD: return "Standard (100%)";
        case SKILL_EXPERIENCED: return "Experienced (110%)";
        case SKILL_EXPERT: return "Expert (120%)";
        default: return "Unknown";
    }
}
