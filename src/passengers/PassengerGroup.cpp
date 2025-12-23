#include "PassengerGroup.h"

using namespace std;

PassengerGroup::PassengerGroup(int id, int flight, int pax_count) {
    group_id = id;
    flight_id = flight;
    count = pax_count;
    status = PAX_ARRIVED;
    has_connection = false;
    connecting_flight_id = -1;
    connection_time_minutes = 0;
    check_in_time = 0;
    boarding_time = 0;
}

PassengerGroup::~PassengerGroup() {}

void PassengerGroup::set_status(PassengerStatus s) { status = s; }

void PassengerGroup::set_connection(int conn_flight, int conn_time) {
    has_connection = true;
    connecting_flight_id = conn_flight;
    connection_time_minutes = conn_time;
}

void PassengerGroup::record_check_in(long long time) {
    check_in_time = time;
    status = PAX_CHECKED_IN;
}

void PassengerGroup::record_boarding(long long time) {
    boarding_time = time;
    status = PAX_ONBOARD;
}

bool PassengerGroup::is_at_risk_of_missing_connection(long long current_time) {
    if (!has_connection) return false;
    // Risk if less than 45 minutes to connection
    long long time_to_connection = connection_time_minutes * 60;
    return time_to_connection < 2700;  // 45 minutes
}
