#include "PassengerPipeline.h"

using namespace std;

PassengerPipeline::PassengerPipeline(int counters, int lanes) {
    checkin_counters = counters;
    security_lanes = lanes;
    pthread_mutex_init(&pipeline_mutex, nullptr);
    pthread_cond_init(&stage_ready, nullptr);
}

PassengerPipeline::~PassengerPipeline() {
    pthread_mutex_destroy(&pipeline_mutex);
    pthread_cond_destroy(&stage_ready);
    for (PassengerGroup* g : all_groups) delete g;
}

void PassengerPipeline::add_arrival(PassengerGroup* group) {
    pthread_mutex_lock(&pipeline_mutex);
    all_groups.push_back(group);
    arrival_queue.push(group);
    group->set_status(PAX_ARRIVED);
    pthread_mutex_unlock(&pipeline_mutex);
}

void PassengerPipeline::process_checkin(long long current_time) {
    pthread_mutex_lock(&pipeline_mutex);
    
    int processed = 0;
    while (!arrival_queue.empty() && processed < checkin_counters) {
        PassengerGroup* g = arrival_queue.front();
        arrival_queue.pop();
        g->record_check_in(current_time);
        security_queue.push(g);
        processed++;
    }
    
    pthread_mutex_unlock(&pipeline_mutex);
}

void PassengerPipeline::process_security(long long current_time) {
    pthread_mutex_lock(&pipeline_mutex);
    
    int processed = 0;
    while (!security_queue.empty() && processed < security_lanes) {
        PassengerGroup* g = security_queue.front();
        security_queue.pop();
        g->set_status(PAX_AT_GATE);
        gate_queue.push(g);
        processed++;
    }
    
    pthread_mutex_unlock(&pipeline_mutex);
}

void PassengerPipeline::process_gate(int flight_id, long long current_time) {
    pthread_mutex_lock(&pipeline_mutex);
    
    queue<PassengerGroup*> temp;
    while (!gate_queue.empty()) {
        PassengerGroup* g = gate_queue.front();
        gate_queue.pop();
        
        if (g->get_flight_id() == flight_id) {
            g->set_status(PAX_BOARDING);
            boarding_queue.push(g);
        } else {
            temp.push(g);
        }
    }
    gate_queue = temp;
    
    pthread_mutex_unlock(&pipeline_mutex);
}

void PassengerPipeline::process_boarding(int flight_id, long long current_time) {
    pthread_mutex_lock(&pipeline_mutex);
    
    queue<PassengerGroup*> temp;
    while (!boarding_queue.empty()) {
        PassengerGroup* g = boarding_queue.front();
        boarding_queue.pop();
        
        if (g->get_flight_id() == flight_id) {
            g->record_boarding(current_time);
        } else {
            temp.push(g);
        }
    }
    boarding_queue = temp;
    
    pthread_mutex_unlock(&pipeline_mutex);
}

vector<PassengerGroup*> PassengerPipeline::get_boarding_ready(int flight_id) {
    vector<PassengerGroup*> ready;
    pthread_mutex_lock(&pipeline_mutex);
    
    for (PassengerGroup* g : all_groups) {
        if (g->get_flight_id() == flight_id && g->get_status() == PAX_BOARDING) {
            ready.push_back(g);
        }
    }
    
    pthread_mutex_unlock(&pipeline_mutex);
    return ready;
}

vector<PassengerGroup*> PassengerPipeline::get_at_risk_passengers(long long current_time) {
    vector<PassengerGroup*> at_risk;
    pthread_mutex_lock(&pipeline_mutex);
    
    for (PassengerGroup* g : all_groups) {
        if (g->is_at_risk_of_missing_connection(current_time)) {
            at_risk.push_back(g);
        }
    }
    
    pthread_mutex_unlock(&pipeline_mutex);
    return at_risk;
}

int PassengerPipeline::get_queue_length(PassengerStatus stage) {
    pthread_mutex_lock(&pipeline_mutex);
    int len = 0;
    switch (stage) {
        case PAX_ARRIVED: len = arrival_queue.size(); break;
        case PAX_CHECKED_IN: len = security_queue.size(); break;
        case PAX_AT_GATE: len = gate_queue.size(); break;
        case PAX_BOARDING: len = boarding_queue.size(); break;
        default: break;
    }
    pthread_mutex_unlock(&pipeline_mutex);
    return len;
}

int PassengerPipeline::get_total_passengers() {
    pthread_mutex_lock(&pipeline_mutex);
    int total = 0;
    for (PassengerGroup* g : all_groups) {
        total += g->get_count();
    }
    pthread_mutex_unlock(&pipeline_mutex);
    return total;
}
