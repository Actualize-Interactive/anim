#ifndef ANIM_POINT_HPP
#define ANIM_POINT_HPP

#include <stdexcept>

namespace anim {

/**
 * @brief A point in (time, value) space.
 *
 * A Point is the fundamental coordinate type of the library. Its two
 * components are named @c time (the horizontal axis of an animation curve)
 * and @c value (the vertical axis). The same type doubles as a 2D vector for
 * direction and displacement math; see the Vector alias.
 */
struct Point {
    double time;   ///< Position along the time (horizontal) axis.
    double value;  ///< Position along the value (vertical) axis.

    /**
     * @brief Constructs a point, defaulting to the origin (0, 0).
     * @param time  Time component.
     * @param value Value component.
     */
    Point(double time = 0.0, double value = 0.0) : time(time), value(value) {}

    /// @brief Component-wise addition.
    Point operator+(const Point& other) const {
        return Point(time + other.time, value + other.value);
    }

    /// @brief Component-wise subtraction.
    Point operator-(const Point& other) const {
        return Point(time - other.time, value - other.value);
    }

    /// @brief Scales both components by @p scalar.
    Point operator*(double scalar) const {
        return Point(time * scalar, value * scalar);
    }

    /**
     * @brief Divides both components by @p scalar.
     * @param scalar Divisor.
     * @return The scaled point.
     * @throws std::domain_error if @p scalar is zero.
     */
    Point operator/(double scalar) const {
        if (scalar == 0.0) {
            throw std::domain_error("Division by zero");
        }
        return Point(time / scalar, value / scalar);
    }
    /// @brief Exact (bit-for-bit) equality of both components.
    bool operator==(const Point& other) const {
        return time == other.time && value == other.value;
    }
    /// @brief Negation of operator==.
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
    /// @brief Component-wise compound addition.
    Point& operator+=(const Point& other) {
        time += other.time;
        value += other.value;
        return *this;
    }
    /// @brief Component-wise compound subtraction.
    Point& operator-=(const Point& other) {
        time -= other.time;
        value -= other.value;
        return *this;
    }
    /// @brief Compound scale by @p scalar.
    Point& operator*=(double scalar) {
        time *= scalar;
        value *= scalar;
        return *this;
    }
    /**
     * @brief Compound divide by @p scalar.
     * @param scalar Divisor.
     * @return Reference to this point after scaling.
     * @throws std::domain_error if @p scalar is zero.
     */
    Point& operator/=(double scalar) {
        if (scalar == 0.0) {
            throw std::domain_error("Division by zero");
        }
        time /= scalar;
        value /= scalar;
        return *this;
    }
    /// @brief Returns true if both components are exactly zero.
    bool is_zero() const {
        return time == 0.0 && value == 0.0;
    }
    /// @brief Resets both components to zero.
    void reset() {
        time = 0.0;
        value = 0.0;
    }
    /// @brief Sets both components in one call.
    void set(double new_time, double new_value) {
        time = new_time;
        value = new_value;
    }
};

/// @brief Alias used when a Point represents a direction or displacement rather than a position.
using Vector = Point;

} // namespace anim

#endif // ANIM_POINT_HPP
