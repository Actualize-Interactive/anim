#ifndef ANIM_KEYFRAME_HPP
#define ANIM_KEYFRAME_HPP

#include "point.hpp"
#include "handle_mode.hpp"
#include "function.hpp"


namespace anim {

/**
 * @brief A single control point on an animation curve.
 *
 * A keyframe is a value at a point in time together with the data needed to
 * interpolate towards its neighbours: the interpolation Function, the
 * HandleMode, and the two Bézier handles. The handles are only used when
 * the function is Function::Bezier.
 *
 * Keyframes are value types: they can be constructed, copied and compared
 * freely. To place a keyframe in a channel (which keeps them time-sorted and
 * applies handle constraints) use Channel::create_keyframe.
 */
struct Keyframe {
    Point      position;    ///< The keyframe's (time, value) position.
    Point      in_handle;   ///< Incoming Bézier handle (controls the curve arriving from the previous keyframe).
    Point      out_handle;  ///< Outgoing Bézier handle (controls the curve leaving towards the next keyframe).
    Function   function;    ///< Interpolation function for the segment starting at this keyframe.
    HandleMode handle_mode; ///< How the handles are computed and constrained.

    /**
     * @brief Constructs a keyframe from a position point.
     * @param position    The (time, value) position.
     * @param function    Interpolation function (default Function::Bezier).
     * @param handle_mode Handle mode (default HandleMode::Smooth).
     * @param in_handle   Incoming handle (defaults to the origin).
     * @param out_handle  Outgoing handle (defaults to the origin).
     */
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

    /**
     * @brief Constructs a keyframe from a time and value.
     * @param time        Time component of the position.
     * @param value       Value component of the position.
     * @param function    Interpolation function (default Function::Bezier).
     * @param handle_mode Handle mode (default HandleMode::Smooth).
     * @param in_handle   Incoming handle (defaults to the origin).
     * @param out_handle  Outgoing handle (defaults to the origin).
     */
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

    /// @brief Constructs a default keyframe at the origin (Bézier / Smooth).
    inline Keyframe()
      : position(),
        in_handle(),
        out_handle(),
        function(Function::Bezier), handle_mode(HandleMode::Smooth) {}

    inline Keyframe(const Keyframe& other) = default;
    inline Keyframe(Keyframe&& other) noexcept = default;
    inline Keyframe& operator=(const Keyframe& other) = default;
    inline Keyframe& operator=(Keyframe&& other) noexcept = default;

    /// @brief Equality across position, both handles, function and handle mode.
    inline bool operator==(const Keyframe& other) const {
        return position == other.position &&
               in_handle == other.in_handle &&
               out_handle == other.out_handle &&
               function == other.function &&
               handle_mode == other.handle_mode;
    }
    /// @brief Negation of operator==.
    inline bool operator!=(const Keyframe& other) const {
        return !(*this == other);
    }

    /// @brief Convenience accessor for the position's time component.
    inline double time() const { return position.time; }
    /// @brief Convenience accessor for the position's value component.
    inline double value() const { return position.value; }
};

} // namespace anim

#endif // ANIM_KEYFRAME_HPP
