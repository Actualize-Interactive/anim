#ifndef ANIM_BEZIER_UTILS_HPP
#define ANIM_BEZIER_UTILS_HPP

#include "anim/point2d.hpp"
#include <vector>
#include <stdexcept>

namespace anim {

/**
 * @brief Utilities for working with Bézier curves.
 * 
 * This namespace contains functions for evaluating and manipulating cubic Bézier curves,
 * which are used to create smooth animation curves between keyframes.
 */
namespace bezier_utils {
    
    /**
     * @brief Evaluate a cubic Bézier curve at a specific parameter value.
     * 
     * @param p0 The first anchor point (start point)
     * @param p1 The first control point
     * @param p2 The second control point
     * @param p3 The second anchor point (end point)
     * @param t The parameter value, typically in range [0,1]
     * @return The point on the Bézier curve at parameter t
     */
    Point2D evaluate_cubic_bezier(
        const Point2D& p0, const Point2D& p1,
        const Point2D& p2, const Point2D& p3,
        double t);
    
    /**
     * @brief Find the parameter t that corresponds to a specific time value on a Bézier curve.
     * 
     * This is useful for non-uniform Bézier curves where the time (x) value doesn't
     * change linearly with the parameter t.
     * 
     * @param p0 The first anchor point (start point)
     * @param p1 The first control point
     * @param p2 The second control point
     * @param p3 The second anchor point (end point)
     * @param target_time The time value to find on the curve
     * @param precision The maximum allowed error
     * @param max_iterations Maximum number of iterations for numerical solution
     * @return The parameter t that gives the target time value
     */
    double find_parameter_for_time(
        const Point2D& p0, const Point2D& p1,
        const Point2D& p2, const Point2D& p3,
        double target_time,
        double precision = 1e-6,
        int max_iterations = 30);
    
    /**
     * @brief Create control points for a linear Bézier curve.
     * 
     * Sets the control points to create a straight line between the anchor points.
     * 
     * @param p0 The first anchor point (start point)
     * @param p3 The second anchor point (end point)
     * @param out_p1 Output parameter for the first control point
     * @param out_p2 Output parameter for the second control point
     */
    void create_linear_bezier_handles(
        const Point2D& p0, const Point2D& p3,
        Point2D& out_p1, Point2D& out_p2);
    
    /**
     * @brief Create flat tangent handles for a keyframe.
     * 
     * Sets the incoming and outgoing handles to create horizontal tangents,
     * resulting in a curve that approaches the keyframe with zero slope.
     * 
     * @param keyframe_point The keyframe point
     * @param time_offset The horizontal distance from the keyframe to the handles
     * @param out_in_handle Output parameter for the incoming handle
     * @param out_out_handle Output parameter for the outgoing handle
     */
    void create_flat_bezier_handles(
        const Point2D& keyframe_point, 
        double time_offset,
        Point2D& out_in_handle, 
        Point2D& out_out_handle);
    
} // namespace bezier_utils

} // namespace anim

#endif // ANIM_BEZIER_UTILS_HPP
