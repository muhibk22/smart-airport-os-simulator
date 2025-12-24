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

// REQ-3: Special passenger flags (bitmask)
enum PassengerFlags {
    PAX_FLAG_NONE = 0,
    PAX_FLAG_VIP = 1,                  // 0.5x processing time
    PAX_FLAG_DISABLED = 2,             // +5 min assistance delay
    PAX_FLAG_UNACCOMPANIED_MINOR = 4   // Requires escort
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
    
    int passenger_flags;  // REQ-3: Bitmask of PassengerFlags

    
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
    
    // REQ-3: VIP/Special passenger handling
    void set_flags(int flags) { passenger_flags = flags; }
    int get_flags() const { return passenger_flags; }
    bool is_vip() const { return (passenger_flags & PAX_FLAG_VIP) != 0; }
    bool is_disabled() const { return (passenger_flags & PAX_FLAG_DISABLED) != 0; }
    bool is_unaccompanied_minor() const { return (passenger_flags & PAX_FLAG_UNACCOMPANIED_MINOR) != 0; }
    double get_processing_multiplier() const { return is_vip() ? 0.5 : 1.0; }  // VIP = 50% faster
    int get_assistance_delay() const { return is_disabled() ? 300 : 0; }  // +5 min = 300 time units
};

#endif // PASSENGER_GROUP_H
