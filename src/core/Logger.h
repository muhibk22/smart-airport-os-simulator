#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <pthread.h>

enum LogChannel {
    SCHEDULING_LOG,
    MEMORY_LOG,
    EVENTS_LOG,
    PERFORMANCE_LOG
};

class Logger {
private:
    std::ofstream scheduling_log;
    std::ofstream memory_log;
    std::ofstream events_log;
    std::ofstream performance_log;
    
    pthread_mutex_t log_mutex[4];
    
    static Logger* instance;
    
    Logger();
    
public:
    ~Logger();
    
    static Logger* get_instance();
    
    void log(LogChannel channel, const std::string& message);
    void log_scheduling(const std::string& message);
    void log_memory(const std::string& message);
    void log_event(const std::string& message);
    void log_performance(const std::string& message);
    
    void flush_all();
};

#endif // LOGGER_H
