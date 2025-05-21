#ifndef ANIM_POINT2D_HPP
#define ANIM_POINT2D_HPP

#include <cmath>
#include <stdexcept>

namespace anim {

/**
 * @brief A 2D point representing a time-value pair.
 * 
 * In animation contexts, this is typically used to represent a point in a
 * time-value curve, where time is the x-coordinate and value is the y-coordinate.
 */
struct Point2D {
    double time;   /**< The time (x-coordinate) of the point */
    double value;  /**< The value (y-coordinate) of the point */

    /**
     * @brief Construct a new Point2D object
     * 
     * @param time The time (x-coordinate)
     * @param value The value (y-coordinate)
     */
    Point2D(double time = 0.0, double value = 0.0);

    /**
     * @brief Component-wise addition of two points
     * @param other The point to add
     * @return A new point with the sum of components
     */
    Point2D operator+(const Point2D& other) const;
    
    /**
     * @brief Component-wise subtraction of two points
     * @param other The point to subtract
     * @return A new point with the difference of components
     */
    Point2D operator-(const Point2D& other) const;
    
    /**
     * @brief Scale a point by a scalar value
     * @param scalar The scaling factor
     * @return A new point with scaled components
     */
    Point2D operator*(double scalar) const;
    
    /**
     * @brief Divide a point by a scalar value
     * @param scalar The divisor
     * @return A new point with divided components
     * @throws std::domain_error if scalar is zero
     */
    Point2D operator/(double scalar) const;
    bool operator==(const Point2D& other) const;
    bool operator!=(const Point2D& other) const;
    
    /**
     * @brief Calculate the Euclidean length (magnitude) of the point vector
     * @return The length of the vector from origin to this point
     */
    double length() const;
    
    /**
     * @brief Create a normalized version of this vector
     * @return A new point with the same direction but unit length
     * @throws std::domain_error if the vector has zero length
     */
    Point2D normalized() const;
};

/**
 * @brief Type alias for Bézier curve handle points.
 * 
 * BezierHandle is just an alias for Point2D, representing the control
 * points that define the shape of a Bézier curve.
 */
using BezierHandle = Point2D;

} // namespace anim

#endif // ANIM_POINT2D_HPP
