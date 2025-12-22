#ifndef TAXIWAY_GRAPH_H
#define TAXIWAY_GRAPH_H

#include <vector>
#include <unordered_map>
#include <string>
#include <pthread.h>

using namespace std;

struct TaxiwayNode {
    int id;
    string name;
    bool occupied;
    int occupying_flight_id;
};

struct TaxiwayEdge {
    int from_node;
    int to_node;
    int weight;  // Time to traverse
};

class TaxiwayGraph {
private:
    vector<TaxiwayNode> nodes;
    unordered_map<int, vector<int>> adjacency_list;
    vector<TaxiwayEdge> edges;
    
    pthread_mutex_t graph_mutex;
    
    // Gridlock detection
    long long gridlock_threshold_seconds;
    
public:
    TaxiwayGraph();
    ~TaxiwayGraph();
    
    void add_node(int id, const string& name);
    void add_edge(int from, int to, int weight);
    
    // Find path between two nodes (simple BFS)
    vector<int> find_path(int from, int to);
    
    // Reserve path segments
    bool try_reserve_path(const vector<int>& path, int flight_id);
    
    // Release path
    void release_path(const vector<int>& path);
    
    // Gridlock detection
    bool detect_gridlock();
};

#endif // TAXIWAY_GRAPH_H
