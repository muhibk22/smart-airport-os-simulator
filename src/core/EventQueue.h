#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "Event.h"
#include <queue>
#include <vector>
#include <functional>
#include <pthread.h>

using namespace std;

class EventQueue {
private:
    priority_queue<Event*, vector<Event*>, 
                        function<bool(Event*, Event*)>> queue;
    pthread_mutex_t queue_mutex;
    pthread_cond_t event_available;
    
    // Comparator for min-heap (Event::operator< already handles the ordering)
    static bool compare(Event* a, Event* b) {
        return *a < *b; // Use Event's operator< directly (already returns time > for min-heap)
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
