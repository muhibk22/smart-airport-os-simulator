# SmartAirportOS

**Smart Airport Operations Management System Simulator**

A comprehensive discrete-event simulation of a Tier-1 international airport implementing advanced Operating System concepts including **HMFQ-PPRA Scheduling** (8 layers) and **AWSC-PPC Page Replacement** (8 components).

---


## Project Overview

### Purpose

This simulator demonstrates OS concepts through airport operations management:

- **Scheduling**: HMFQ-PPRA scheduler with 8 layers managing flight operations as processes
- **Memory Management**: AWSC-PPC page replacement algorithm with 8 components
- **Resource Allocation**: Deadlock-free management of airport resources
- **Synchronization**: Thread-safe operations across all services
- **Real-time Dashboard**: Text-based monitoring of all operations

### Scope

**In-Scope:**
- Multi-level flight operations (commercial, cargo, private, emergency)
- Gate assignment with size/type constraints
- Ground service dependency management (15+ services)
- Resource allocation (7 resource types)
- Passenger and baggage pipelines
- Crew management with fatigue modeling
- Weather and emergency crisis handling
- Financial cost/revenue tracking
- Comprehensive logging and metrics

**Out-of-Scope:**
- GUI/graphical interface
- Real networking
- Real OS kernel interaction

---

## File Structure

```
smart-airport-os-simulator/
│
├── README.md                      # This file
├── CMakeLists.txt                 # Build configuration
│
├── config/                        # Configuration files
│   ├── airport_config.json        # Airport layout, gates, runways
│   ├── resources.json             # Resource pool definitions
│   ├── scheduling_weights.json    # HMFQ-PPRA parameters (α-ε)
│   └── simulation_params.json     # Simulation settings
│
├── src/
│   ├── main.cpp                   # Entry point
│
│   ├── core/                      # Core Simulation Engine (FR-14)
│   │   ├── SimulationEngine.cpp/h # Main simulation loop
│   │   ├── TimeManager.cpp/h      # Discrete-event time management
│   │   ├── Event.cpp/h            # Event base class
│   │   ├── EventQueue.cpp/h       # Priority queue for events
│   │   ├── Logger.cpp/h           # Multi-channel logging system
│   │   └── Dashboard.cpp/h        # Real-time text dashboard
│
│   ├── airport/                   # Airport Operations (FR-1, FR-2)
│   │   ├── Aircraft.cpp/h         # Aircraft types (A380, B777, etc.)
│   │   ├── Flight.cpp/h           # Flight entity with attributes
│   │   ├── Runway.cpp/h           # Runway resource with wake separation
│   │   ├── RunwayManager.cpp/h    # Runway assignment & parallel ops
│   │   ├── Gate.cpp/h             # Gate resource with constraints
│   │   ├── GateManager.cpp/h      # Bipartite gate assignment
│   │   └── TaxiwayGraph.cpp/h     # Taxiway graph & gridlock detection
│
│   ├── services/                  # Ground Services (FR-3)
│   │   ├── GroundService.cpp/h    # Base service class
│   │   ├── ServiceDependencyGraph.cpp/h  # DAG for dependencies
│   │   └── ServiceExecutor.cpp/h  # Topological execution engine
│
│   ├── resources/                 # Resource Management (FR-4)
│   │   ├── Resource.cpp/h         # Generic resource (GSE, crew, etc.)
│   │   ├── ResourceManager.cpp/h  # Banker's algorithm allocation
│   │   └── DeadlockDetector.cpp/h # Cycle detection in wait-for graph
│
│   ├── passengers/                # Passenger Systems (FR-6, FR-7, FR-8)
│   │   ├── PassengerGroup.cpp/h   # Passenger entity with attributes
│   │   ├── PassengerPipeline.cpp/h # Check-in → boarding pipeline
│   │   ├── Baggage.cpp/h          # Baggage entity with RFID
│   │   └── BaggageSystem.cpp/h    # Sorting, tracking, transfers
│
│   ├── crew/                      # Crew Management (FR-11)
│   │   ├── Crew.cpp/h             # Crew member with skills
│   │   ├── CrewManager.cpp/h      # Crew pool & LRU replacement
│   │   └── FatigueModel.cpp/h     # Duty limits & fatigue scoring
│
│   ├── crisis/                    # Crisis Management (FR-12)
│   │   ├── WeatherEvent.cpp/h     # Fog, storm, wind events
│   │   ├── EmergencyEvent.cpp/h   # Medical, technical emergencies
│   │   └── CrisisManager.cpp/h    # Priority escalation handler
│
│   ├── scheduling/                # HMFQ-PPRA Scheduler (100 Points)
│   │   ├── Operation.cpp/h        # Process abstraction (flights, services)
│   │   ├── HMFQQueue.cpp/h        # 5-level multi-feedback queue
│   │   ├── PISCalculator.cpp/h    # Priority Index Score (α-ε formula)
│   │   ├── AgingManager.cpp/h     # Exponential aging to prevent starvation
│   │   ├── QuantumManager.cpp/h   # Dynamic time quantum adjustment
│   │   ├── PriorityInheritance.cpp/h # Priority inversion prevention
│   │   ├── PreemptionManager.cpp/h # Cost-benefit preemption logic
│   │   └── LearningEngine.cpp/h   # Adaptive weight adjustment
│
│   ├── memory/                    # AWSC-PPC Memory Manager (100 Points)
│   │   ├── Page.cpp/h             # Page entity with metadata
│   │   ├── PageFrame.cpp/h        # Physical frame
│   │   ├── PageTable.cpp/h        # Virtual → physical mapping
│   │   ├── TLB.cpp/h              # Translation lookaside buffer
│   │   ├── WorkingSetManager.cpp/h # Dynamic working set tracking
│   │   ├── ClockReplacer.cpp/h    # Multi-pass clock algorithm
│   │   ├── Prefetcher.cpp/h       # Pattern-based prefetching
│   │   ├── CompressionManager.cpp/h # Memory compression simulation
│   │   └── ThrashingDetector.cpp/h # Markov-based thrashing detection
│
│   ├── data/                      # Data Hierarchy (FR-9, FR-10)
│   │   ├── DataRecord.cpp/h       # Versioned data record
│   │   └── DataStore.cpp/h        # 4-tier storage with latency
│
│   └── finance/                   # Financial Tracking (FR-13)
│       ├── CostModel.cpp/h        # Cost accumulators per operation
│       └── RevenueModel.cpp/h     # Revenue streams tracking
│
├── tests/                         # Test Scenarios (10 Points)
│   ├── test_scheduler.cpp         # HMFQ-PPRA validation
│   ├── test_memory.cpp            # AWSC-PPC validation
│   ├── test_resources.cpp         # Deadlock prevention tests
│   └── test_simulation.cpp        # End-to-end scenarios
│
├── logs/                          # Comprehensive Logging
│   ├── scheduling.log             # Every scheduling decision
│   ├── memory.log                 # Page faults & replacements
│   ├── events.log                 # All simulation events
│   └── performance.log            # Metrics & KPIs
│
└── scripts/                       # Utility Scripts
    ├── run_simulation.sh          # Execute simulation runs
    └── generate_metrics.sh        # Extract performance metrics
```

---

## Module Descriptions

### Core Simulation Engine

**Files:** `core/SimulationEngine.*`, `TimeManager.*`, `Event.*`, `EventQueue.*`

The core engine drives discrete-event simulation:
- **TimeManager**: Maintains simulation clock, advances time
- **EventQueue**: Priority queue ordered by event time
- **Event Types**: FlightArrival, ServiceStart, ServiceEnd, ResourceRequest, PageFault
- **Dashboard**: Real-time text display of airport status

### Airport Operations

**Files:** `airport/*`

Implements multi-level flight operations:
- **Aircraft Types**: A380, B777, B737, A320, B747F, B777F, G650, Falcon 7X, Emergency (9 types)
- **Wake Turbulence**: Heavy/Medium/Light separation rules
- **Runway Manager**: Parallel runway logic, go-around events
- **Gate Manager**: Size compatibility (A380, Heavy, Medium, Regional), domestic/international
- **Taxiway Graph**: Adjacency list with gridlock detection via wait-time threshold

### Ground Services

**Files:** `services/*`

Service dependency chain with 15+ services:
1. Marshalling → 2. Chock → 3. GPU connection → 4. Stairs/Jetbridge → 5. Door opening → 6. Passenger disembark → 7. Cabin cleaning → 8. Catering → 9. Fueling → 10. Water → 11. Waste → 12. Cargo unload → 13. Cargo load → 14. Boarding → 15. Pushback

- **ServiceDependencyGraph**: DAG representation
- **ServiceExecutor**: Topological sort execution with resource locking

### Scheduling: HMFQ-PPRA

**Files:** `scheduling/*`

**8-Layer Coverage:**

1. **5-Level Queue**: Critical → High → Normal → Low → Background
2. **PIS Calculation**: \( PIS = α \cdot urgency + β \cdot priority + γ \cdot wait + δ \cdot resource + ε \cdot predictive \)
3. **Exponential Aging**: \( boost = base \times e^{wait\_time / threshold} \)
4. **Dynamic Quantum**: Shorter for interactive, longer for batch
5. **Priority Inheritance**: Resource holders inherit requester priority
6. **Cost-Benefit Preemption**: \( benefit - cost > threshold \)
7. **Guaranteed Service**: Max wait time enforcement
8. **Adaptive Learning**: Adjust α-ε based on performance

**Logging:** Every scheduling decision logs PIS breakdown

### Memory: AWSC-PPC

**Files:** `memory/*`

**8-Component Coverage:**

1. **Dynamic Working Set**: Track recent page references
2. **Multi-Pass Clock**: 2nd-chance with reference/dirty bits
3. **Pattern Prefetch**: Detect sequential/stride patterns
4. **Compression**: Simulate compression ratio for swapped pages
5. **Frequency Decay**: Age-based access frequency reduction
6. **Priority Integration**: Protect high-priority process pages
7. **Thrashing Detection**: Detect page fault rate spikes
8. **Markov Prediction**: Predict future page access patterns

**Logging:** Page fault log with victim selection scores

### Resource Management

**Files:** `resources/*`

**7 Resource Types:**
1. Ground Support Equipment (GSE)
2. Refueling trucks
3. Catering vehicles
4. Cleaning crews
5. Baggage handlers
6. Technical staff
7. Tow tractors

**Deadlock Prevention:** Banker's algorithm with safe-state checking

### Passenger & Baggage

**Files:** `passengers/*`

**Passenger Pipeline:**
Check-in → Security → Immigration → Lounge → Boarding gate

**Special Handling:** VIP, disabled, unaccompanied minors

**Baggage System:**
- RFID tracking
- Sorting by destination
- Transfer management
- Misrouting simulation (1-2% error rate)

### Crew Management

**Files:** `crew/*`

- **Duty Limits**: 8-hour shift, 12-hour max with breaks
- **Fatigue Model**: Score decay over shift duration
- **LRU Replacement**: Least-recently-used crew for new assignments

### Crisis Management

**Files:** `crisis/*`

**Weather Events:**
- Fog (visibility < 400m): No landings
- High wind (>35 knots): Restricted ops
- Thunderstorms: Ground delay

**Emergency Events:**
- Medical emergency
- Technical failure
- Security threat

**Priority Escalation:** Emergency flights get Critical priority

### Financial Tracking

**Files:** `finance/*`

**Costs:**
- Fuel, crew wages, ground services, gate fees, delay penalties

**Revenue:**
- Landing fees, gate fees, passenger charges, cargo fees

---

## Build Instructions

### Prerequisites

- C++17 compatible compiler (g++ 7.0+, clang 5.0+, MSVC 2017+)
- CMake 3.10+
- JSON library (nlohmann/json recommended)

### Build Steps

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Compile
cmake --build .

# Run
./SmartAirportOS
```

### Configuration

Edit files in `config/` directory:

- `airport_config.json`: Number of runways, gates, terminal layout
- `resources.json`: Resource pool sizes
- `scheduling_weights.json`: Adjust α, β, γ, δ, ε for PIS calculation
- `simulation_params.json`: Simulation duration, flight arrival rate

---

## Testing

### Test Scenarios

Run from `tests/` directory:

1. **Rush Hour Surge** (`test_simulation.cpp`)
   - 20+ simultaneous arrivals
   - Validates scheduling fairness and resource contention

2. **Weather Crisis** (`test_simulation.cpp`)
   - Fog event causing runway closure
   - Validates priority escalation and rescheduling

3. **Emergency Landing** (`test_simulation.cpp`)
   - Medical emergency flight
   - Validates preemption and resource reallocation

4. **Deadlock Prevention** (`test_resources.cpp`)
   - Circular wait scenario
   - Validates Banker's algorithm

5. **Thrashing Workload** (`test_memory.cpp`)
   - Excessive page faults
   - Validates working set adjustment

### Metrics Generation

```bash
./scripts/generate_metrics.sh
```

**Output:**
- Runway utilization %
- Average gate turnaround time
- Service completion rates
- Passenger connection success rate
- Resource wait times
- Page fault rate
- Scheduler response time

---



## License

Academic project for Operating Systems course. Not for commercial use.

---

## Contact

For questions or contributions, feel free to contact me :).


