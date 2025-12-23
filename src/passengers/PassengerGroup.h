#ifndef PASSENGER_GROUP_H
#define PASSENGER_GROUP_H

#include <string>

using namespace std;

enum PassengerStatus {
    PAX_ARRIVED,
    PAX_CHECKING_IN,
    PAX_CHECKED_IN,
    PAX_SECURITY,
    PAX_AT_GATE,
    PAX_BOARDING,
    PAX_ONBOARD,
    PAX_DEPLANING,
    PAX_DEPARTED
};

class PassengerGroup {
private:
    int group_id;
    int flight_id;
    int count;
    string destination;
    
    PassengerStatus status;
    bool has_connection;
    int connecting_flight_id;
    int connection_time_minutes;
    
    long long check_in_time;
    long long boarding_time;
    
public:
    PassengerGroup(int id, int flight, int pax_count);
    ~PassengerGroup();
    
    void set_status(PassengerStatus s);
    void set_connection(int conn_flight, int conn_time);
    void record_check_in(long long time);
    void record_boarding(long long time);
    
    // Getters
    int get_id() const { return group_id; }
    int get_flight_id() const { return flight_id; }
    int get_count() const { return count; }
    PassengerStatus get_status() const { return status; }
    bool is_connecting() const { return has_connection; }
    int get_connecting_flight() const { return connecting_flight_id; }
    int get_connection_time() const { return connection_time_minutes; }
    
    // Risk assessment
    bool is_at_risk_of_missing_connection(long long current_time);
};

#endif // PASSENGER_GROUP_H
