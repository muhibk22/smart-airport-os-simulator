#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace std;

Logger* Logger::instance = nullptr;

Logger::Logger() {
    // Initialize log mutexes
    for (int i = 0; i < 4; i++) {
        pthread_mutex_init(&log_mutex[i], nullptr);
    }
    
    // Open log files
    scheduling_log.open("logs/scheduling.log", ios::out | ios::trunc);
    memory_log.open("logs/memory.log", ios::out | ios::trunc);
    events_log.open("logs/events.log", ios::out | ios::trunc);
    performance_log.open("logs/performance.log", ios::out | ios::trunc);
    
    if (!scheduling_log.is_open() || !memory_log.is_open() || 
        !events_log.is_open() || !performance_log.is_open()) {
        cerr << "Error: Could not open log files\n";
    }
}

Logger::~Logger() {
    flush_all();
    
    scheduling_log.close();
    memory_log.close();
    events_log.close();
    performance_log.close();
    
    for (int i = 0; i < 4; i++) {
        pthread_mutex_destroy(&log_mutex[i]);
    }
}

Logger* Logger::get_instance() {
    if (instance == nullptr) {
        instance = new Logger();
    }
    return instance;
}

void Logger::log(LogChannel channel, const std::string& message) {
    pthread_mutex_lock(&log_mutex[channel]);
    
    std::ofstream* log_file = nullptr;
    switch(channel) {
        case SCHEDULING_LOG: log_file = &scheduling_log; break;
        case MEMORY_LOG: log_file = &memory_log; break;
        case EVENTS_LOG: log_file = &events_log; break;
        case PERFORMANCE_LOG: log_file = &performance_log; break;
    }
    
    if (log_file && log_file->is_open()) {
        (*log_file) << message << std::endl;
        log_file->flush();
    }
    
    pthread_mutex_unlock(&log_mutex[channel]);
}

void Logger::log_scheduling(const std::string& message) {
    log(SCHEDULING_LOG, message);
}

void Logger::log_memory(const std::string& message) {
    log(MEMORY_LOG, message);
}

void Logger::log_event(const std::string& message) {
    log(EVENTS_LOG, message);
}

void Logger::log_performance(const std::string& message) {
    log(PERFORMANCE_LOG, message);
}

void Logger::flush_all() {
    for (int i = 0; i < 4; i++) {
        pthread_mutex_lock(&log_mutex[i]);
    }
    
    scheduling_log.flush();
    memory_log.flush();
    events_log.flush();
    performance_log.flush();
    
    for (int i = 3; i >= 0; i--) {
        pthread_mutex_unlock(&log_mutex[i]);
    }
}
