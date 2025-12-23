#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "DataRecord.h"
#include <vector>
#include <fstream>
#include <pthread.h>
#include <unordered_map>
#include <chrono>
#include <thread>

using namespace std;

// DataStore manages simulation data persistence
// Implements multi-level access times and MVCC for versioning

// Data tier access times (in simulation time units)
// From spec: L1=1, L2=5, L3=20, L4=100 time units
enum DataTier {
    DATA_TIER_L1 = 1,     // Critical Real-Time Data (runway, gates, emergencies)
    DATA_TIER_L2 = 5,     // Active Operations Data (schedules, weather)
    DATA_TIER_L3 = 20,    // System Data (manifests, maintenance)
    DATA_TIER_L4 = 100    // Historical Data (past records, audit logs)
};

class DataStore {
private:
    vector<DataRecord> records;
    int next_record_id;
    pthread_mutex_t store_mutex;
    
    string output_directory;
    bool auto_flush;
    int flush_threshold;
    
    // Version tracking: record_id -> list of all versions
    unordered_map<int, vector<int>> record_versions;
    
    // Access statistics per tier
    int tier_access_count[4];
    long long total_access_delay[4];
    
    // Simulate access delay based on data tier
    void simulate_access_delay(DataTier tier);
    
public:
    DataStore(const string& output_dir = "data/output");
    ~DataStore();
    
    // Add records
    void add_record(RecordType type, const string& category, const string& description,
                   double v1 = 0, double v2 = 0, double v3 = 0);
    void add_record(const DataRecord& record);
    
    // Add record with tier-based access simulation
    void add_record_tiered(RecordType type, const string& category, const string& description,
                          DataTier tier, double v1 = 0, double v2 = 0, double v3 = 0);
    
    // Query records with access time simulation
    vector<DataRecord> get_records_by_type(RecordType type);
    vector<DataRecord> get_records_by_category(const string& category);
    vector<DataRecord> get_records_in_range(long long start_time, long long end_time);
    
    // Tiered access with delay simulation
    vector<DataRecord> access_with_delay(RecordType type, DataTier tier);
    DataRecord* access_record(int record_id, DataTier tier);
    
    // MVCC Version Control
    int update_record(int record_id, const string& new_description, 
                     double v1, double v2, double v3);
    vector<DataRecord> get_record_history(int record_id);
    DataRecord* get_record_version(int record_id, int version);
    int get_current_version(int record_id);
    
    // Export
    bool export_to_csv(const string& filename);
    bool export_by_type(RecordType type, const string& filename);
    
    // Persistence
    void flush();
    void clear();
    
    // Statistics
    int get_record_count() const { return records.size(); }
    int get_record_count_by_type(RecordType type);
    double get_average_access_time(DataTier tier);
    int get_tier_access_count(DataTier tier);
    
    // Configuration
    void set_auto_flush(bool enabled, int threshold = 1000);
    void set_output_directory(const string& dir);
};

#endif // DATA_STORE_H
