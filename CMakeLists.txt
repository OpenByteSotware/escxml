cmake_minimum_required(VERSION 3.15)
project(scxml_ansi_c_runtime LANGUAGES C)

add_library(scxml_state_machine
    src/AssertHandler.c
    src/StateMachine.c
    src/StateMachineEventQueue.c)
target_include_directories(scxml_state_machine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(scxml_state_machine PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)

if(MSVC)
    target_compile_options(scxml_state_machine PRIVATE /W4)
else()
    target_compile_options(scxml_state_machine PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(SCXML_BUILD_TESTS "Build tests and example" ON)
if(SCXML_BUILD_TESTS)
    enable_testing()
    add_executable(scxml_state_machine_test example/SimpleMachine.c tests/test_state_machine.c)
    target_include_directories(scxml_state_machine_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/example)
    target_link_libraries(scxml_state_machine_test PRIVATE scxml_state_machine)
    set_target_properties(scxml_state_machine_test PROPERTIES C_STANDARD 90 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    add_test(NAME scxml_state_machine_test COMMAND scxml_state_machine_test)
endif()
