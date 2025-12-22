#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "Event.h"
#include <queue>
#include <vector>
#include <functional>
#include <pthread.h>

class EventQueue {
private:
    std::priority_queue<Event*, std::vector<Event*>, 
                        std::function<bool(Event*, Event*)>> queue;
    pthread_mutex_t queue_mutex;
    pthread_cond_t event_available;
    
    // Comparator for min-heap
    static bool compare(Event* a, Event* b) {
        return *b < *a; // Invert for min-heap
    }
    
public:
    EventQueue();
    ~EventQueue();
    
    void push(Event* event);
    Event* pop();
    Event* peek();
    bool empty();
    size_t size();
    
    // Wait for event (blocking)
    Event* wait_for_event();
};

#endif // EVENT_QUEUE_H
