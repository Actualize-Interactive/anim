#ifndef ANIM_BEZIER_UTILS_H
#define ANIM_BEZIER_UTILS_H

#include "point2d.h"
#include <vector>
#include <stdexcept>

namespace anim {

/**
 * @brief Utility functions for Bézier curve calculations
 */
namespace bezier_utils {
    
    /**
     * @brief Evaluate a cubic Bézier curve at a specific parameter value
     * 
     * @param p0 First control point
     * @param p1 Second control point
     * @param p2 Third control point
     * @param p3 Fourth control point
     * @param t Parameter value in range [0, 1]
     * @return Point2D The point on the curve at parameter t
     */
    inline Point2D evaluate_cubic_bezier(
        const Point2D& p0, const Point2D& p1,
        const Point2D& p2, const Point2D& p3,
        double t) {
        
        if (t < 0.0 || t > 1.0) {
            throw std::invalid_argument("Bézier parameter t must be in range [0, 1]");
        }
        
        // Calculate Bézier basis functions
        double t2 = t * t;
        double t3 = t2 * t;
        double mt = 1.0 - t;
        double mt2 = mt * mt;
        double mt3 = mt2 * mt;
        
        // Calculate point using the Bézier formula
        return p0 * mt3 + 
               p1 * (3.0 * mt2 * t) + 
               p2 * (3.0 * mt * t2) + 
               p3 * t3;
    }
    
    /**
     * @brief Find the parameter t that gives the X value (typically time) for a Bézier curve
     * 
     * This is used to find the parameter t where the Bézier curve's time coordinate
     * matches a desired time, which is then used to evaluate the value.
     * 
     * @param p0 First control point
     * @param p1 Second control point
     * @param p2 Third control point
     * @param p3 Fourth control point
     * @param target_time The time value to find
     * @param precision The precision for the binary search
     * @param max_iterations Maximum iterations for the binary search
     * @return double The parameter t where the curve's time is approximately target_time
     */
    inline double find_parameter_for_time(
        const Point2D& p0, const Point2D& p1,
        const Point2D& p2, const Point2D& p3,
        double target_time,
        double precision = 1e-6,
        int max_iterations = 30) {
        
        // For efficiency, check if target_time is at one of the endpoints
        if (std::abs(target_time - p0.time) < precision) return 0.0;
        if (std::abs(target_time - p3.time) < precision) return 1.0;
        
        // Check if target_time is out of range
        if (target_time < p0.time || target_time > p3.time) {
            throw std::out_of_range("Target time is outside the curve segment");
        }
        
        // Binary search to find the parameter t
        double t_min = 0.0;
        double t_max = 1.0;
        double t = 0.5;
        
        for (int i = 0; i < max_iterations; ++i) {
            Point2D point = evaluate_cubic_bezier(p0, p1, p2, p3, t);
            
            double diff = point.time - target_time;
            
            if (std::abs(diff) < precision) {
                break;
            }
            
            if (diff < 0) {
                t_min = t;
            } else {
                t_max = t;
            }
            
            t = (t_min + t_max) / 2.0;
        }
        
        return t;
    }
    
    /**
     * @brief Creates a Linear curve by setting handle points to create a straight line
     * 
     * @param p0 First keyframe point
     * @param p3 Second keyframe point
     * @param out_p1 Output for first handle point
     * @param out_p2 Output for second handle point
     */
    inline void create_linear_bezier_handles(
        const Point2D& p0, const Point2D& p3,
        Point2D& out_p1, Point2D& out_p2) {
        
        // For a true linear segment, we'd set P1=P0 and P2=P3, but
        // for better visualization, place handles at 1/3 along segment
        double t_diff = p3.time - p0.time;
        double v_diff = p3.value - p0.value;
        
        out_p1 = Point2D(p0.time + t_diff / 3.0, p0.value + v_diff / 3.0);
        out_p2 = Point2D(p3.time - t_diff / 3.0, p3.value - v_diff / 3.0);
    }
    
    /**
     * @brief Creates Flat handles by setting horizontal tangents
     * 
     * @param keyframe_point The keyframe point
     * @param time_offset The time offset for handles
     * @param out_in_handle Output for in-tangent handle
     * @param out_out_handle Output for out-tangent handle
     */
    inline void create_flat_bezier_handles(
        const Point2D& keyframe_point, 
        double time_offset,
        Point2D& out_in_handle, 
        Point2D& out_out_handle) {
        
        out_in_handle = Point2D(keyframe_point.time - time_offset, keyframe_point.value);
        out_out_handle = Point2D(keyframe_point.time + time_offset, keyframe_point.value);
    }
    
} // namespace bezier_utils

} // namespace anim

#endif // ANIM_BEZIER_UTILS_H
