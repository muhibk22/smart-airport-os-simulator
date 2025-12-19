#include "TimeManager.h"

TimeManager::TimeManager() : current_time(0) {
    pthread_mutex_init(&time_mutex, nullptr);
}

TimeManager::~TimeManager() {
    pthread_mutex_destroy(&time_mutex);
}

void TimeManager::advance_time(long long delta) {
    pthread_mutex_lock(&time_mutex);
    current_time += delta;
    pthread_mutex_unlock(&time_mutex);
}

long long TimeManager::get_current_time() {
    pthread_mutex_lock(&time_mutex);
    long long time = current_time;
    pthread_mutex_unlock(&time_mutex);
    return time;
}

void TimeManager::set_time(long long time) {
    pthread_mutex_lock(&time_mutex);
    current_time = time;
    pthread_mutex_unlock(&time_mutex);
}
