#ifndef ANIM_BEZIER_UTILS_HPP
#define ANIM_BEZIER_UTILS_HPP

#include "anim/point.hpp"
#include <vector>
#include <stdexcept>

namespace anim {

namespace bezier_utils {
    
    Point evaluate_cubic_bezier(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t);
    double find_parameter_for_time(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double target_time, double precision = 1e-6, int max_iterations = 30);
    void create_linear_bezier_handles(const Point& p0, const Point& p3, Point& out_p1, Point& out_p2);
    void create_flat_bezier_handles(const Point& keyframe_point, double time_offset, Point& out_in_handle, Point& out_out_handle);
    
} // namespace bezier_utils

} // namespace anim

#endif // ANIM_BEZIER_UTILS_HPP
