#include "BaggageSystem.h"

using namespace std;

BaggageSystem::BaggageSystem() {
    lost_count = 0;
    processed_count = 0;
    pthread_mutex_init(&system_mutex, nullptr);
}

BaggageSystem::~BaggageSystem() {
    pthread_mutex_destroy(&system_mutex);
    for (Baggage* b : all_bags) delete b;
}

void BaggageSystem::check_bag(Baggage* bag) {
    pthread_mutex_lock(&system_mutex);
    all_bags.push_back(bag);
    flight_bags[bag->get_flight()].push(bag);
    pthread_mutex_unlock(&system_mutex);
}

void BaggageSystem::screen_bags() {
    pthread_mutex_lock(&system_mutex);
    for (Baggage* b : all_bags) {
        if (b->get_status() == BAG_CHECKED) {
            b->set_status(BAG_SCREENED);
        }
    }
    pthread_mutex_unlock(&system_mutex);
}

void BaggageSystem::sort_bags() {
    pthread_mutex_lock(&system_mutex);
    for (Baggage* b : all_bags) {
        if (b->get_status() == BAG_SCREENED) {
            b->set_status(BAG_SORTED);
            if (b->needs_transfer()) {
                transfer_queue.push(b);
            }
        }
    }
    pthread_mutex_unlock(&system_mutex);
}

void BaggageSystem::load_bags_for_flight(int flight_id, long long time) {
    pthread_mutex_lock(&system_mutex);
    
    auto it = flight_bags.find(flight_id);
    if (it != flight_bags.end()) {
        while (!it->second.empty()) {
            Baggage* b = it->second.front();
            it->second.pop();
            if (b->get_status() == BAG_SORTED) {
                b->record_loaded(time);
                processed_count++;
            }
        }
    }
    
    pthread_mutex_unlock(&system_mutex);
}

void BaggageSystem::process_transfers(long long current_time) {
    pthread_mutex_lock(&system_mutex);
    
    queue<Baggage*> temp;
    while (!transfer_queue.empty()) {
        Baggage* b = transfer_queue.front();
        transfer_queue.pop();
        
        // Move to destination flight queue
        int dest = b->get_transfer_flight();
        flight_bags[dest].push(b);
        b->set_status(BAG_IN_TRANSIT);
    }
    
    pthread_mutex_unlock(&system_mutex);
}

void BaggageSystem::unload_flight(int flight_id) {
    pthread_mutex_lock(&system_mutex);
    for (Baggage* b : all_bags) {
        if (b->get_flight() == flight_id && b->get_status() == BAG_LOADED) {
            b->set_status(BAG_ARRIVED);
        }
    }
    pthread_mutex_unlock(&system_mutex);
}

void BaggageSystem::process_claims(int flight_id) {
    pthread_mutex_lock(&system_mutex);
    for (Baggage* b : all_bags) {
        if (b->get_flight() == flight_id && b->get_status() == BAG_ARRIVED) {
            b->set_status(BAG_CLAIMED);
        }
    }
    pthread_mutex_unlock(&system_mutex);
}

vector<Baggage*> BaggageSystem::get_bags_for_flight(int flight_id) {
    vector<Baggage*> result;
    pthread_mutex_lock(&system_mutex);
    for (Baggage* b : all_bags) {
        if (b->get_flight() == flight_id) {
            result.push_back(b);
        }
    }
    pthread_mutex_unlock(&system_mutex);
    return result;
}

double BaggageSystem::get_on_time_rate() const {
    if (all_bags.empty()) return 1.0;
    return 1.0 - ((double)lost_count / all_bags.size());
}
