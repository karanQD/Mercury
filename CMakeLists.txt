cmake_minimum_required(VERSION 3.20)
project(Mercury VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MERCURY_BUILD_TESTS "Build Mercury tests" ON)
option(MERCURY_BUILD_BENCHMARKS "Build Mercury benchmarks" ON)

add_compile_options(
    -O3
    -march=native
    -mtune=native
    -Wall
    -Wextra
    -Werror
    -Wpedantic
    -ffast-math
    -funroll-loops
    -finline-functions
)

include(GNUInstallDirs)
include_directories(include)

set(MERCURY_SOURCES
    src/main.cpp
    src/receiver/pcap_receiver.cpp
    src/receiver/udp_receiver.cpp
    src/receiver/io_uring_receiver.cpp
    src/parser/itch50_parser.cpp
    src/parser/message_validator.cpp
    src/queue/spsc_ring_buffer.cpp
    src/router/symbol_router.cpp
    src/timestamp/timestamp_engine.cpp
    src/consumer/latency_logger.cpp
    src/consumer/order_book.cpp
    src/consumer/metrics_consumer.cpp
)

add_executable(mercury_main ${MERCURY_SOURCES})
target_include_directories(mercury_main PRIVATE include)
target_link_libraries(mercury_main PRIVATE pthread)

if(MERCURY_BUILD_TESTS)
    enable_testing()
    foreach(test_name parser_tests queue_tests validator_tests integration_tests)
        add_executable(${test_name} tests/${test_name}.cpp)
        target_include_directories(${test_name} PRIVATE include)
        target_link_libraries(${test_name} PRIVATE pthread)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()

if(MERCURY_BUILD_BENCHMARKS)
    foreach(bench_name parser_benchmark queue_comparison latency_benchmark routing_benchmark)
        add_executable(${bench_name} benchmarks/${bench_name}.cpp)
        target_include_directories(${bench_name} PRIVATE include)
        target_link_libraries(${bench_name} PRIVATE pthread)
    endforeach()
endif()
