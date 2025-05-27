#ifndef ANIM_KEYFRAME_HPP
#define ANIM_KEYFRAME_HPP

#include "point.hpp"
#include "handle_type.hpp"
#include "function_type.hpp"


namespace anim {

struct Keyframe {
    Point        position;
    Point        in_handle;
    Point        out_handle;
    FunctionType function_type;
    HandleType   handle_type;
    Keyframe(const Point& pos, const Point& in, const Point& out,
             FunctionType func_type = FunctionType::bezier,
             HandleType handle_type = HandleType::smooth)
        : position(pos),
          in_handle(in), out_handle(out),
          function_type(func_type), handle_type(handle_type) {}

    Keyframe(double time, double value,
             const Point& in_handle = Point(),
             const Point& out_handle = Point(),
             FunctionType function_type = FunctionType::bezier,
             HandleType handle_type = HandleType::smooth)
        : position(time, value),
          in_handle(in_handle),
          out_handle(out_handle),
          function_type(function_type),
          handle_type(handle_type) {}

    Keyframe(double time = 0.0, double value = 0.0,
             double in_handle_time = 0.0, double in_handle_value = 0.0,
             double out_handle_time = 0.0, double out_handle_value = 0.0,
             FunctionType function_type = FunctionType::bezier,
             HandleType handle_type = HandleType::smooth)
        : position(time, value),
          in_handle(in_handle_time, in_handle_value),
          out_handle(out_handle_time, out_handle_value),
          function_type(function_type),
          handle_type(handle_type) {}



    Keyframe(const Keyframe& other) = default;
    Keyframe(Keyframe&& other) noexcept = default;
    Keyframe& operator=(const Keyframe& other) = default;
    Keyframe& operator=(Keyframe&& other) noexcept = default;

    bool operator==(const Keyframe& other) const {
        return position == other.position &&
               in_handle == other.in_handle &&
               out_handle == other.out_handle &&
               function_type == other.function_type &&
               handle_type == other.handle_type;
    }
    bool operator!=(const Keyframe& other) const {
        return !(*this == other);
    }

    double time() const { return position.time; }
    double value() const { return position.value; }
};

} // namespace anim

#endif // ANIM_KEYFRAME_HPP
