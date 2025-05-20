cmake_minimum_required(VERSION 3.14)
project(anim VERSION 0.1.0 LANGUAGES CXX)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(ANIM_BUILD_TESTS "Build tests" ON)
option(ANIM_BUILD_EXAMPLES "Build examples" ON)

# The library target (header-only)
add_library(anim INTERFACE)
target_include_directories(anim 
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Tests
if(ANIM_BUILD_TESTS)
    # Enable testing
    enable_testing()
    
    # Include FetchContent for getting Catch2
    include(FetchContent)
    
    # Fetch Catch2 (v3 for modern C++ testing)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0 # Use a specific version for reproducibility
    )
    FetchContent_MakeAvailable(Catch2)
      # Add the test executable
    add_executable(anim_tests 
        tests/test_main.cpp
        tests/test_point2d.cpp
        tests/test_keyframe.cpp
        tests/test_animation_channel.cpp
        tests/test_animation.cpp
        tests/test_bezier_utils.cpp
    )
    
    # Link against Catch2 and our anim library
    target_link_libraries(anim_tests PRIVATE Catch2::Catch2WithMain anim)
    
    # Include the Catch2 CMake scripts
    include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)
    
    # Register tests
    catch_discover_tests(anim_tests)
endif()

# Examples
if(ANIM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Install rules
install(TARGETS anim
    EXPORT anim-targets
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT anim-targets
    FILE anim-targets.cmake
    NAMESPACE anim::
    DESTINATION lib/cmake/anim
)

# Create and install the config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/anim-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/anim-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/anim-config.cmake"
    INSTALL_DESTINATION lib/cmake/anim
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/anim-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/anim-config-version.cmake"
    DESTINATION lib/cmake/anim
)
