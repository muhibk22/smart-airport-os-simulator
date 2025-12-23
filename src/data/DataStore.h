#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "DataRecord.h"
#include <vector>
#include <fstream>
#include <pthread.h>

using namespace std;

// DataStore manages simulation data persistence
// Stores records in memory and can export to CSV

class DataStore {
private:
    vector<DataRecord> records;
    int next_record_id;
    pthread_mutex_t store_mutex;
    
    string output_directory;
    bool auto_flush;
    int flush_threshold;
    
public:
    DataStore(const string& output_dir = "data/output");
    ~DataStore();
    
    // Add records
    void add_record(RecordType type, const string& category, const string& description,
                   double v1 = 0, double v2 = 0, double v3 = 0);
    void add_record(const DataRecord& record);
    
    // Query records
    vector<DataRecord> get_records_by_type(RecordType type);
    vector<DataRecord> get_records_by_category(const string& category);
    vector<DataRecord> get_records_in_range(long long start_time, long long end_time);
    
    // Export
    bool export_to_csv(const string& filename);
    bool export_by_type(RecordType type, const string& filename);
    
    // Persistence
    void flush();
    void clear();
    
    // Statistics
    int get_record_count() const { return records.size(); }
    int get_record_count_by_type(RecordType type);
    
    // Configuration
    void set_auto_flush(bool enabled, int threshold = 1000);
    void set_output_directory(const string& dir);
};

#endif // DATA_STORE_H
