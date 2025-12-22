#include "TaxiwayGraph.h"
#include <queue>
#include <unordered_set>
#include <algorithm>

using namespace std;

TaxiwayGraph::TaxiwayGraph() : gridlock_threshold_seconds(300) {
    pthread_mutex_init(&graph_mutex, nullptr);
}

TaxiwayGraph::~TaxiwayGraph() {
    pthread_mutex_destroy(&graph_mutex);
}

void TaxiwayGraph::add_node(int id, const string& name) {
    pthread_mutex_lock(&graph_mutex);
    
    TaxiwayNode node;
    node.id = id;
    node.name = name;
    node.occupied = false;
    node.occupying_flight_id = -1;
    
    nodes.push_back(node);
    
    pthread_mutex_unlock(&graph_mutex);
}

void TaxiwayGraph::add_edge(int from, int to, int weight) {
    pthread_mutex_lock(&graph_mutex);
    
    adjacency_list[from].push_back(to);
    
    TaxiwayEdge edge;
    edge.from_node = from;
    edge.to_node = to;
    edge.weight = weight;
    edges.push_back(edge);
    
    pthread_mutex_unlock(&graph_mutex);
}

vector<int> TaxiwayGraph::find_path(int from, int to) {
    pthread_mutex_lock(&graph_mutex);
    
    vector<int> path;
    
    if (from == to) {
        path.push_back(from);
        pthread_mutex_unlock(&graph_mutex);
        return path;
    }
    
    // BFS for shortest path
    queue<int> q;
    unordered_map<int, int> parent;
    unordered_set<int> visited;
    
    q.push(from);
    visited.insert(from);
    parent[from] = -1;
    
    bool found = false;
    
    while (!q.empty() && !found) {
        int current = q.front();
        q.pop();
        
        if (adjacency_list.find(current) != adjacency_list.end()) {
            for (int neighbor : adjacency_list[current]) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    q.push(neighbor);
                    
                    if (neighbor == to) {
                        found = true;
                        break;
                    }
                }
            }
        }
    }
    
    // Reconstruct path
    if (found) {
        int current = to;
        while (current != -1) {
            path.push_back(current);
            current = parent[current];
        }
        reverse(path.begin(), path.end());
    }
    
    pthread_mutex_unlock(&graph_mutex);
    return path;
}

bool TaxiwayGraph::try_reserve_path(const vector<int>& path, int flight_id) {
    pthread_mutex_lock(&graph_mutex);
    
    // Check if all nodes in path are free
    for (int node_id : path) {
        for (TaxiwayNode& node : nodes) {
            if (node.id == node_id && node.occupied) {
                pthread_mutex_unlock(&graph_mutex);
                return false; // Path blocked
            }
        }
    }
    
    // Reserve all nodes in path
    for (int node_id : path) {
        for (TaxiwayNode& node : nodes) {
            if (node.id == node_id) {
                node.occupied = true;
                node.occupying_flight_id = flight_id;
            }
        }
    }
    
    pthread_mutex_unlock(&graph_mutex);
    return true;
}

void TaxiwayGraph::release_path(const vector<int>& path) {
    pthread_mutex_lock(&graph_mutex);
    
    for (int node_id : path) {
        for (TaxiwayNode& node : nodes) {
            if (node.id == node_id) {
                node.occupied = false;
                node.occupying_flight_id = -1;
            }
        }
    }
    
    pthread_mutex_unlock(&graph_mutex);
}

bool TaxiwayGraph::detect_gridlock() {
    pthread_mutex_lock(&graph_mutex);
    
    // Simple gridlock detection: check if any cycles exist in wait-for graph
    // In production, would implement more sophisticated cycle detection
    
    bool gridlock = false;
    
    // Count occupied nodes
    int occupied_count = 0;
    for (const TaxiwayNode& node : nodes) {
        if (node.occupied) {
            occupied_count++;
        }
    }
    
    // If more than 80% occupied, potential gridlock
    if (nodes.size() > 0 && occupied_count > (nodes.size() * 80 / 100)) {
        gridlock = true;
    }
    
    pthread_mutex_unlock(&graph_mutex);
    return gridlock;
}
