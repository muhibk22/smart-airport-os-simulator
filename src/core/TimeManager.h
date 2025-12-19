#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <pthread.h>

class TimeManager {
private:
    long long current_time;
    pthread_mutex_t time_mutex;
    
public:
    TimeManager();
    ~TimeManager();
    
    void advance_time(long long delta);
    long long get_current_time();
    void set_time(long long time);
};

#endif // TIME_MANAGER_H
