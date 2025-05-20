#ifndef ANIM_POINT2D_H
#define ANIM_POINT2D_H

#include <cmath>
#include <stdexcept>

namespace anim {

/**
 * @brief Represents a 2D point, typically used for (time, value) coordinates.
 * 
 * This struct is commonly used for representing keyframe positions and Bézier handle positions.
 */
struct Point2D {
    double time;  ///< X-coordinate, typically representing time
    double value; ///< Y-coordinate, typically representing the animated value

    /**
     * @brief Constructor for a 2D point
     * 
     * @param time X-coordinate, typically representing time
     * @param value Y-coordinate, typically representing the animated value
     */
    Point2D(double time = 0.0, double value = 0.0) : time(time), value(value) {}

    /**
     * @brief Addition operator for Point2D
     * 
     * @param other The point to add
     * @return Point2D The result of the addition
     */
    Point2D operator+(const Point2D& other) const {
        return Point2D(time + other.time, value + other.value);
    }

    /**
     * @brief Subtraction operator for Point2D
     * 
     * @param other The point to subtract
     * @return Point2D The result of the subtraction
     */
    Point2D operator-(const Point2D& other) const {
        return Point2D(time - other.time, value - other.value);
    }

    /**
     * @brief Multiplication operator for Point2D with scalar
     * 
     * @param scalar The scalar to multiply by
     * @return Point2D The result of the multiplication
     */
    Point2D operator*(double scalar) const {
        return Point2D(time * scalar, value * scalar);
    }

    /**
     * @brief Division operator for Point2D with scalar
     * 
     * @param scalar The scalar to divide by
     * @return Point2D The result of the division
     * @throws std::invalid_argument if scalar is zero
     */
    Point2D operator/(double scalar) const {
        if (std::abs(scalar) < 1e-10) {
            throw std::invalid_argument("Division by zero or near-zero in Point2D");
        }
        return Point2D(time / scalar, value / scalar);
    }

    /**
     * @brief Equality operator for Point2D
     * 
     * @param other The point to compare with
     * @return bool True if the points are equal, false otherwise
     */
    bool operator==(const Point2D& other) const {
        constexpr double epsilon = 1e-10;
        return std::abs(time - other.time) < epsilon && 
               std::abs(value - other.value) < epsilon;
    }

    /**
     * @brief Inequality operator for Point2D
     * 
     * @param other The point to compare with
     * @return bool True if the points are not equal, false otherwise
     */
    bool operator!=(const Point2D& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Type alias for Bézier handle, used for clarity when referring to Bézier control points
 */
using BezierHandle = Point2D;

} // namespace anim

#endif // ANIM_POINT2D_H
