#include "Event.h"

Event::Event(EventType t, long long time, int pri) 
    : type(t), event_time(time), priority(pri) {
    // Set description based on type
    switch(type) {
        case FLIGHT_ARRIVAL: description = "FlightArrival"; break;
        case FLIGHT_DEPARTURE: description = "FlightDeparture"; break;
        case SERVICE_START: description = "ServiceStart"; break;
        case SERVICE_END: description = "ServiceEnd"; break;
        case RESOURCE_REQUEST: description = "ResourceRequest"; break;
        case RESOURCE_RELEASE: description = "ResourceRelease"; break;
        case PAGE_FAULT: description = "PageFault"; break;
        case WEATHER_EVENT: description = "WeatherEvent"; break;
        case EMERGENCY_EVENT: description = "EmergencyEvent"; break;
        case CRISIS_RESOLVED: description = "CrisisResolved"; break;
        default: description = "Unknown"; break;
    }
}
