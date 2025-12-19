cmake_minimum_required(VERSION 3.10)
project(SmartAirportOS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)

# Collect all source files
file(GLOB_RECURSE SOURCES
    "src/core/*.cpp"
    "src/airport/*.cpp"
    "src/services/*.cpp"
    "src/resources/*.cpp"
    "src/passengers/*.cpp"
    "src/crew/*.cpp"
    "src/crisis/*.cpp"
    "src/scheduling/*.cpp"
    "src/memory/*.cpp"
    "src/data/*.cpp"
    "src/finance/*.cpp"
)

# Main executable
add_executable(SmartAirportOS
    src/main.cpp
    ${SOURCES}
)

# Link pthread
target_link_libraries(SmartAirportOS PRIVATE Threads::Threads)

# Test executables
add_executable(test_scheduler tests/test_scheduler.cpp ${SOURCES})
target_link_libraries(test_scheduler PRIVATE Threads::Threads)

add_executable(test_memory tests/test_memory.cpp ${SOURCES})
target_link_libraries(test_memory PRIVATE Threads::Threads)

add_executable(test_resources tests/test_resources.cpp ${SOURCES})
target_link_libraries(test_resources PRIVATE Threads::Threads)

add_executable(test_simulation tests/test_simulation.cpp ${SOURCES})
target_link_libraries(test_simulation PRIVATE Threads::Threads)

add_executable(simple_test tests/simple_test.cpp ${SOURCES})
target_link_libraries(simple_test PRIVATE Threads::Threads)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(SmartAirportOS PRIVATE -Wall -Wextra -pthread)
endif()
