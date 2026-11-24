cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL
        https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)

if(WIN32 AND MSVC)
    set(
        gtest_force_shared_crt
        ON
        CACHE BOOL ""
        FORCE
    )
endif()

FetchContent_MakeAvailable(googletest)

file(
    GLOB_RECURSE
    ESCX_UNIT_TEST_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp"
)

add_executable(
    escxml_unit_tests
    ${ESCX_UNIT_TEST_SOURCES}
)

target_link_libraries(
    escxml_unit_tests
    PRIVATE
        ESCXML::Runtime
        GTest::gtest
        GTest::gtest_main
)

target_compile_features(
    escxml_unit_tests
    PRIVATE
        cxx_std_17
)

if(MSVC)
    target_compile_options(
        escxml_unit_tests
        PRIVATE
            /W4
            /permissive-
            /MP
    )
else()
    target_compile_options(
        escxml_unit_tests
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

include(GoogleTest)

gtest_discover_tests(
    escxml_unit_tests
    DISCOVERY_MODE POST_BUILD
    DISCOVERY_TIMEOUT 60
)