#ifndef WEATHER_EVENT_H
#define WEATHER_EVENT_H

#include <string>
#include <pthread.h>

using namespace std;

enum WeatherType {
    WEATHER_CLEAR,
    WEATHER_RAIN,
    WEATHER_SNOW,
    WEATHER_FOG,
    WEATHER_THUNDERSTORM,
    WEATHER_ICE
};

enum WeatherSeverity {
    SEV_NONE,
    SEV_MINOR,      // Small delays
    SEV_MODERATE,   // Significant delays
    SEV_SEVERE,     // Possible closures
    SEV_EXTREME     // Airport closure
};

class WeatherEvent {
private:
    int event_id;
    WeatherType type;
    WeatherSeverity severity;
    
    long long start_time;
    long long expected_duration;
    long long end_time;
    
    double visibility_km;       // 0-10+ km
    double wind_speed_knots;    // 0-100+
    double precipitation_mm;    // mm/hour
    
    bool is_active;
    
public:
    WeatherEvent(int id, WeatherType t, WeatherSeverity s, long long start, long long duration);
    ~WeatherEvent();
    
    // Lifecycle
    void activate();
    void deactivate();
    bool check_expired(long long current_time);
    
    // Impact calculations
    int get_delay_minutes();
    double get_capacity_reduction();  // 0.0-1.0 (how much to reduce operations)
    bool requires_ground_stop();
    
    // Getters
    int get_id() const { return event_id; }
    WeatherType get_type() const { return type; }
    WeatherSeverity get_severity() const { return severity; }
    bool get_active() const { return is_active; }
    double get_visibility() const { return visibility_km; }
    double get_wind_speed() const { return wind_speed_knots; }
    
    static string type_to_string(WeatherType type);
    static string severity_to_string(WeatherSeverity sev);
};

#endif // WEATHER_EVENT_H
