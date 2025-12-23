#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

using namespace std;

Logger* Logger::instance = nullptr;

Logger::Logger() {
    // Initialize mutexes for each log channel
    for (int i = 0; i < 5; i++) {
        pthread_mutex_init(&log_mutexes[i], nullptr);
    }
    
    // Create logs directory if it doesn't exist
    #ifdef _WIN32
    system("if not exist logs mkdir logs");
    #else
    system("mkdir -p logs");
    #endif
    
    // Open log files
    log_files[SCHEDULING_LOG].open("logs/scheduling.log", ios::out | ios::trunc);
    log_files[MEMORY_LOG].open("logs/memory.log", ios::out | ios::trunc);
    log_files[EVENTS_LOG].open("logs/events.log", ios::out | ios::trunc);
    log_files[PERFORMANCE_LOG].open("logs/performance.log", ios::out | ios::trunc);
    log_files[RESOURCES_LOG].open("logs/resources.log", ios::out | ios::trunc);
    
    for (int i = 0; i < 5; i++) {
        if (!log_files[i].is_open()) {
            cerr << "Error: Could not open log file " << i << endl;
        }
    }
}

Logger::~Logger() {
    flush_all();
    
    for (int i = 0; i < 5; i++) {
        log_files[i].close();
        pthread_mutex_destroy(&log_mutexes[i]);
    }
}

Logger* Logger::get_instance() {
    if (instance == nullptr) {
        instance = new Logger();
    }
    return instance;
}

void Logger::log(LogChannel channel, const string& message) {
    pthread_mutex_lock(&log_mutexes[channel]);
    
    if (log_files[channel].is_open()) {
        log_files[channel] << message << endl;
        log_files[channel].flush();
    }
    
    pthread_mutex_unlock(&log_mutexes[channel]);
}

void Logger::log_scheduling(const string& message) {
    log(SCHEDULING_LOG, message);
}

void Logger::log_memory(const string& message) {
    log(MEMORY_LOG, message);
}

void Logger::log_event(const string& message) {
    log(EVENTS_LOG, message);
}

void Logger::log_performance(const string& message) {
    log(PERFORMANCE_LOG, message);
}

void Logger::log_resource(const string& message) {
    log(RESOURCES_LOG, message);
}

void Logger::flush_all() {
    for (int i = 0; i < 5; i++) {
        pthread_mutex_lock(&log_mutexes[i]);
        log_files[i].flush();
        pthread_mutex_unlock(&log_mutexes[i]);
    }
}
