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

# Test executable (integration test)
add_executable(simple_test tests/simple_test.cpp ${SOURCES})
target_link_libraries(simple_test PRIVATE Threads::Threads)

# Requirements test executable (REQ-6)
add_executable(requirements_test tests/requirements_test.cpp ${SOURCES})
target_link_libraries(requirements_test PRIVATE Threads::Threads)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(SmartAirportOS PRIVATE -Wall -Wextra -pthread)
endif()
