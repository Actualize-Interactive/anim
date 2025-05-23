#ifndef ANIM_BEZIER_HANDLE_HPP
#define ANIM_BEZIER_HANDLE_HPP

#include <cmath>
#include <stdexcept>

namespace anim {

/**
 * @brief A handle point for Bézier curve control.
 * 
 * In animation contexts, this represents a control point in a
 * time-value curve, where time is the x-coordinate and value is the y-coordinate.
 */
struct BezierHandle {
    double time;   /**< The time (x-coordinate) of the point */
    double value;  /**< The value (y-coordinate) of the point */

    /**
     * @brief Construct a new BezierHandle object
     * 
     * @param time The time (x-coordinate)
     * @param value The value (y-coordinate)
     */
    BezierHandle(double time = 0.0, double value = 0.0);

    /**
     * @brief Component-wise addition of two handles
     * @param other The handle to add
     * @return A new handle with the sum of components
     */
    BezierHandle operator+(const BezierHandle& other) const;
    
    /**
     * @brief Component-wise subtraction of two handles
     * @param other The handle to subtract
     * @return A new handle with the difference of components
     */
    BezierHandle operator-(const BezierHandle& other) const;
    
    /**
     * @brief Scale a handle by a scalar value
     * @param scalar The scaling factor
     * @return A new handle with scaled components
     */
    BezierHandle operator*(double scalar) const;
    
    /**
     * @brief Divide a handle by a scalar value
     * @param scalar The divisor
     * @return A new handle with divided components
     * @throws std::domain_error if scalar is zero
     */
    BezierHandle operator/(double scalar) const;
    bool operator==(const BezierHandle& other) const;
    bool operator!=(const BezierHandle& other) const;
    
    /**
     * @brief Calculate the Euclidean length (magnitude) of the handle vector
     * @return The length of the vector from origin to this point
     */
    double length() const;
    
    /**
     * @brief Create a normalized version of this vector
     * @return A new handle with the same direction but unit length
     * @throws std::domain_error if the vector has zero length
     */
    BezierHandle normalized() const;
};

} // namespace anim

#endif // ANIM_BEZIER_HANDLE_HPP