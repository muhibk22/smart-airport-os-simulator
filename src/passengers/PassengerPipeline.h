#ifndef PASSENGER_PIPELINE_H
#define PASSENGER_PIPELINE_H

#include "PassengerGroup.h"
#include <vector>
#include <queue>
#include <pthread.h>

using namespace std;

// PassengerPipeline implements passenger flow through airport stages
// Stages: Arrival -> Check-in -> Security -> Gate -> Boarding

class PassengerPipeline {
private:
    queue<PassengerGroup*> arrival_queue;
    queue<PassengerGroup*> checkin_queue;
    queue<PassengerGroup*> security_queue;
    queue<PassengerGroup*> gate_queue;
    queue<PassengerGroup*> boarding_queue;
    
    vector<PassengerGroup*> all_groups;
    
    int checkin_counters;
    int security_lanes;
    
    pthread_mutex_t pipeline_mutex;
    pthread_cond_t stage_ready;
    
public:
    PassengerPipeline(int counters, int lanes);
    ~PassengerPipeline();
    
    // Add passengers to pipeline
    void add_arrival(PassengerGroup* group);
    
    // Process stage (move to next)
    void process_checkin(long long current_time);
    void process_security(long long current_time);
    void process_gate(int flight_id, long long current_time);
    void process_boarding(int flight_id, long long current_time);
    
    // Get passengers ready for boarding
    vector<PassengerGroup*> get_boarding_ready(int flight_id);
    
    // Get at-risk passengers
    vector<PassengerGroup*> get_at_risk_passengers(long long current_time);
    
    // Statistics
    int get_queue_length(PassengerStatus stage);
    int get_total_passengers();
};

#endif // PASSENGER_PIPELINE_H
