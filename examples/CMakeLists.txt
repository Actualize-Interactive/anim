cmake_minimum_required(VERSION 3.14)
project(anim_examples CXX)

# Add the examples
add_executable(curve_visualization curve_visualization.cpp)

# Include the anim library
target_include_directories(curve_visualization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
