#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <pthread.h>

using namespace std;

enum LogChannel {
    SCHEDULING_LOG,
    MEMORY_LOG,
    EVENTS_LOG,
    PERFORMANCE_LOG,
    RESOURCES_LOG
};

class Logger {
private:
    static Logger* instance;
    
    ofstream log_files[5];
    pthread_mutex_t log_mutexes[5];
    
    Logger();
    
public:
    ~Logger();
    
    static Logger* get_instance();
    
    void log(LogChannel channel, const string& message);
    void log_scheduling(const string& message);
    void log_memory(const string& message);
    void log_event(const string& message);
    void log_performance(const string& message);
    void log_resource(const string& message);
    
    void flush_all();
};

#endif // LOGGER_H
