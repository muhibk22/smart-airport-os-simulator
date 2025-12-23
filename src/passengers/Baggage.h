#ifndef BAGGAGE_H
#define BAGGAGE_H

#include <string>

using namespace std;

enum BaggageStatus {
    BAG_CHECKED,
    BAG_SCREENED,
    BAG_SORTED,
    BAG_LOADED,
    BAG_IN_TRANSIT,
    BAG_ARRIVED,
    BAG_CLAIMED,
    BAG_LOST
};

class Baggage {
private:
    string rfid_tag;
    int owner_group_id;
    int origin_flight_id;
    int destination_flight_id;
    
    BaggageStatus status;
    double weight_kg;
    bool is_oversized;
    bool requires_transfer;
    
    long long check_time;
    long long load_time;
    
public:
    Baggage(const string& rfid, int owner, int flight, double weight);
    ~Baggage();
    
    void set_status(BaggageStatus s);
    void set_transfer(int dest_flight);
    void record_loaded(long long time);
    
    // Getters
    string get_rfid() const { return rfid_tag; }
    int get_owner() const { return owner_group_id; }
    int get_flight() const { return origin_flight_id; }
    BaggageStatus get_status() const { return status; }
    bool needs_transfer() const { return requires_transfer; }
    int get_transfer_flight() const { return destination_flight_id; }
    double get_weight() const { return weight_kg; }
    
    static string status_to_string(BaggageStatus s);
};

#endif // BAGGAGE_H
