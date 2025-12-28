# SmartAirportOS

**Smart Airport Operations Management System Simulator**

A discrete-event simulation of a Tier-1 international airport implementing Operating System concepts using **POSIX pthreads** for concurrency, **HMFQ-PPRA Scheduling**, and **AWSC-PPC Page Replacement**.

## Features

- **Process Scheduling**: HMFQ-PPRA scheduler managing 50+ concurrent flight operations
- **Memory Management**: AWSC-PPC page replacement with working set prediction
- **Concurrency Control**: POSIX pthread-based multi-threading with mutex/condition variable synchronization
- **Resource Allocation**: Deadlock-free management using Banker's algorithm
- **Real-time Monitoring**: Text-based dashboard displaying live system metrics

## Project Structure

```
smart-airport-os-simulator/
├── config/                        # Configuration files
│   ├── airport_config.json        # Airport layout, gates, runways
│   ├── resources.json             # Resource pool definitions
│   ├── scheduling_weights.json    # HMFQ-PPRA parameters
│   └── simulation_params.json     # Simulation settings
│
├── src/
│   ├── main.cpp                   # Entry point
│   ├── core/                      # Core Simulation Engine
│   ├── airport/                   # Airport Operations (Aircraft, Runway, Gate, etc.)
│   ├── services/                  # Ground Services
│   ├── resources/                 # Resource Management & Deadlock Prevention
│   ├── passengers/                # Passenger & Baggage Systems
│   ├── crew/                      # Crew Management & Fatigue Model
│   ├── crisis/                    # Crisis & Emergency Management
│   ├── scheduling/                # HMFQ-PPRA Scheduler
│   ├── memory/                    # AWSC-PPC Memory Manager
│   ├── data/                      # Data Hierarchy
│   └── finance/                   # Financial Tracking
│
├── tests/                         # Test Scenarios
├── logs/                          # Simulation Logs
└── scripts/                       # Utility Scripts
```

## Build Instructions

### Prerequisites

- **C++17 compatible compiler**: g++ 7.0+, clang 5.0+, MSVC 2017+
- **CMake 3.10+**
- **POSIX threads library**: pthread (included in Linux/macOS, MinGW-w64 for Windows)

### Build Steps

```bash
# Create build directory
mkdir build && cd build

# Configure and compile
cmake ..
cmake --build .

# Run
./SmartAirportOS
```

**Windows with MinGW:**
```bash
g++ -std=c++17 -pthread src/*.cpp -o SmartAirportOS
```

## Configuration

Edit files in `config/` directory to customize:

- **airport_config.json**: Number of runways, gates, terminal layout
- **resources.json**: Resource pool sizes (fuel trucks, crews, etc.)
- **scheduling_weights.json**: PIS calculation weights
- **simulation_params.json**: Duration, flight arrival rate, random seed

Example `simulation_params.json`:
```json
{
  "simulation_duration": 86400,
  "flight_arrival_rate": 30,
  "random_seed": 42,
  "time_unit_ms": 10,
  "max_concurrent_flights": 50
}
```

## Logs

The simulator generates detailed logs in the `logs/` directory:

- `scheduling.log` - Scheduling decisions
- `memory.log` - Page faults & replacements
- `events.log` - All simulation events
- `performance.log` - Metrics & KPIs

## License

Academic project for Operating Systems course.
