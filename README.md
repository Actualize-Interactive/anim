# anim - Animation Curve Library

A modern C++ header-only library for creating, managing, and evaluating animation channels based on Bézier curves and immutable keyframe objects.

## Features

- Header-only C++ library (C++20 compatible)
- Immutable keyframe design with efficient update patterns
- Multiple tangent modes for animation curves:
  - flat: Horizontal tangents
  - linear: Direct linear interpolation between keyframes
  - stepped: Value stays constant until the next keyframe
  - smoothAuto: Automatically calculated smooth tangents
  - smoothManual: Manual control of one tangent with auto-adjustment of the other
  - broken: Complete manual control of both tangents
- Cubic Bézier curve interpolation
- Support for multiple named animation channels
- Comprehensive test suite

## Requirements

- C++17 compatible compiler
- CMake 3.14 or newer (for building tests)

## Integration

Since `anim` is a header-only library, you can simply include the headers in your project:

```cpp
#include <anim.h>

// Or individual components:
#include <anim/animation_channel.h>
#include <anim/animation.h>
```

### CMake Integration

Add as a subdirectory:

```cmake
add_subdirectory(path/to/anim)
target_link_libraries(your_target PRIVATE anim)
```

Or install and use with find_package:

```cmake
find_package(anim REQUIRED)
target_link_libraries(your_target PRIVATE anim::anim)
```

## Basic Usage

### Creating and Evaluating an Animation Channel

```cpp
#include <anim.h>
#include <iostream>

int main() {
    // Create an animation channel
    anim::AnimationChannel channel;
    
    // Set keyframes with different tangent modes
    // Parameters: time, value, in_tangent, out_tangent, mode
    
    // First keyframe at time 0.0 with linear mode
    channel.set_keyframe(0.0, 0.0, 
        anim::Point2D(-0.1, 0.0),  // in-tangent
        anim::Point2D(0.1, 0.0),   // out-tangent
        anim::TangentMode::linear);
    
    // Second keyframe at time 1.0 with smoothAuto mode
    channel.set_keyframe(1.0, 1.0,
        anim::Point2D(0.9, 1.0),   // in-tangent
        anim::Point2D(1.1, 1.0),   // out-tangent
        anim::TangentMode::smoothAuto);
    
    // Third keyframe at time 2.0 with stepped mode
    channel.set_keyframe(2.0, 0.0,
        anim::Point2D(1.9, 0.0),   // in-tangent
        anim::Point2D(2.1, 0.0),   // out-tangent
        anim::TangentMode::stepped);
    
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
#include <anim.h>
#include <iostream>

int main() {
    // Create an animation with multiple channels
    anim::Animation animation;
    
    // Create position.x channel
    anim::AnimationChannel position_x;
    position_x.set_keyframe(0.0, 0.0, 
        anim::Point2D(-0.1, 0.0), 
        anim::Point2D(0.1, 0.0), 
        anim::TangentMode::linear);
    position_x.set_keyframe(1.0, 10.0,
        anim::Point2D(0.9, 10.0), 
        anim::Point2D(1.1, 10.0), 
        anim::TangentMode::linear);
    
    // Create position.y channel
    anim::AnimationChannel position_y;
    position_y.set_keyframe(0.0, 0.0, 
        anim::Point2D(-0.1, 0.0), 
        anim::Point2D(0.1, 0.0), 
        anim::TangentMode::smoothAuto);
    position_y.set_keyframe(0.5, 5.0,
        anim::Point2D(0.4, 5.0), 
        anim::Point2D(0.6, 5.0), 
        anim::TangentMode::smoothAuto);
    position_y.set_keyframe(1.0, 0.0,
        anim::Point2D(0.9, 0.0), 
        anim::Point2D(1.1, 0.0), 
        anim::TangentMode::smoothAuto);
    
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
