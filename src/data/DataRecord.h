#ifndef DATA_RECORD_H
#define DATA_RECORD_H

#include <string>
#include <ctime>

using namespace std;

// DataRecord represents a single simulation data entry for logging and analysis

enum RecordType {
    REC_FLIGHT,
    REC_RESOURCE,
    REC_SCHEDULER,
    REC_MEMORY,
    REC_CRISIS,
    REC_FINANCE
};

struct DataRecord {
    int record_id;
    RecordType type;
    long long timestamp;
    string category;
    string description;
    double value1;
    double value2;
    double value3;
    string metadata;
    
    DataRecord() : record_id(0), type(REC_FLIGHT), timestamp(0), 
                   value1(0), value2(0), value3(0) {}
    
    DataRecord(int id, RecordType t, long long ts, const string& cat, 
               const string& desc, double v1 = 0, double v2 = 0, double v3 = 0)
        : record_id(id), type(t), timestamp(ts), category(cat), 
          description(desc), value1(v1), value2(v2), value3(v3) {}
    
    // Convert to CSV format
    string to_csv() const {
        return to_string(record_id) + "," + to_string(type) + "," + 
               to_string(timestamp) + "," + category + "," + description + "," +
               to_string(value1) + "," + to_string(value2) + "," + to_string(value3);
    }
    
    static string get_csv_header() {
        return "record_id,type,timestamp,category,description,value1,value2,value3";
    }
    
    static string type_to_string(RecordType t) {
        switch (t) {
            case REC_FLIGHT: return "FLIGHT";
            case REC_RESOURCE: return "RESOURCE";
            case REC_SCHEDULER: return "SCHEDULER";
            case REC_MEMORY: return "MEMORY";
            case REC_CRISIS: return "CRISIS";
            case REC_FINANCE: return "FINANCE";
            default: return "UNKNOWN";
        }
    }
};

#endif // DATA_RECORD_H
