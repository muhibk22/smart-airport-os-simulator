#!/bin/bash

echo "Compiling with g++ directly..."

# Create logs directory if needed
mkdir -p logs
mkdir -p data/output

# Compile simple_test with all modules
g++ -std=c++17 -pthread -I src \
    src/core/TimeManager.cpp \
    src/core/Event.cpp \
    src/core/EventQueue.cpp \
    src/core/Logger.cpp \
    src/core/Dashboard.cpp \
    src/core/SimulationEngine.cpp \
    src/core/FlightEvents.cpp \
    src/airport/Aircraft.cpp \
    src/airport/Flight.cpp \
    src/airport/Runway.cpp \
    src/airport/RunwayManager.cpp \
    src/airport/Gate.cpp \
    src/airport/GateManager.cpp \
    src/airport/TaxiwayGraph.cpp \
    src/scheduling/Operation.cpp \
    src/scheduling/PISCalculator.cpp \
    src/scheduling/AgingManager.cpp \
    src/scheduling/QuantumManager.cpp \
    src/scheduling/PreemptionManager.cpp \
    src/scheduling/HMFQQueue.cpp \
    src/scheduling/LearningEngine.cpp \
    src/scheduling/PriorityInheritance.cpp \
    src/memory/Page.cpp \
    src/memory/PageFrame.cpp \
    src/memory/PageTable.cpp \
    src/memory/TLB.cpp \
    src/memory/WorkingSetManager.cpp \
    src/memory/ClockReplacer.cpp \
    src/memory/ThrashingDetector.cpp \
    src/memory/Prefetcher.cpp \
    src/memory/CompressionManager.cpp \
    src/resources/Resource.cpp \
    src/resources/ResourceManager.cpp \
    src/resources/DeadlockDetector.cpp \
    src/crisis/WeatherEvent.cpp \
    src/crisis/EmergencyEvent.cpp \
    src/crisis/CrisisManager.cpp \
    src/crew/Crew.cpp \
    src/crew/CrewManager.cpp \
    src/crew/FatigueModel.cpp \
    src/finance/CostModel.cpp \
    src/finance/RevenueModel.cpp \
    src/data/DataStore.cpp \
    src/services/GroundService.cpp \
    src/services/ServiceDependencyGraph.cpp \
    src/services/ServiceExecutor.cpp \
    src/passengers/Baggage.cpp \
    src/passengers/PassengerGroup.cpp \
    src/passengers/PassengerPipeline.cpp \
    src/passengers/BaggageSystem.cpp \
    tests/simple_test.cpp \
    -o simple_test

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
    echo "Run with: ./simple_test"
else
    echo "✗ Build failed"
fi
