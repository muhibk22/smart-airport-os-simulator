#include "DeadlockDetector.h"

using namespace std;

DeadlockDetector::DeadlockDetector() {
    pthread_mutex_init(&detector_mutex, nullptr);
}

DeadlockDetector::~DeadlockDetector() {
    pthread_mutex_destroy(&detector_mutex);
}

void DeadlockDetector::add_wait(int waiter_id, int holder_id) {
    pthread_mutex_lock(&detector_mutex);
    wait_for_graph[waiter_id].insert(holder_id);
    pthread_mutex_unlock(&detector_mutex);
}

void DeadlockDetector::remove_wait(int waiter_id, int holder_id) {
    pthread_mutex_lock(&detector_mutex);
    auto it = wait_for_graph.find(waiter_id);
    if (it != wait_for_graph.end()) {
        it->second.erase(holder_id);
        if (it->second.empty()) {
            wait_for_graph.erase(it);
        }
    }
    pthread_mutex_unlock(&detector_mutex);
}

void DeadlockDetector::remove_flight(int flight_id) {
    pthread_mutex_lock(&detector_mutex);
    
    // Remove as waiter
    wait_for_graph.erase(flight_id);
    
    // Remove as holder from all waiters
    for (auto& pair : wait_for_graph) {
        pair.second.erase(flight_id);
    }
    
    pthread_mutex_unlock(&detector_mutex);
}

bool DeadlockDetector::dfs_detect_cycle(int node, unordered_set<int>& visited,
                                         unordered_set<int>& rec_stack) {
    visited.insert(node);
    rec_stack.insert(node);
    
    auto it = wait_for_graph.find(node);
    if (it != wait_for_graph.end()) {
        for (int neighbor : it->second) {
            if (rec_stack.count(neighbor)) {
                return true;  // Cycle found
            }
            if (!visited.count(neighbor)) {
                if (dfs_detect_cycle(neighbor, visited, rec_stack)) {
                    return true;
                }
            }
        }
    }
    
    rec_stack.erase(node);
    return false;
}

bool DeadlockDetector::detect_deadlock() {
    pthread_mutex_lock(&detector_mutex);
    
    unordered_set<int> visited;
    unordered_set<int> rec_stack;
    
    for (auto& pair : wait_for_graph) {
        if (!visited.count(pair.first)) {
            if (dfs_detect_cycle(pair.first, visited, rec_stack)) {
                pthread_mutex_unlock(&detector_mutex);
                return true;
            }
        }
    }
    
    pthread_mutex_unlock(&detector_mutex);
    return false;
}

vector<int> DeadlockDetector::get_deadlocked_flights() {
    vector<int> deadlocked;
    
    pthread_mutex_lock(&detector_mutex);
    
    // Find all flights in cycles using Tarjan's algorithm (simplified)
    unordered_set<int> in_cycle;
    
    for (auto& pair : wait_for_graph) {
        int start = pair.first;
        unordered_set<int> visited;
        unordered_set<int> rec_stack;
        
        // DFS to find if this node is in a cycle
        if (dfs_detect_cycle(start, visited, rec_stack)) {
            in_cycle.insert(start);
        }
    }
    
    for (int flight : in_cycle) {
        deadlocked.push_back(flight);
    }
    
    pthread_mutex_unlock(&detector_mutex);
    return deadlocked;
}

void DeadlockDetector::reset() {
    pthread_mutex_lock(&detector_mutex);
    wait_for_graph.clear();
    pthread_mutex_unlock(&detector_mutex);
}
