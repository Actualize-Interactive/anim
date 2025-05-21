cmake_minimum_required(VERSION 3.14)
project(anim_examples CXX)

# Add the examples
add_executable(curve_visualization curve_visualization.cpp)

# Link with the anim library
target_link_libraries(curve_visualization PRIVATE anim)
