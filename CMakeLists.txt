cmake_minimum_required(VERSION 3.15)
project(escxml_embedded_runtime VERSION 0.4.0 LANGUAGES C)

add_library(escxml_runtime
    src/AssertHandler.c
    src/StateMachine.c
    src/StateMachineEventQueue.c)
add_library(ESCXML::Runtime ALIAS escxml_runtime)
target_include_directories(escxml_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(escxml_runtime PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)

if(MSVC)
    target_compile_options(escxml_runtime PRIVATE /W4)
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(escxml_runtime PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GHS")
    message(STATUS "Green Hills compiler detected; use project-specific MULTI warning/MISRA options")
endif()

option(ESCX_BUILD_TESTS "Build tests and example" ON)
set(ESCX_MAX_STATES 255 CACHE STRING "Maximum generated states")
set(ESCX_MAX_EVENTS 255 CACHE STRING "Maximum generated events")
set(ESCX_MAX_TRANSITIONS 65535 CACHE STRING "Maximum generated transitions")
set(ESCX_MAX_RANGES 65535 CACHE STRING "Maximum generated selector functions")
set(ESCX_MAX_SLOTS 65535 CACHE STRING "Maximum generated row-displacement slots")
set(ESCX_QUEUE_SIZE 4 CACHE STRING "Event queue depth")
option(ESCX_ENABLE_EVENT_DATA "Store event payload pointers in queue" ON)
option(ESCX_ENABLE_TIMERS "Include timer fields and timer dispatch" ON)
option(ESCX_ENABLE_ASSERTS "Include ESCXML assertions" ON)

target_compile_definitions(escxml_runtime PUBLIC
    ESCX_MAX_STATES=${ESCX_MAX_STATES}
    ESCX_MAX_EVENTS=${ESCX_MAX_EVENTS}
    ESCX_MAX_TRANSITIONS=${ESCX_MAX_TRANSITIONS}
    ESCX_MAX_RANGES=${ESCX_MAX_RANGES}
    ESCX_MAX_SLOTS=${ESCX_MAX_SLOTS}
    ESCX_QUEUE_SIZE=${ESCX_QUEUE_SIZE}
    ESCX_ENABLE_EVENT_DATA=$<BOOL:${ESCX_ENABLE_EVENT_DATA}>
    ESCX_ENABLE_TIMERS=$<BOOL:${ESCX_ENABLE_TIMERS}>
    ESCX_ENABLE_ASSERTS=$<BOOL:${ESCX_ENABLE_ASSERTS}>)

if(ESCX_BUILD_TESTS)
    enable_testing()
    add_executable(escxml_runtime_test example/SimpleMachine.c tests/test_state_machine.c)
    target_include_directories(escxml_runtime_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/example)
    target_link_libraries(escxml_runtime_test PRIVATE ESCXML::Runtime)
    set_target_properties(escxml_runtime_test PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    add_test(NAME escxml_runtime_test COMMAND escxml_runtime_test)
endif()
