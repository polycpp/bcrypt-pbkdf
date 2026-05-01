cmake_minimum_required(VERSION 3.20)
project(polycpp_bcrypt_pbkdf VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(POLYCPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(POLYCPP_SOURCE_DIR "" CACHE PATH "Path to a local polycpp checkout")

if(POLYCPP_SOURCE_DIR AND EXISTS "${POLYCPP_SOURCE_DIR}/CMakeLists.txt")
    FetchContent_Declare(polycpp SOURCE_DIR "${POLYCPP_SOURCE_DIR}")
else()
    FetchContent_Declare(
        polycpp
        GIT_REPOSITORY https://github.com/enricohuang/polycpp.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
endif()
FetchContent_MakeAvailable(polycpp)

add_library(polycpp_bcrypt_pbkdf
    src/bcrypt_pbkdf.cpp
)
target_link_libraries(polycpp_bcrypt_pbkdf PUBLIC polycpp)
target_include_directories(polycpp_bcrypt_pbkdf PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(polycpp::bcrypt_pbkdf ALIAS polycpp_bcrypt_pbkdf)

set(POLYCPP_BCRYPT_PBKDF_BUILD_TESTS_DEFAULT OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(POLYCPP_BCRYPT_PBKDF_BUILD_TESTS_DEFAULT ON)
endif()
option(POLYCPP_BCRYPT_PBKDF_BUILD_TESTS "Build tests" ${POLYCPP_BCRYPT_PBKDF_BUILD_TESTS_DEFAULT})
if(POLYCPP_BCRYPT_PBKDF_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(polycpp_bcrypt_pbkdf_test_smoke tests/test_smoke.cpp)
    target_link_libraries(polycpp_bcrypt_pbkdf_test_smoke PRIVATE polycpp::bcrypt_pbkdf GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(polycpp_bcrypt_pbkdf_test_smoke)
endif()

option(POLYCPP_BCRYPT_PBKDF_BUILD_EXAMPLES "Build examples" OFF)
if(POLYCPP_BCRYPT_PBKDF_BUILD_EXAMPLES)
    file(GLOB POLYCPP_BCRYPT_PBKDF_EXAMPLES CONFIGURE_DEPENDS examples/*.cpp)
    foreach(example_source IN LISTS POLYCPP_BCRYPT_PBKDF_EXAMPLES)
        get_filename_component(example_name "${example_source}" NAME_WE)
        set(example_target "polycpp_bcrypt_pbkdf_example_${example_name}")
        add_executable("${example_target}" "${example_source}")
        target_link_libraries("${example_target}" PRIVATE polycpp::bcrypt_pbkdf)
        set_target_properties("${example_target}" PROPERTIES
            OUTPUT_NAME "${example_name}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
        )
    endforeach()
endif()
