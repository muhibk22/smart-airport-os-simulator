#include "WeatherEvent.h"

using namespace std;

WeatherEvent::WeatherEvent(int id, WeatherType t, WeatherSeverity s, 
                           long long start, long long duration) {
    event_id = id;
    type = t;
    severity = s;
    start_time = start;
    expected_duration = duration;
    end_time = start + duration;
    is_active = false;
    
    // Set defaults based on type/severity
    switch (s) {
        case SEV_NONE: visibility_km = 10.0; wind_speed_knots = 5.0; break;
        case SEV_MINOR: visibility_km = 5.0; wind_speed_knots = 20.0; break;
        case SEV_MODERATE: visibility_km = 2.0; wind_speed_knots = 35.0; break;
        case SEV_SEVERE: visibility_km = 0.5; wind_speed_knots = 50.0; break;
        case SEV_EXTREME: visibility_km = 0.1; wind_speed_knots = 75.0; break;
    }
}

WeatherEvent::~WeatherEvent() {}

void WeatherEvent::activate() { is_active = true; }
void WeatherEvent::deactivate() { is_active = false; }

bool WeatherEvent::check_expired(long long current_time) {
    if (current_time >= end_time) {
        is_active = false;
        return true;
    }
    return false;
}

int WeatherEvent::get_delay_minutes() {
    switch (severity) {
        case SEV_NONE: return 0;
        case SEV_MINOR: return 15;
        case SEV_MODERATE: return 45;
        case SEV_SEVERE: return 120;
        case SEV_EXTREME: return -1;  // Indefinite
        default: return 0;
    }
}

double WeatherEvent::get_capacity_reduction() {
    switch (severity) {
        case SEV_NONE: return 0.0;
        case SEV_MINOR: return 0.1;
        case SEV_MODERATE: return 0.3;
        case SEV_SEVERE: return 0.6;
        case SEV_EXTREME: return 1.0;  // Full stop
        default: return 0.0;
    }
}

bool WeatherEvent::requires_ground_stop() {
    return severity == SEV_EXTREME || 
           (type == WEATHER_THUNDERSTORM && severity >= SEV_SEVERE);
}

string WeatherEvent::type_to_string(WeatherType type) {
    switch (type) {
        case WEATHER_CLEAR: return "Clear";
        case WEATHER_RAIN: return "Rain";
        case WEATHER_SNOW: return "Snow";
        case WEATHER_FOG: return "Fog";
        case WEATHER_THUNDERSTORM: return "Thunderstorm";
        case WEATHER_ICE: return "Ice";
        default: return "Unknown";
    }
}

string WeatherEvent::severity_to_string(WeatherSeverity sev) {
    switch (sev) {
        case SEV_NONE: return "None";
        case SEV_MINOR: return "Minor";
        case SEV_MODERATE: return "Moderate";
        case SEV_SEVERE: return "Severe";
        case SEV_EXTREME: return "Extreme";
        default: return "Unknown";
    }
}
