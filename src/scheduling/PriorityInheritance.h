#ifndef PRIORITY_INHERITANCE_H
#define PRIORITY_INHERITANCE_H

#include "Operation.h"
#include <vector>
#include <pthread.h>

using namespace std;

// PriorityInheritance - Prevents priority inversion
// When a high-priority operation waits for a resource held by low-priority operation,
// the low-priority operation temporarily inherits the high priority

class PriorityInheritance {
private:
    pthread_mutex_t inheritance_mutex;
    
    // Track inherited priorities
    struct InheritedPriority {
        Operation* holder;      // Operation holding resource
        Operation* waiter;      // High-priority operation waiting
        int original_queue;     // Holder's original queue
        double original_pis;    // Holder's original PIS
    };
    
    vector<InheritedPriority> active_inheritances;
    
public:
    PriorityInheritance();
    ~PriorityInheritance();
    
    // Apply priority inheritance when high-priority op waits on low-priority holder
    void apply_inheritance(Operation* holder, Operation* waiter);
    
    // Restore original priority when resource is released
    void restore_priority(Operation* holder);
    
    // Check if an operation has inherited priority
    bool has_inherited_priority(Operation* op);
    
    // Get the waiting operation (highest priority waiter)
    Operation* get_waiter(Operation* holder);
};

#endif // PRIORITY_INHERITANCE_H
