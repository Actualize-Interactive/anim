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
    inline Keyframe(const Point& position, 
          Function function = Function::Bezier,
          HandleMode handle_mode = HandleMode::Smooth,
          const Point& in_handle = Point(),
          const Point& out_handle = Point())
      : position(position),
        in_handle(in_handle),
        out_handle(out_handle),
        function(function),
        handle_mode(handle_mode) {}

    inline Keyframe(double time, double value, 
          Function function = Function::Bezier,
          HandleMode handle_mode = HandleMode::Smooth,
          const Point& in_handle = Point(),
          const Point& out_handle = Point())
      : position(time, value),
        in_handle(in_handle),
        out_handle(out_handle),
        function(function),
        handle_mode(handle_mode) {}

    inline Keyframe() 
      : position(), 
        in_handle(), 
        out_handle(), 
        function(Function::Bezier), handle_mode(HandleMode::Smooth) {}

    inline Keyframe(const Keyframe& other) = default;
    inline Keyframe(Keyframe&& other) noexcept = default;
    inline Keyframe& operator=(const Keyframe& other) = default;
    inline Keyframe& operator=(Keyframe&& other) noexcept = default;

    inline bool operator==(const Keyframe& other) const {
        return position == other.position &&
               in_handle == other.in_handle &&
               out_handle == other.out_handle &&
               function == other.function &&
               handle_mode == other.handle_mode;
    }
    inline bool operator!=(const Keyframe& other) const {
        return !(*this == other);
    }

    inline double time() const { return position.time; }
    inline double value() const { return position.value; }
};

} // namespace anim

#endif // ANIM_KEYFRAME_HPP
