cmake_minimum_required(VERSION 3.8)

project(spi)

find_package(verilator HINTS $ENV{VERILATOR_ROOT} ${VERILATOR_ROOT})
if (NOT verilator_FOUND)
  message(FATAL_ERROR "Verilator was not found. Either install it, or set the VERILATOR_ROOT environment variable")
endif()


set(CMAKE_BUILD_TYPE DEBUG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRACE")

# ${PROJECT_SOURCE_DIR}

add_executable(simulation sim.cpp spislave.cpp)

target_include_directories(simulation PUBLIC ../import/uartmaster/sim)

message(STATUS "${PROJECT_SOURCE_DIR}/../rtl")

# Add the Verilated circuit to the target
verilate(simulation TRACE
    SOURCES     ../rtl/spi.v
    TOP_MODULE  spi
    THREADS 8
  )

add_custom_target(sim
    COMMAND simulation
    COMMENT "Executing simulation..."
)
