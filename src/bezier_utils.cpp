#include "anim/bezier_utils.hpp"

namespace anim {

namespace bezier_utils {

Point evaluate_cubic_bezier(
    const Point& p0, const Point& p1,
    const Point& p2, const Point& p3,
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

double find_parameter_for_time(
    const Point& p0, const Point& p1,
    const Point& p2, const Point& p3,
    double target_time,
    double precision,
    int max_iterations) {
    
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
        Point point = evaluate_cubic_bezier(p0, p1, p2, p3, t);
        
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

void create_linear_bezier_handles(
    const Point& p0, const Point& p3,
    Point& out_p1, Point& out_p2) {
    
    // For a true linear segment, we'd set P1=P0 and P2=P3, but
    // for better visualization, place handles at 1/3 along segment
    double t_diff = p3.time - p0.time;
    double v_diff = p3.value - p0.value;
    
    out_p1 = Point(p0.time + t_diff / 3.0, p0.value + v_diff / 3.0);
    out_p2 = Point(p3.time - t_diff / 3.0, p3.value - v_diff / 3.0);
}

void create_flat_bezier_handles(
    const Point& keyframe_point, 
    double time_offset,
    Point& out_in_handle, 
    Point& out_out_handle) {
    
    out_in_handle = Point(keyframe_point.time - time_offset, keyframe_point.value);
    out_out_handle = Point(keyframe_point.time + time_offset, keyframe_point.value);
}

} // namespace bezier_utils

} // namespace anim
