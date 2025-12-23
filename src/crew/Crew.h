#ifndef CREW_H
#define CREW_H

#include <string>
#include <pthread.h>

using namespace std;

enum CrewRole {
    CREW_PILOT,
    CREW_CO_PILOT,
    CREW_FLIGHT_ATTENDANT,
    CREW_GROUND_TECHNICIAN,
    CREW_BAGGAGE_HANDLER,
    CREW_GATE_AGENT,
    CREW_FUEL_TECHNICIAN
};

enum CrewStatus {
    CREW_AVAILABLE,
    CREW_ON_DUTY,
    CREW_ON_BREAK,
    CREW_FATIGUED,
    CREW_OFF_DUTY
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
    
    // Getters
    int get_id() const { return crew_id; }
    string get_name() const { return name; }
    CrewRole get_role() const { return role; }
    CrewStatus get_status() const { return status; }
    double get_fatigue_score() const { return fatigue_score; }
    int get_assigned_flight() const { return assigned_flight_id; }
    
    static string role_to_string(CrewRole role);
};

#endif // CREW_H
