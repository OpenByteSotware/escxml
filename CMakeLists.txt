cmake_minimum_required(VERSION 3.15)
project(scxml_ansi_c_runtime LANGUAGES C)

add_library(escxml_runtime
    src/AssertHandler.c
    src/StateMachine.c
    src/StateMachineEventQueue.c)
target_include_directories(escxml_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(escxml_runtime PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)

if(MSVC)
    target_compile_options(escxml_runtime PRIVATE /W4)
else()
    target_compile_options(escxml_runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(SCXML_BUILD_TESTS "Build tests and example" ON)
if(SCXML_BUILD_TESTS)
    enable_testing()
    add_executable(escxml_runtime_test example/SimpleMachine.c tests/test_state_machine.c)
    target_include_directories(escxml_runtime_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/example)
    target_link_libraries(escxml_runtime_test PRIVATE escxml_runtime)
    set_target_properties(escxml_runtime_test PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    add_test(NAME escxml_runtime_test COMMAND escxml_runtime_test)
endif()

set(ESCX_MAX_STATES 32 CACHE STRING "Maximum generated states")
set(ESCX_MAX_EVENTS 32 CACHE STRING "Maximum generated events")
set(ESCX_MAX_TRANSITIONS 64 CACHE STRING "Maximum generated transitions")
set(ESCX_QUEUE_SIZE 4 CACHE STRING "Event queue depth")
option(ESCX_ENABLE_EVENT_DATA "Store event payload pointers in queue" ON)
target_compile_definitions(escxml_runtime PUBLIC
    ESCX_MAX_STATES=${ESCX_MAX_STATES}
    ESCX_MAX_EVENTS=${ESCX_MAX_EVENTS}
    ESCX_MAX_TRANSITIONS=${ESCX_MAX_TRANSITIONS}
    ESCX_QUEUE_SIZE=${ESCX_QUEUE_SIZE}
    ESCX_ENABLE_EVENT_DATA=$<BOOL:${ESCX_ENABLE_EVENT_DATA}>)
