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
    
    // Initialize tier access statistics
    for (int i = 0; i < 4; i++) {
        tier_access_count[i] = 0;
        total_access_delay[i] = 0;
    }
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

// ============================================================================
// TIER-BASED ACCESS TIME SIMULATION
// From spec: L1=1, L2=5, L3=20, L4=100 time units
// ============================================================================

void DataStore::simulate_access_delay(DataTier tier) {
    // Convert tier enum value to array index (0-3)
    int tier_index = 0;
    int delay_ms = 0;
    
    switch (tier) {
        case DATA_TIER_L1:
            tier_index = 0;
            delay_ms = 1;    // 1 time unit = 1ms simulation
            break;
        case DATA_TIER_L2:
            tier_index = 1;
            delay_ms = 5;    // 5 time units = 5ms simulation
            break;
        case DATA_TIER_L3:
            tier_index = 2;
            delay_ms = 20;   // 20 time units = 20ms simulation
            break;
        case DATA_TIER_L4:
            tier_index = 3;
            delay_ms = 100;  // 100 time units = 100ms simulation
            break;
    }
    
    // Simulate the access delay
    this_thread::sleep_for(chrono::milliseconds(delay_ms));
    
    // Track statistics (already holding mutex from caller)
    tier_access_count[tier_index]++;
    total_access_delay[tier_index] += delay_ms;
}

void DataStore::add_record_tiered(RecordType type, const string& category, 
                                   const string& description, DataTier tier,
                                   double v1, double v2, double v3) {
    pthread_mutex_lock(&store_mutex);
    
    // Simulate access delay for the tier
    simulate_access_delay(tier);
    
    DataRecord record;
    record.record_id = next_record_id++;
    record.type = type;
    record.timestamp = time(nullptr);
    record.category = category;
    record.description = description;
    record.value1 = v1;
    record.value2 = v2;
    record.value3 = v3;
    record.version = 1;
    record.previous_version_id = -1;
    record.is_current = true;
    record.created_at = time(nullptr);
    
    records.push_back(record);
    record_versions[record.record_id].push_back(records.size() - 1);
    
    if (auto_flush && (int)records.size() >= flush_threshold) {
        flush();
    }
    
    pthread_mutex_unlock(&store_mutex);
}

vector<DataRecord> DataStore::access_with_delay(RecordType type, DataTier tier) {
    pthread_mutex_lock(&store_mutex);
    
    // Simulate access delay for the tier
    simulate_access_delay(tier);
    
    vector<DataRecord> result;
    for (const auto& rec : records) {
        if (rec.type == type && rec.is_current) {
            result.push_back(rec);
        }
    }
    
    pthread_mutex_unlock(&store_mutex);
    return result;
}

DataRecord* DataStore::access_record(int record_id, DataTier tier) {
    pthread_mutex_lock(&store_mutex);
    
    // Simulate access delay for the tier
    simulate_access_delay(tier);
    
    DataRecord* result = nullptr;
    for (auto& rec : records) {
        if (rec.record_id == record_id && rec.is_current) {
            result = &rec;
            break;
        }
    }
    
    pthread_mutex_unlock(&store_mutex);
    return result;
}

// ============================================================================
// MVCC VERSION CONTROL
// Implements multi-version concurrency control for data consistency
// ============================================================================

int DataStore::update_record(int record_id, const string& new_description,
                              double v1, double v2, double v3) {
    pthread_mutex_lock(&store_mutex);
    
    // Find current version
    DataRecord* current = nullptr;
    int current_index = -1;
    
    for (int i = 0; i < (int)records.size(); i++) {
        if (records[i].record_id == record_id && records[i].is_current) {
            current = &records[i];
            current_index = i;
            break;
        }
    }
    
    if (current == nullptr) {
        pthread_mutex_unlock(&store_mutex);
        return -1;  // Record not found
    }
    
    // Create new version
    DataRecord new_version;
    new_version.record_id = record_id;
    new_version.type = current->type;
    new_version.timestamp = time(nullptr);
    new_version.category = current->category;
    new_version.description = new_description;
    new_version.value1 = v1;
    new_version.value2 = v2;
    new_version.value3 = v3;
    new_version.metadata = current->metadata;
    new_version.version = current->version + 1;
    new_version.previous_version_id = current_index;
    new_version.is_current = true;
    new_version.created_at = time(nullptr);
    
    // Mark old version as not current
    current->is_current = false;
    
    // Add new version
    records.push_back(new_version);
    record_versions[record_id].push_back(records.size() - 1);
    
    int new_version_num = new_version.version;
    
    pthread_mutex_unlock(&store_mutex);
    return new_version_num;
}

vector<DataRecord> DataStore::get_record_history(int record_id) {
    pthread_mutex_lock(&store_mutex);
    
    vector<DataRecord> history;
    
    if (record_versions.find(record_id) != record_versions.end()) {
        for (int idx : record_versions[record_id]) {
            if (idx >= 0 && idx < (int)records.size()) {
                history.push_back(records[idx]);
            }
        }
    }
    
    pthread_mutex_unlock(&store_mutex);
    return history;
}

DataRecord* DataStore::get_record_version(int record_id, int version) {
    pthread_mutex_lock(&store_mutex);
    
    DataRecord* result = nullptr;
    
    for (auto& rec : records) {
        if (rec.record_id == record_id && rec.version == version) {
            result = &rec;
            break;
        }
    }
    
    pthread_mutex_unlock(&store_mutex);
    return result;
}

int DataStore::get_current_version(int record_id) {
    pthread_mutex_lock(&store_mutex);
    
    int version = -1;
    
    for (const auto& rec : records) {
        if (rec.record_id == record_id && rec.is_current) {
            version = rec.version;
            break;
        }
    }
    
    pthread_mutex_unlock(&store_mutex);
    return version;
}

// ============================================================================
// ACCESS STATISTICS
// ============================================================================

double DataStore::get_average_access_time(DataTier tier) {
    int tier_index = 0;
    switch (tier) {
        case DATA_TIER_L1: tier_index = 0; break;
        case DATA_TIER_L2: tier_index = 1; break;
        case DATA_TIER_L3: tier_index = 2; break;
        case DATA_TIER_L4: tier_index = 3; break;
    }
    
    pthread_mutex_lock(&store_mutex);
    double avg = tier_access_count[tier_index] > 0 
                 ? (double)total_access_delay[tier_index] / tier_access_count[tier_index]
                 : 0.0;
    pthread_mutex_unlock(&store_mutex);
    return avg;
}

int DataStore::get_tier_access_count(DataTier tier) {
    int tier_index = 0;
    switch (tier) {
        case DATA_TIER_L1: tier_index = 0; break;
        case DATA_TIER_L2: tier_index = 1; break;
        case DATA_TIER_L3: tier_index = 2; break;
        case DATA_TIER_L4: tier_index = 3; break;
    }
    
    pthread_mutex_lock(&store_mutex);
    int count = tier_access_count[tier_index];
    pthread_mutex_unlock(&store_mutex);
    return count;
}
