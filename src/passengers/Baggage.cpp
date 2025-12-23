#include "Baggage.h"

using namespace std;

Baggage::Baggage(const string& rfid, int owner, int flight, double weight) {
    rfid_tag = rfid;
    owner_group_id = owner;
    origin_flight_id = flight;
    destination_flight_id = flight;
    status = BAG_CHECKED;
    weight_kg = weight;
    is_oversized = weight > 32.0;
    requires_transfer = false;
    check_time = 0;
    load_time = 0;
}

Baggage::~Baggage() {}

void Baggage::set_status(BaggageStatus s) { status = s; }

void Baggage::set_transfer(int dest_flight) {
    requires_transfer = true;
    destination_flight_id = dest_flight;
}

void Baggage::record_loaded(long long time) {
    load_time = time;
    status = BAG_LOADED;
}

string Baggage::status_to_string(BaggageStatus s) {
    switch (s) {
        case BAG_CHECKED: return "Checked";
        case BAG_SCREENED: return "Screened";
        case BAG_SORTED: return "Sorted";
        case BAG_LOADED: return "Loaded";
        case BAG_IN_TRANSIT: return "In Transit";
        case BAG_ARRIVED: return "Arrived";
        case BAG_CLAIMED: return "Claimed";
        case BAG_LOST: return "Lost";
        default: return "Unknown";
    }
}
