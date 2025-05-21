#ifndef ANIM_BEZIER_UTILS_HPP
#define ANIM_BEZIER_UTILS_HPP

#include "anim/point2d.hpp"
#include <vector>
#include <stdexcept>

namespace anim {

namespace bezier_utils {
    
    // Evaluate a cubic Bézier curve at a specific parameter value
    Point2D evaluate_cubic_bezier(
        const Point2D& p0, const Point2D& p1,
        const Point2D& p2, const Point2D& p3,
        double t);
    
    // Find the parameter t that gives the X value (typically time) for a Bézier curve
    double find_parameter_for_time(
        const Point2D& p0, const Point2D& p1,
        const Point2D& p2, const Point2D& p3,
        double target_time,
        double precision = 1e-6,
        int max_iterations = 30);
    
    // Creates a Linear curve by setting handle points to create a straight line
    void create_linear_bezier_handles(
        const Point2D& p0, const Point2D& p3,
        Point2D& out_p1, Point2D& out_p2);
    
    // Creates Flat handles by setting horizontal tangents
    void create_flat_bezier_handles(
        const Point2D& keyframe_point, 
        double time_offset,
        Point2D& out_in_handle, 
        Point2D& out_out_handle);
    
} // namespace bezier_utils

} // namespace anim

#endif // ANIM_BEZIER_UTILS_HPP
