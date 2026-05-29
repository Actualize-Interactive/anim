cmake_minimum_required(VERSION 3.25)
project(anim VERSION 0.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Check if this project is being built as a submodule
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    # Building as a standalone project - enable tests by default
    option(ANIM_BUILD_TESTS "Build tests" ON)
    option(ANIM_BUILD_EXAMPLES "Build examples" ON)
else()
    # Building as a submodule - disable tests by default
    option(ANIM_BUILD_TESTS "Build tests" OFF)
    option(ANIM_BUILD_EXAMPLES "Build examples" OFF)
endif()

option(ANIM_BUILD_STATIC "Build static library" ON)
option(ANIM_BUILD_SHARED "Build shared library" OFF)

# Source files
file(GLOB_RECURSE ANIM_SOURCES 
    src/*.cpp
)

# Header files (for IDE organization)
file(GLOB_RECURSE ANIM_HEADERS 
    "include/*.hpp"

)

# Static library (default)
if(ANIM_BUILD_STATIC)
    add_library(anim_static STATIC ${ANIM_SOURCES} ${ANIM_HEADERS})
    target_include_directories(anim_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    set_target_properties(anim_static PROPERTIES
        OUTPUT_NAME anim
        POSITION_INDEPENDENT_CODE ON
    )
    
    # Set anim as the main target that tests will link against
    add_library(anim ALIAS anim_static)
endif()

# Shared library (optional)
if(ANIM_BUILD_SHARED)
    add_library(anim_shared SHARED ${ANIM_SOURCES} ${ANIM_HEADERS})
    target_include_directories(anim_shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    set_target_properties(anim_shared PROPERTIES
        OUTPUT_NAME anim
        POSITION_INDEPENDENT_CODE ON
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
    
    # Only set anim as the main target if static library is not being built
    if(NOT ANIM_BUILD_STATIC)
        add_library(anim ALIAS anim_shared)
    endif()
endif()

if(ANIM_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)    

    file(GLOB_RECURSE TEST_SOURCES 
        tests/*.cpp
    )

    add_executable(anim_tests 
        ${TEST_SOURCES}
    )
    
    target_link_libraries(anim_tests PRIVATE Catch2::Catch2WithMain anim)
    include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)
    catch_discover_tests(anim_tests)
endif()

# Examples
if(ANIM_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

# Install rules
if(ANIM_BUILD_STATIC)
    install(TARGETS anim_static
        EXPORT anim-targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
    )
endif()

if(ANIM_BUILD_SHARED)
    install(TARGETS anim_shared
        EXPORT anim-targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
    )
endif()

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
