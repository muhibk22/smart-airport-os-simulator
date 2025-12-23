#include "DataStore.h"
#include <algorithm>
#include <iostream>

using namespace std;

DataStore::DataStore(const string& output_dir) {
    output_directory = output_dir;
    next_record_id = 1;
    auto_flush = false;
    flush_threshold = 1000;
    pthread_mutex_init(&store_mutex, nullptr);
}

DataStore::~DataStore() {
    flush();
    pthread_mutex_destroy(&store_mutex);
}

void DataStore::add_record(RecordType type, const string& category, 
                           const string& description, double v1, double v2, double v3) {
    pthread_mutex_lock(&store_mutex);
    
    DataRecord record;
    record.record_id = next_record_id++;
    record.type = type;
    record.timestamp = time(nullptr);
    record.category = category;
    record.description = description;
    record.value1 = v1;
    record.value2 = v2;
    record.value3 = v3;
    
    records.push_back(record);
    
    // Auto-flush if threshold reached
    if (auto_flush && (int)records.size() >= flush_threshold) {
        flush();
    }
    
    pthread_mutex_unlock(&store_mutex);
}

void DataStore::add_record(const DataRecord& record) {
    pthread_mutex_lock(&store_mutex);
    records.push_back(record);
    pthread_mutex_unlock(&store_mutex);
}

vector<DataRecord> DataStore::get_records_by_type(RecordType type) {
    pthread_mutex_lock(&store_mutex);
    
    vector<DataRecord> result;
    for (const auto& rec : records) {
        if (rec.type == type) {
            result.push_back(rec);
        }
    }
    
    pthread_mutex_unlock(&store_mutex);
    return result;
}

vector<DataRecord> DataStore::get_records_by_category(const string& category) {
    pthread_mutex_lock(&store_mutex);
    
    vector<DataRecord> result;
    for (const auto& rec : records) {
        if (rec.category == category) {
            result.push_back(rec);
        }
    }
    
    pthread_mutex_unlock(&store_mutex);
    return result;
}

vector<DataRecord> DataStore::get_records_in_range(long long start_time, long long end_time) {
    pthread_mutex_lock(&store_mutex);
    
    vector<DataRecord> result;
    for (const auto& rec : records) {
        if (rec.timestamp >= start_time && rec.timestamp <= end_time) {
            result.push_back(rec);
        }
    }
    
    pthread_mutex_unlock(&store_mutex);
    return result;
}

bool DataStore::export_to_csv(const string& filename) {
    pthread_mutex_lock(&store_mutex);
    
    string full_path = output_directory + "/" + filename;
    ofstream file(full_path);
    
    if (!file.is_open()) {
        pthread_mutex_unlock(&store_mutex);
        cerr << "Error: Could not open file " << full_path << endl;
        return false;
    }
    
    // Write header
    file << DataRecord::get_csv_header() << endl;
    
    // Write records
    for (const auto& rec : records) {
        file << rec.to_csv() << endl;
    }
    
    file.close();
    pthread_mutex_unlock(&store_mutex);
    return true;
}

bool DataStore::export_by_type(RecordType type, const string& filename) {
    pthread_mutex_lock(&store_mutex);
    
    string full_path = output_directory + "/" + filename;
    ofstream file(full_path);
    
    if (!file.is_open()) {
        pthread_mutex_unlock(&store_mutex);
        return false;
    }
    
    file << DataRecord::get_csv_header() << endl;
    
    for (const auto& rec : records) {
        if (rec.type == type) {
            file << rec.to_csv() << endl;
        }
    }
    
    file.close();
    pthread_mutex_unlock(&store_mutex);
    return true;
}

void DataStore::flush() {
    // Export all records to a timestamped file
    time_t now = time(nullptr);
    string filename = "simulation_data_" + to_string(now) + ".csv";
    export_to_csv(filename);
}

void DataStore::clear() {
    pthread_mutex_lock(&store_mutex);
    records.clear();
    pthread_mutex_unlock(&store_mutex);
}

int DataStore::get_record_count_by_type(RecordType type) {
    pthread_mutex_lock(&store_mutex);
    
    int count = 0;
    for (const auto& rec : records) {
        if (rec.type == type) count++;
    }
    
    pthread_mutex_unlock(&store_mutex);
    return count;
}

void DataStore::set_auto_flush(bool enabled, int threshold) {
    pthread_mutex_lock(&store_mutex);
    auto_flush = enabled;
    flush_threshold = threshold;
    pthread_mutex_unlock(&store_mutex);
}

void DataStore::set_output_directory(const string& dir) {
    pthread_mutex_lock(&store_mutex);
    output_directory = dir;
    pthread_mutex_unlock(&store_mutex);
}
