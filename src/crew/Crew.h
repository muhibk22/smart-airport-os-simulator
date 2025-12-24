#ifndef CREW_H
#define CREW_H

#include <string>
#include <pthread.h>
#include <random>

using namespace std;

enum CrewRole {
    CREW_PILOT,
    CREW_CO_PILOT,
    CREW_FLIGHT_ATTENDANT,
    CREW_GROUND_TECHNICIAN,
    CREW_BAGGAGE_HANDLER,
    CREW_GATE_AGENT,
    CREW_FUEL_TECHNICIAN,
    CREW_ATC                // REQ-5: Air Traffic Controller (8-hour shifts)
};

enum CrewStatus {
    CREW_AVAILABLE,
    CREW_ON_DUTY,
    CREW_ON_BREAK,
    CREW_FATIGUED,
    CREW_OFF_DUTY
};

// Crew Skill Levels (affects efficiency ±20% as per spec)
enum SkillLevel {
    SKILL_NOVICE,       // 0.80x efficiency
    SKILL_STANDARD,     // 1.00x efficiency
    SKILL_EXPERIENCED,  // 1.10x efficiency
    SKILL_EXPERT        // 1.20x efficiency
};

class Crew {
private:
    int crew_id;
    string name;
    CrewRole role;
    CrewStatus status;
    
    int assigned_flight_id;
    double fatigue_score;       // 0.0 (rested) to 1.0 (exhausted)
    long long duty_start_time;
    long long total_duty_hours;
    long long last_rest_time;
    
    // Skill level system (±20% efficiency as per spec)
    SkillLevel skill_level;
    double skill_multiplier;    // 0.80 to 1.20 range
    int experience_points;      // Accumulated experience
    
    // Static random generator for skill assignment
    static mt19937 rng;
    static bool rng_initialized;
    
    pthread_mutex_t crew_mutex;
    
public:
    Crew(int id, const string& n, CrewRole r);
    ~Crew();
    
    // Assignment
    bool assign_to_flight(int flight_id, long long current_time);
    void release_from_duty(long long current_time);
    
    // Fatigue management
    void update_fatigue(long long current_time);
    void take_rest(long long duration);
    bool is_fit_for_duty();
    
    // Skill level management
    SkillLevel get_skill_level() const { return skill_level; }
    double get_efficiency_multiplier() const { return skill_multiplier; }
    void add_experience(int points);       // Gain experience over time
    void set_skill_level(SkillLevel level);
    double calculate_service_duration(int base_duration) const;
    static const char* skill_level_to_string(SkillLevel level);
    
    // Getters
    int get_id() const { return crew_id; }
    string get_name() const { return name; }
    CrewRole get_role() const { return role; }
    CrewStatus get_status() const { return status; }
    double get_fatigue_score() const { return fatigue_score; }
    int get_assigned_flight() const { return assigned_flight_id; }
    int get_experience_points() const { return experience_points; }
    
    static string role_to_string(CrewRole role);
};

#endif // CREW_H
