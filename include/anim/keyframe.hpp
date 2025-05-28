#ifndef ANIM_KEYFRAME_HPP
#define ANIM_KEYFRAME_HPP

#include "point.hpp"
#include "handle_mode.hpp"
#include "function.hpp"


namespace anim {

struct Keyframe {
    Point        position;
    Point        in_handle;
    Point        out_handle;
    Function     function;
    HandleMode   handle_mode;
    Keyframe(const Point& position, 
          Function function = Function::bezier,
          HandleMode handle_mode = HandleMode::smooth,
          const Point& in_handle = Point(),
          const Point& out_handle = Point())
      : position(position),
        in_handle(in_handle),
        out_handle(out_handle),
        function(function),
        handle_mode(handle_mode) {}

    Keyframe(double time, double value, 
          Function function = Function::bezier,
          HandleMode handle_mode = HandleMode::smooth,
          const Point& in_handle = Point(),
          const Point& out_handle = Point())
      : position(time, value),
        in_handle(in_handle),
        out_handle(out_handle),
        function(function),
        handle_mode(handle_mode) {}

    Keyframe() 
      : position(), 
        in_handle(), 
        out_handle(), 
        function(Function::bezier), handle_mode(HandleMode::smooth) {}

    Keyframe(const Keyframe& other) = default;
    Keyframe(Keyframe&& other) noexcept = default;
    Keyframe& operator=(const Keyframe& other) = default;
    Keyframe& operator=(Keyframe&& other) noexcept = default;

    bool operator==(const Keyframe& other) const {
        return position == other.position &&
               in_handle == other.in_handle &&
               out_handle == other.out_handle &&
               function == other.function &&
               handle_mode == other.handle_mode;
    }
    bool operator!=(const Keyframe& other) const {
        return !(*this == other);
    }

    double time() const { return position.time; }
    double value() const { return position.value; }
};

} // namespace anim

#endif // ANIM_KEYFRAME_HPP
