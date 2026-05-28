# anim — Animation Curve Library

A modern C++ library for creating, managing, and evaluating animation curves
built from keyframes and cubic Bézier interpolation.

## Features

- Modern C++20 library; builds as a static (default) or shared library
- `Animation` containers holding multiple named `Channel` curves
- Three interpolation functions per keyframe:
  - `Constant` — hold the value until the next keyframe (step)
  - `Linear` — straight-line interpolation
  - `Bezier` — cubic Bézier interpolation driven by handles
- Rich Bézier handle modes (`HandleMode`): `Flat`, `Smooth`, `Aligned`,
  `Free`, and the aligned variants `AlignStrict` / `AlignFlex` / `AlignAdjustable`
- Out-of-range extrapolation per channel (`Extend`): `Hold`, `Repeat`, `Mirror`
- Stable, unique channel identity via `Id` (survives copies)
- Sampling helpers (`evaluate`, `evaluate_range`, `evaluate_range_by_rate`)
- Comprehensive Catch2 test suite

## Requirements

- A C++20-compatible compiler (MSVC 2022, GCC 11+, Clang 14+)
- CMake 3.25 or newer

## Building

Helper scripts are provided that configure, build, and run the tests:

```bash
./build.sh        # Linux / macOS
```

```powershell
.\build.ps1       # Windows (PowerShell)
```

Or drive CMake directly:

```bash
cmake -B build -S . -DANIM_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

## Integration

### As a CMake subdirectory

```cmake
add_subdirectory(path/to/anim)
target_link_libraries(your_target PRIVATE anim)
```

### Via an installed package

After installing the library, use `find_package`:

```cmake
find_package(anim REQUIRED)
target_link_libraries(your_target PRIVATE anim::anim_static)  # or anim::anim_shared
```

### CMake options

| Option | Default | Description |
| --- | --- | --- |
| `ANIM_BUILD_STATIC` | `ON` | Build the static library. |
| `ANIM_BUILD_SHARED` | `OFF` | Build the shared library. |
| `ANIM_BUILD_TESTS` | `ON` standalone, `OFF` as submodule | Build the test suite. |
| `ANIM_BUILD_EXAMPLES` | `ON` standalone, `OFF` as submodule | Build the examples. |

When `anim` is added with `add_subdirectory`, tests and examples default to
`OFF` so they don't build as part of your project.

## Basic Usage

Include the umbrella header (or the individual headers under `anim/`):

```cpp
#include <anim.hpp>
```

### Creating and evaluating a channel

Channels are created and owned by an `Animation`; you don't construct them
directly. Add keyframes with `create_keyframe`, then evaluate the curve.

```cpp
#include <anim.hpp>
#include <iostream>

int main() {
    anim::Animation animation("demo");

    // Channels are created through the animation, which keeps them sorted
    // and assigns each a unique id.
    anim::Channel& channel = animation.create_channel("value");

    // create_keyframe(time, value, function, handle_mode)
    channel.create_keyframe(0.0, 0.0, anim::Function::Linear);
    channel.create_keyframe(1.0, 1.0);                            // default: Bezier / Smooth
    channel.create_keyframe(2.0, 0.0, anim::Function::Bezier,
                                       anim::HandleMode::Flat);

    // Evaluate single times
    std::cout << "t=0.5 -> " << channel.evaluate(0.5) << "\n";
    std::cout << "t=1.5 -> " << channel.evaluate(1.5) << "\n";

    // Evaluate a range: 5 evenly spaced samples from t=0 to t=2
    for (double v : channel.evaluate_range(0.0, 2.0, 5)) {
        std::cout << v << " ";
    }
    std::cout << "\n";
    return 0;
}
```

### Keyframes with explicit Bézier handles

Handles are `anim::Point` values in `(time, value)` space:

```cpp
anim::Channel& ch = animation.create_channel("curve");

// create_keyframe(time, value, in_handle, out_handle, function, handle_mode)
ch.create_keyframe(0.0, 0.0,
    anim::Point(-0.3, 0.0),   // in-handle
    anim::Point( 0.3, 0.0),   // out-handle
    anim::Function::Bezier,
    anim::HandleMode::Aligned);
```

### Multiple channels

```cpp
anim::Animation transform("transform");

anim::Channel& pos_x = transform.create_channel("position.x");
anim::Channel& pos_y = transform.create_channel("position.y");

pos_x.create_keyframe(0.0, 0.0, anim::Function::Linear);
pos_x.create_keyframe(1.0, 10.0, anim::Function::Linear);

pos_y.create_keyframe(0.0, 0.0);
pos_y.create_keyframe(1.0, 5.0);

double x = transform.channel("position.x").evaluate(0.5);
double y = transform.channel("position.y").evaluate(0.5);
```

### Extending beyond the keyframe range

Each channel decides how to evaluate times before its first keyframe and
after its last:

```cpp
anim::Channel& ch = animation.create_channel("looping");
ch.create_keyframe(0.0, 0.0, anim::Function::Linear);
ch.create_keyframe(1.0, 1.0, anim::Function::Linear);

ch.set_extend_start(anim::Extend::Repeat);  // loop before the start
ch.set_extend_end(anim::Extend::Mirror);    // ping-pong after the end
```

## Examples

With `ANIM_BUILD_EXAMPLES` enabled, the `examples/` directory builds:

- **`test_anim_integration`** — a minimal, dependency-free program that
  exercises the core API (a good starting point for real usage).
- **`curve_visualization`** — an interactive ImGui/ImPlot viewer for inspecting
  curves and handle modes.

## Documentation

The full API reference is generated from the in-header documentation with
Doxygen and Sphinx. See `docs/` for the build scripts, and the published site
linked from the repository.

## License

Distributed under the MIT License. See the [LICENSE](LICENSE) file.

## Contributing

Contributions are welcome — please open an issue or pull request.
