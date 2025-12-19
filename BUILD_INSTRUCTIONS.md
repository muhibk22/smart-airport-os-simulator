# Smart Airport OS Simulator - Build & Test Instructions

## Prerequisites

### Linux (Recommended)
```bash
# Ubuntu/Debian
sudo apt install build-essential cmake

# Fedora/RHEL
sudo dnf install gcc-c++ cmake

# Arch
sudo pacman -S base-devel cmake
```

**Why Linux:** Native pthread support, no additional libraries needed.

### Windows (Advanced)
Requires MinGW-w64 with POSIX threads. **Not recommended** - use Linux/WSL instead.

## Build Instructions (Linux)

### Quick Start
```bash
chmod +x build.sh
./build.sh
```

### Manual Build
```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)  # Use all CPU cores
```

## Running Tests

### Simple pthread Test
```bash
cd build
./simple_test
```

**What it tests:**
- ✓ pthread creation for 5 concurrent flights
- ✓ Per-runway mutex synchronization
- ✓ Wake turbulence separation enforcement
- ✓ Per-gate mutex synchronization
- ✓ Gate compatibility checking
- ✓ No deadlocks with 4 runways, 20 gates

**Expected output:**
```
╔═══════════════════════════════════════════════════════════════╗
║          Simple pthread Test - Airport Infrastructure         ║
╚═══════════════════════════════════════════════════════════════╝

Creating test flights...
Created 5 test flights
  - AA100 (A380)
  - UA200 (B777)
  - DL300 (B737)
  - SW400 (A320)
  - PVT500 (G650)

Scheduling arrival events...
  - Scheduled AA100 at time 100
  - Scheduled UA200 at time 105
  ...

Running simulation for 30 seconds...

Stopping simulation...

Test Summary:
  Total Flights: 5
  - AA100: DEPARTED
  - UA200: DEPARTED
  - DL300: DEPARTED
  - SW400: DEPARTED
  - PVT500: DEPARTED

  Completed: 5/5

✓ Test finished successfully
```

### Check Logs
After running, check `logs/` directory:
- `events.log` - All flight operations with timestamps
- `scheduling.log` - Scheduling decisions (empty for now)
- `memory.log` - Page faults (empty for now)
- `performance.log` - Metrics (empty for now)

**Look for in events.log:**
```
[FLIGHT_THREAD] Flight AA100 thread started
[FLIGHT] AA100 approaching, requesting runway
[RunwayManager] Flight AA100 allocated runway 27L
[FLIGHT] AA100 landing on runway 27L
[FLIGHT] AA100 cleared runway 27L
[GateManager] Flight AA100 allocated gate 0
[FLIGHT] AA100 at gate 0
[FLIGHT] AA100 servicing complete
[FLIGHT] AA100 departed. Total time: 125 time units
```

## Verifying pthread Behavior

### 1. No Interleaved Allocations
✓ **Correct** (mutex working):
```
[RunwayManager] Flight AA100 allocated runway 27L
[RunwayManager] Flight UA200 allocated runway 27R
```

✗ **Wrong** (mutex broken):
```
[RunwayManager] Flight AA100 alloca
[RunwayManager] Flight UA200 allocated
ted runway 27L
runway 27L
```

### 2. Wake Turbulence Delays
If two heavy aircraft try to use same runway, second one waits:
```
[FLIGHT] UA200 waiting for runway (attempt 1)
[FLIGHT] UA200 waiting for runway (attempt 2)
[RunwayManager] Flight UA200 allocated runway 27L  ← After delay
```

### 3. Gate Compatibility
- A380 gets gate 0 or 1 (large gates only)
- B777 gets gates 0-4 (large/heavy)
- B737/A320 get gates 5-14 (medium)
- G650 gets gates 15-19 (small)

## Troubleshooting

### "pthread.h: No such file or directory"
**Fix:** Install build-essential (Ubuntu/Debian) or equivalent
```bash
sudo apt install build-essential
```

### Compilation errors
**Check:** GCC version supports C++17
```bash
g++ --version  # Should be 7.0 or higher
```

### Permission denied on build.sh
**Fix:** Make script executable
```bash
chmod +x build.sh
```

### No log files created
**Fix:** Create logs/ directory:
```bash
mkdir logs
```

## Next Steps

Once simple_test passes:
1. Implement HMFQ-PPRA scheduler (16 files, all 8 layers with formulas)
2. Implement AWSC-PPC memory manager (17 files, all 8 components with formulas)
3. Add ground services, passengers, crew, crisis management
4. Run full simulation with 50+ concurrent flights
