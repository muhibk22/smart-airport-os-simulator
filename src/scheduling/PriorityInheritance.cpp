#include "PriorityInheritance.h"
#include "../core/Logger.h"
#include <sstream>
#include <algorithm>

using namespace std;

PriorityInheritance::PriorityInheritance() {
    pthread_mutex_init(&inheritance_mutex, nullptr);
}

PriorityInheritance::~PriorityInheritance() {
    pthread_mutex_destroy(&inheritance_mutex);
    active_inheritances.clear();
}

void PriorityInheritance::apply_inheritance(Operation* holder, Operation* waiter) {
    if (holder == nullptr || waiter == nullptr) return;
    
    pthread_mutex_lock(&inheritance_mutex);
    
    // Only inherit if waiter has higher priority (lower queue number)
    if (waiter->current_queue < holder->current_queue) {
        InheritedPriority ip;
        ip.holder = holder;
        ip.waiter = waiter;
        ip.original_queue = holder->current_queue;
        ip.original_pis = holder->priority_score;
        
        // Boost holder to waiter's priority
        pthread_mutex_lock(&holder->op_mutex);
        holder->current_queue = waiter->current_queue;
        holder->priority_score = waiter->priority_score;
        pthread_mutex_unlock(&holder->op_mutex);
        
        active_inheritances.push_back(ip);
        
        Logger* logger = Logger::get_instance();
        ostringstream msg;
        msg << "[PRIORITY_INHERITANCE] Op " << holder->id 
            << " inherited priority from Op " << waiter->id
            << " (Q" << ip.original_queue << " -> Q" << holder->current_queue << ")";
        logger->log_scheduling(msg.str());
    }
    
    pthread_mutex_unlock(&inheritance_mutex);
}

void PriorityInheritance::restore_priority(Operation* holder) {
    if (holder == nullptr) return;
    
    pthread_mutex_lock(&inheritance_mutex);
    
    // Find and remove inheritance record
    auto it = find_if(active_inheritances.begin(), active_inheritances.end(),
        [holder](const InheritedPriority& ip) { return ip.holder == holder; });
    
    if (it != active_inheritances.end()) {
        // Restore original priority
        pthread_mutex_lock(&holder->op_mutex);
        holder->current_queue = it->original_queue;
        holder->priority_score = it->original_pis;
        pthread_mutex_unlock(&holder->op_mutex);
        
        Logger* logger = Logger::get_instance();
        ostringstream msg;
        msg << "[PRIORITY_INHERITANCE] Op " << holder->id 
            << " priority restored to Q" << holder->current_queue;
        logger->log_scheduling(msg.str());
        
        active_inheritances.erase(it);
    }
    
    pthread_mutex_unlock(&inheritance_mutex);
}

bool PriorityInheritance::has_inherited_priority(Operation* op) {
    if (op == nullptr) return false;
    
    pthread_mutex_lock(&inheritance_mutex);
    
    bool found = any_of(active_inheritances.begin(), active_inheritances.end(),
        [op](const InheritedPriority& ip) { return ip.holder == op; });
    
    pthread_mutex_unlock(&inheritance_mutex);
    
    return found;
}

Operation* PriorityInheritance::get_waiter(Operation* holder) {
    if (holder == nullptr) return nullptr;
    
    pthread_mutex_lock(&inheritance_mutex);
    
    Operation* waiter = nullptr;
    auto it = find_if(active_inheritances.begin(), active_inheritances.end(),
        [holder](const InheritedPriority& ip) { return ip.holder == holder; });
    
    if (it != active_inheritances.end()) {
        waiter = it->waiter;
    }
    
    pthread_mutex_unlock(&inheritance_mutex);
    
    return waiter;
}
