#!/bin/bash

echo "Compiling with g++ directly..."

# Create logs directory if needed
mkdir -p logs

# Compile simple_test
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
    tests/simple_test.cpp \
    -o simple_test

if [ $? -eq 0 ]; then
    echo "✓ Build successful!"
    echo "Run with: ./simple_test"
else
    echo "✗ Build failed"
fi
