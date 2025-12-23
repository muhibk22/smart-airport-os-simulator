#include "HMFQQueue.h"
#include "../core/Logger.h"
#include <sstream>
#include <algorithm>

using namespace std;

HMFQQueue::HMFQQueue() {
    current_operation = nullptr;
    
    pis_calculator = new PISCalculator();
    aging_manager = new AgingManager();
    quantum_manager = new QuantumManager();
    preemption_manager = new PreemptionManager();
    learning_engine = new LearningEngine(pis_calculator);
    priority_inheritance = new PriorityInheritance();
    
    pthread_mutex_init(&scheduler_mutex, nullptr);
    pthread_cond_init(&operation_available, nullptr);
    
    total_operations_scheduled = 0;
    total_preemptions = 0;
    total_context_switches = 0;
    total_wait_time = 0;
    next_operation_id = 1;
}

HMFQQueue::~HMFQQueue() {
    pthread_mutex_destroy(&scheduler_mutex);
    pthread_cond_destroy(&operation_available);
    
    delete pis_calculator;
    delete aging_manager;
    delete quantum_manager;
    delete preemption_manager;
    delete learning_engine;
    delete priority_inheritance;
    
    // Clean up remaining operations
    for (int i = 0; i < 5; i++) {
        for (Operation* op : queues[i]) {
            delete op;
        }
        queues[i].clear();
    }
}

void HMFQQueue::initialize() {
    Logger* logger = Logger::get_instance();
    logger->log_scheduling("[HMFQ] Scheduler initialized with 5 priority queues");
}

Operation* HMFQQueue::create_operation(Flight* flight, OperationType type, long long current_time) {
    pthread_mutex_lock(&scheduler_mutex);
    
    Operation* op = new Operation();
    op->id = next_operation_id++;
    op->flight = flight;
    op->type = type;
    op->arrival_time = current_time;
    
    // Set complexity based on operation type
    switch (type) {
        case OP_TAXIING:
            op->complexity = COMPLEXITY_SIMPLE;
            op->total_time = 60;  // 1 minute taxi
            break;
        case OP_LANDING:
        case OP_TAKEOFF:
            op->complexity = COMPLEXITY_MEDIUM;
            op->total_time = 90;  // 90 seconds
            break;
        case OP_REFUELING:
            op->complexity = COMPLEXITY_MEDIUM;
            op->total_time = 300; // 5 minutes
            break;
        case OP_GATE_ARRIVAL:
        case OP_GATE_DEPARTURE:
            op->complexity = COMPLEXITY_COMPLEX;
            op->total_time = 180; // 3 minutes
            break;
        case OP_CLEANING:
        case OP_CATERING:
            op->complexity = COMPLEXITY_MEDIUM;
            op->total_time = 600; // 10 minutes
            break;
        case OP_BOARDING:
        case OP_BAGGAGE:
            op->complexity = COMPLEXITY_COMPLEX;
            op->total_time = 1200; // 20 minutes
            break;
        case OP_EMERGENCY:
            op->complexity = COMPLEXITY_SIMPLE;
            op->total_time = 30;  // Fast emergency handling
            break;
    }
    op->remaining_time = op->total_time;
    
    // Assign initial queue based on flight priority
    if (flight) {
        if (flight->is_emergency()) {
            op->current_queue = 0;  // Emergency
        } else if (flight->priority <= 20) {
            op->current_queue = 1;  // Critical
        } else if (flight->priority <= 40) {
            op->current_queue = 2;  // High
        } else if (flight->priority <= 60) {
            op->current_queue = 3;  // Normal
        } else {
            op->current_queue = 4;  // Low
        }
    }
    
    // Calculate initial PIS
    op->priority_score = pis_calculator->calculate_pis(op);
    
    pthread_mutex_unlock(&scheduler_mutex);
    
    Logger* logger = Logger::get_instance();
    ostringstream msg;
    msg << "[HMFQ] Created operation " << op->id << " for flight " 
        << (flight ? flight->flight_id : "N/A") << " in Q" << op->current_queue
        << " (PIS: " << op->priority_score << ")";
    logger->log_scheduling(msg.str());
    
    return op;
}

void HMFQQueue::enqueue(Operation* op) {
    if (op == nullptr) return;
    
    pthread_mutex_lock(&scheduler_mutex);
    
    int queue = op->current_queue;
    if (queue < 0) queue = 0;
    if (queue > 4) queue = 4;
    
    queues[queue].push_back(op);
    total_operations_scheduled++;
    
    // Signal that an operation is available
    pthread_cond_signal(&operation_available);
    
    pthread_mutex_unlock(&scheduler_mutex);
    
    Logger* logger = Logger::get_instance();
    ostringstream msg;
    msg << "[HMFQ] Enqueued operation " << op->id << " to Q" << queue;
    logger->log_scheduling(msg.str());
    
    // Check if this new operation should preempt current
    check_preemption(op);
}

Operation* HMFQQueue::find_next_operation() {
    // Search from highest priority (Q0) to lowest (Q4)
    for (int q = 0; q < 5; q++) {
        if (!queues[q].empty()) {
            // Find operation with highest PIS in this queue
            Operation* best = nullptr;
            int best_index = -1;
            double best_score = -1.0;
            
            for (size_t i = 0; i < queues[q].size(); i++) {
                Operation* op = queues[q][i];
                if (!op->is_blocked && !op->is_running) {
                    if (op->priority_score > best_score) {
                        best_score = op->priority_score;
                        best = op;
                        best_index = i;
                    }
                }
            }
            
            if (best != nullptr) {
                queues[q].erase(queues[q].begin() + best_index);
                return best;
            }
        }
    }
    return nullptr;
}

void HMFQQueue::recalculate_priorities(long long current_time) {
    for (int q = 0; q < 5; q++) {
        for (Operation* op : queues[q]) {
            op->priority_score = pis_calculator->calculate_pis(op);
        }
    }
}

Operation* HMFQQueue::dequeue(long long current_time) {
    pthread_mutex_lock(&scheduler_mutex);
    
    // Apply aging to all waiting operations
    apply_aging(current_time);
    
    // Recalculate priorities
    recalculate_priorities(current_time);
    
    // Find next operation
    Operation* op = find_next_operation();
    
    if (op != nullptr) {
        op->is_running = true;
        op->start_time = current_time;
        current_operation = op;
        total_context_switches++;
        
        Logger* logger = Logger::get_instance();
        ostringstream msg;
        msg << "[HMFQ] Dequeued operation " << op->id 
            << " (Q" << op->current_queue << ", PIS: " << op->priority_score << ")";
        logger->log_scheduling(msg.str());
    }
    
    pthread_mutex_unlock(&scheduler_mutex);
    
    return op;
}

void HMFQQueue::complete(Operation* op) {
    if (op == nullptr) return;
    
    pthread_mutex_lock(&scheduler_mutex);
    
    op->is_completed = true;
    op->is_running = false;
    
    if (current_operation == op) {
        current_operation = nullptr;
    }
    
    // Track wait time
    total_wait_time += op->wait_time;
    
    pthread_mutex_unlock(&scheduler_mutex);
    
    Logger* logger = Logger::get_instance();
    ostringstream msg;
    msg << "[HMFQ] Completed operation " << op->id 
        << " (wait: " << op->wait_time << ", preemptions: " << op->preemption_count << ")";
    logger->log_scheduling(msg.str());
    
    // Update learning engine with completion data
    long long completion_time = op->total_time - op->remaining_time;
    learning_engine->update_completion_time(completion_time);
    learning_engine->update_wait_time(op->wait_time);
    learning_engine->update_on_time_rate(op->wait_time < 300);  // On-time if wait < 5 min
}

void HMFQQueue::block(Operation* op) {
    if (op == nullptr) return;
    
    pthread_mutex_lock(&scheduler_mutex);
    
    op->is_blocked = true;
    op->is_running = false;
    
    // Re-add to queue
    int queue = op->current_queue;
    queues[queue].push_back(op);
    
    if (current_operation == op) {
        current_operation = nullptr;
    }
    
    pthread_mutex_unlock(&scheduler_mutex);
}

void HMFQQueue::unblock(Operation* op) {
    if (op == nullptr) return;
    
    pthread_mutex_lock(&scheduler_mutex);
    op->is_blocked = false;
    pthread_cond_signal(&operation_available);
    pthread_mutex_unlock(&scheduler_mutex);
}

bool HMFQQueue::check_preemption(Operation* new_op) {
    if (current_operation == nullptr || new_op == nullptr) {
        return false;
    }
    
    pthread_mutex_lock(&scheduler_mutex);
    
    // Only consider preemption if new op is in higher priority queue
    if (new_op->current_queue < current_operation->current_queue) {
        if (preemption_manager->should_preempt(new_op, current_operation)) {
            preemption_manager->perform_preemption(new_op, current_operation);
            
            // Re-add preempted operation to its queue
            int q = current_operation->current_queue;
            queues[q].push_back(current_operation);
            
            current_operation = nullptr;
            total_preemptions++;
            
            pthread_mutex_unlock(&scheduler_mutex);
            return true;
        }
    }
    
    pthread_mutex_unlock(&scheduler_mutex);
    return false;
}

void HMFQQueue::apply_aging(long long current_time) {
    for (int q = 1; q < 5; q++) {  // Skip Q0 (Emergency)
        for (Operation* op : queues[q]) {
            aging_manager->apply_aging(op, current_time);
        }
    }
}

int HMFQQueue::get_queue_size(int queue_level) {
    if (queue_level < 0 || queue_level > 4) return 0;
    
    pthread_mutex_lock(&scheduler_mutex);
    int size = queues[queue_level].size();
    pthread_mutex_unlock(&scheduler_mutex);
    
    return size;
}

double HMFQQueue::get_average_wait_time() const {
    if (total_operations_scheduled == 0) return 0.0;
    return (double)total_wait_time / total_operations_scheduled;
}

void HMFQQueue::trigger_learning_adjustment() {
    pthread_mutex_lock(&scheduler_mutex);
    
    // Adjust PIS weights based on learning engine feedback
    learning_engine->adjust_weights();
    
    Logger* logger = Logger::get_instance();
    ostringstream msg;
    msg << "[HMFQ] Learning adjustment triggered - new weights: "
        << "alpha=" << pis_calculator->get_alpha()
        << " beta=" << pis_calculator->get_beta()
        << " gamma=" << pis_calculator->get_gamma()
        << " delta=" << pis_calculator->get_delta()
        << " epsilon=" << pis_calculator->get_epsilon();
    logger->log_scheduling(msg.str());
    
    pthread_mutex_unlock(&scheduler_mutex);
}
