# anim - Animation Curve Library

A modern C++ library for creating, managing, and evaluating animation channels based on Bézier curves and immutable keyframe objects.

## Features

- Modern C++ library (C++20 compatible)
- Static library with option for shared library build
- Immutable keyframe design with efficient update patterns
- Multiple tangent modes for animation curves:
  - flat: Horizontal tangents
  - linear: Direct linear interpolation between keyframes
  - constant: Value stays constant until the next keyframe
  - smooth: Automatically calculated smooth tangents
  - manual: Manual control of one tangent with auto-adjustment of the other
  - broken: Complete manual control of both tangents
- Cubic Bézier curve interpolation
- Support for multiple named animation channels
- Comprehensive test suite

## Requirements

- C++20 compatible compiler
- CMake 3.25 or newer (for building)

## Integration

To use `anim` in your project, you need to build and link against the library:

```cpp
#include <anim.hpp>

// Or individual components:
#include <anim/channel.hpp>
#include <anim/animation.hpp>
```

### CMake Integration

You can add `anim` as a subdirectory in your CMake project:

```cmake
add_subdirectory(path/to/anim)
target_link_libraries(your_target PRIVATE anim)
```

Or install and use with find_package:

```cmake
find_package(anim REQUIRED)
target_link_libraries(your_target PRIVATE anim::anim)
```

### CMake Options

The following CMake options are available:

```cmake
# When building as a standalone project, tests and examples are ON by default
# When building as a submodule, they are OFF by default

# Explicitly control whether to build tests
set(ANIM_BUILD_TESTS OFF CACHE BOOL "Build anim tests")

# Explicitly control whether to build examples  
set(ANIM_BUILD_EXAMPLES OFF CACHE BOOL "Build anim examples")

# Library type options
set(ANIM_BUILD_STATIC ON CACHE BOOL "Build static library")
set(ANIM_BUILD_SHARED OFF CACHE BOOL "Build shared library")

# Then include the subdirectory
add_subdirectory(path/to/anim)
```

By default, `anim` is built as a static library. You can enable building a shared library by setting the `ANIM_BUILD_SHARED` option to ON:

```cmake
set(ANIM_BUILD_SHARED ON)
add_subdirectory(path/to/anim)
```

### Building the Library

To build the library from source:

#### Windows (PowerShell)

```powershell
# Create build directory
New-Item -Path .\build -ItemType Directory -Force
Set-Location -Path .\build

# Configure using CMake
cmake ..

# Build
cmake --build .

# Run tests
ctest
```

#### Unix (Bash)

```bash
# Create build directory
mkdir -p build
cd build

# Configure using CMake
cmake ..

# Build
cmake --build .

# Run tests
ctest
```

## Basic Usage

### Creating and Evaluating an Animation Channel

```cpp
#include <anim.hpp>
#include <iostream>

int main() {    // Create an animation channel
    anim::Channel channel;
    
    // Set keyframes with different tangent modes
    // Parameters: time, value, in_tangent, out_tangent, mode
    
    // First keyframe at time 0.0 with linear mode
    channel.set_keyframe(0.0, 0.0, 
        anim::Point2D(-0.1, 0.0),  // in-tangent
        anim::Point2D(0.1, 0.0),   // out-tangent
        anim::TangentMode::linear);
    
    // Second keyframe at time 1.0 with smooth mode
    channel.set_keyframe(1.0, 1.0,
        anim::Point2D(0.9, 1.0),   // in-tangent
        anim::Point2D(1.1, 1.0),   // out-tangent
        anim::TangentMode::smooth);
    
    // Third keyframe at time 2.0 with constant mode    channel.set_keyframe(2.0, 0.0,
        anim::Point2D(1.9, 0.0),   // in-tangent
        anim::Point2D(2.1, 0.0),   // out-tangent
        anim::TangentMode::constant);
    
    // Evaluate the channel at different times
    std::cout << "t=0.0: " << channel.evaluate(0.0) << std::endl;
    std::cout << "t=0.5: " << channel.evaluate(0.5) << std::endl;
    std::cout << "t=1.0: " << channel.evaluate(1.0) << std::endl;
    std::cout << "t=1.5: " << channel.evaluate(1.5) << std::endl;
    std::cout << "t=2.0: " << channel.evaluate(2.0) << std::endl;
    
    // Evaluate a range of samples
    auto samples = channel.evaluate_range(0.0, 2.0, 5);
    std::cout << "Samples: ";
    for (double value : samples) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### Working with Multiple Animation Channels

```cpp
#include <anim.hpp>
#include <iostream>

int main() {
    // Create an animation with multiple channels
    anim::Animation animation;
    
    // Create position.x channel
    anim::Channel position_x;
    position_x.set_keyframe(0.0, 0.0, 
        anim::Point2D(-0.1, 0.0), 
        anim::Point2D(0.1, 0.0), 
        anim::TangentMode::linear);    position_x.set_keyframe(1.0, 10.0,
        anim::Point2D(0.9, 10.0), 
        anim::Point2D(1.1, 10.0), 
        anim::TangentMode::linear);
    
    // Create position.y channel
    anim::Channel position_y;
    position_y.set_keyframe(0.0, 0.0, 
        anim::Point2D(-0.1, 0.0), 
        anim::Point2D(0.1, 0.0), 
        anim::TangentMode::smooth);
    position_y.set_keyframe(0.5, 5.0,
        anim::Point2D(0.4, 5.0), 
        anim::Point2D(0.6, 5.0), 
        anim::TangentMode::smooth);
    position_y.set_keyframe(1.0, 0.0,
        anim::Point2D(0.9, 0.0), 
        anim::Point2D(1.1, 0.0), 
        anim::TangentMode::smooth);
    
    // Add channels to animation
    animation.add_channel("position.x", position_x);
    animation.add_channel("position.y", position_y);
    
    // Evaluate all channels at specific times
    std::cout << "Positions at t=0.25:" << std::endl;
    auto pos_025 = animation.evaluate_channels(0.25);
    std::cout << "  x: " << pos_025["position.x"] << std::endl;
    std::cout << "  y: " << pos_025["position.y"] << std::endl;
      std::cout << "Positions at t=0.75:" << std::endl;
    auto pos_075 = animation.evaluate_channels(0.75);
    std::cout << "  x: " << pos_075["position.x"] << std::endl;
    std::cout << "  y: " << pos_075["position.y"] << std::endl;
    
    return 0;
}
```

## Examples

The library includes examples that demonstrate advanced usage:

### Curve Visualization

The `curve_visualization.cpp` example shows how to visualize animation curves using ASCII art in the terminal:

```cpp
#include <anim.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

// Helper function to print an animation curve as ASCII art
void print_curve_ascii(const anim::Channel& channel, double start_time, double end_time, int width = 80, int height = 20) {
    // Sample the curve
    std::vector<double> samples = channel.evaluate_range(start_time, end_time, width);
    
    // Find min and max values to scale the output
    double min_val = samples[0];
    double max_val = samples[0];
    for (double val : samples) {
        min_val = std::min(min_val, val);
        max_val = std::max(max_val, val);
    }
    
    // Print the curve
    // ...
}

int main() {
    // Create a channel with various tangent modes
    anim::Channel channel;
    
    // Add keyframes with different tangent modes
    channel.set_keyframe(0.0, 0.0, 
        anim::Point2D(-0.1, 0.0), 
        anim::Point2D(0.1, 0.0), 
        anim::TangentMode::linear);
    
    channel.set_keyframe(1.0, 1.0, 
        anim::Point2D(0.9, 1.0), 
        anim::Point2D(1.1, 1.0), 
        anim::TangentMode::smooth);
    
    // Visualize the curve
    print_curve_ascii(channel, 0.0, 1.0);
    
    return 0;
}
```

To build and run the examples, build the library with the `ANIM_BUILD_EXAMPLES` option set to ON (it's ON by default).

## License

This library is distributed under the MIT License. See the LICENSE file for more information.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
