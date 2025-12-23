#ifndef BAGGAGE_SYSTEM_H
#define BAGGAGE_SYSTEM_H

#include "Baggage.h"
#include <vector>
#include <unordered_map>
#include <queue>
#include <pthread.h>

using namespace std;

class BaggageSystem {
private:
    vector<Baggage*> all_bags;
    unordered_map<int, queue<Baggage*>> flight_bags;  // flight_id -> bags for that flight
    queue<Baggage*> transfer_queue;
    
    int lost_count;
    int processed_count;
    
    pthread_mutex_t system_mutex;
    
public:
    BaggageSystem();
    ~BaggageSystem();
    
    // Add baggage
    void check_bag(Baggage* bag);
    
    // Processing stages
    void screen_bags();
    void sort_bags();
    void load_bags_for_flight(int flight_id, long long time);
    
    // Transfers
    void process_transfers(long long current_time);
    
    // Arrivals
    void unload_flight(int flight_id);
    void process_claims(int flight_id);
    
    // Get bags for flight
    vector<Baggage*> get_bags_for_flight(int flight_id);
    
    // Statistics
    int get_total_bags() const { return all_bags.size(); }
    int get_lost_bags() const { return lost_count; }
    int get_processed() const { return processed_count; }
    double get_on_time_rate() const;
};

#endif // BAGGAGE_SYSTEM_H
