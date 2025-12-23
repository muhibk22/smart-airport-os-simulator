#ifndef DEADLOCK_DETECTOR_H
#define DEADLOCK_DETECTOR_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <pthread.h>

using namespace std;

// DeadlockDetector implements wait-for graph with cycle detection

class DeadlockDetector {
private:
    // Wait-for graph: flight -> set of flights it's waiting for
    unordered_map<int, unordered_set<int>> wait_for_graph;
    
    pthread_mutex_t detector_mutex;
    
    // DFS for cycle detection
    bool dfs_detect_cycle(int node, unordered_set<int>& visited, 
                          unordered_set<int>& rec_stack);
    
public:
    DeadlockDetector();
    ~DeadlockDetector();
    
    // Add wait edge: waiter is waiting for holder
    void add_wait(int waiter_id, int holder_id);
    
    // Remove wait edge
    void remove_wait(int waiter_id, int holder_id);
    
    // Remove all edges involving a flight
    void remove_flight(int flight_id);
    
    // Detect if there's a cycle (deadlock)
    bool detect_deadlock();
    
    // Get flights involved in deadlock cycle
    vector<int> get_deadlocked_flights();
    
    // Clear all edges
    void reset();
};

#endif // DEADLOCK_DETECTOR_H
