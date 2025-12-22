#include "LearningEngine.h"
#include "../core/Logger.h"
#include <sstream>
#include <cmath>

using namespace std;

LearningEngine::LearningEngine(PISCalculator* calculator) {
    pis_calculator = calculator;
    avg_completion_time = 100.0;  // Initial estimate
    avg_wait_time = 30.0;
    on_time_rate = 0.90;  // 90% on-time target
    pthread_mutex_init(&learning_mutex, nullptr);
}

LearningEngine::~LearningEngine() {
    pthread_mutex_destroy(&learning_mutex);
}

void LearningEngine::update_completion_time(double actual_time) {
    pthread_mutex_lock(&learning_mutex);
    // New_Estimate = 0.7 × Old_Estimate + 0.3 × Actual_Value
    avg_completion_time = LEARNING_RATE_OLD * avg_completion_time + 
                          LEARNING_RATE_NEW * actual_time;
    pthread_mutex_unlock(&learning_mutex);
}

void LearningEngine::update_wait_time(double actual_wait) {
    pthread_mutex_lock(&learning_mutex);
    avg_wait_time = LEARNING_RATE_OLD * avg_wait_time + 
                    LEARNING_RATE_NEW * actual_wait;
    pthread_mutex_unlock(&learning_mutex);
}

void LearningEngine::update_on_time_rate(bool was_on_time) {
    pthread_mutex_lock(&learning_mutex);
    double actual = was_on_time ? 1.0 : 0.0;
    on_time_rate = LEARNING_RATE_OLD * on_time_rate + 
                   LEARNING_RATE_NEW * actual;
    pthread_mutex_unlock(&learning_mutex);
}

void LearningEngine::adjust_weights() {
    if (pis_calculator == nullptr) return;
    
    pthread_mutex_lock(&learning_mutex);
    
    double alpha = pis_calculator->get_alpha();
    double beta = pis_calculator->get_beta();
    double gamma = pis_calculator->get_gamma();
    double delta = pis_calculator->get_delta();
    double epsilon = pis_calculator->get_epsilon();
    
    // Adjust based on performance metrics
    // If wait time is high, increase delay propagation weight
    if (avg_wait_time > 50.0 && alpha < 0.35) {
        alpha += WEIGHT_ADJUSTMENT;
        gamma -= WEIGHT_ADJUSTMENT;  // Take from lowest impact
    }
    
    // If on-time rate is low, increase fuel criticality (affects urgency)
    if (on_time_rate < 0.85 && epsilon < 0.30) {
        epsilon += WEIGHT_ADJUSTMENT;
        gamma -= WEIGHT_ADJUSTMENT;
    }
    
    // Ensure weights sum to 1.0
    double sum = alpha + beta + gamma + delta + epsilon;
    if (abs(sum - 1.0) < 0.05) {
        pis_calculator->update_weights(alpha, beta, gamma, delta, epsilon);
        
        Logger* logger = Logger::get_instance();
        ostringstream msg;
        msg << "[LEARNING] Adjusted weights: α=" << alpha << " β=" << beta 
            << " γ=" << gamma << " δ=" << delta << " ε=" << epsilon;
        logger->log_scheduling(msg.str());
    }
    
    pthread_mutex_unlock(&learning_mutex);
}
