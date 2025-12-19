#include "EventQueue.h"
#include <functional>

EventQueue::EventQueue() 
    : queue(compare) {
    pthread_mutex_init(&queue_mutex, nullptr);
    pthread_cond_init(&event_available, nullptr);
}

EventQueue::~EventQueue() {
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&event_available);
}

void EventQueue::push(Event* event) {
    pthread_mutex_lock(&queue_mutex);
    queue.push(event);
    pthread_cond_signal(&event_available);
    pthread_mutex_unlock(&queue_mutex);
}

Event* EventQueue::pop() {
    pthread_mutex_lock(&queue_mutex);
    
    if (queue.empty()) {
        pthread_mutex_unlock(&queue_mutex);
        return nullptr;
    }
    
    Event* event = queue.top();
    queue.pop();
    pthread_mutex_unlock(&queue_mutex);
    
    return event;
}

Event* EventQueue::peek() {
    pthread_mutex_lock(&queue_mutex);
    
    Event* event = queue.empty() ? nullptr : queue.top();
    
    pthread_mutex_unlock(&queue_mutex);
    return event;
}

bool EventQueue::empty() {
    pthread_mutex_lock(&queue_mutex);
    bool is_empty = queue.empty();
    pthread_mutex_unlock(&queue_mutex);
    return is_empty;
}

size_t EventQueue::size() {
    pthread_mutex_lock(&queue_mutex);
    size_t s = queue.size();
    pthread_mutex_unlock(&queue_mutex);
    return s;
}

Event* EventQueue::wait_for_event() {
    pthread_mutex_lock(&queue_mutex);
    
    while (queue.empty()) {
        pthread_cond_wait(&event_available, &queue_mutex);
    }
    
    Event* event = queue.top();
    queue.pop();
    
    pthread_mutex_unlock(&queue_mutex);
    return event;
}
