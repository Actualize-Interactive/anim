#ifndef ANIM_BEZIER_UTILS_HPP
#define ANIM_BEZIER_UTILS_HPP

#include "anim/bezier_handle.hpp"
#include <vector>
#include <stdexcept>

namespace anim {

namespace bezier_utils {
    
    BezierHandle evaluate_cubic_bezier(const BezierHandle& p0, const BezierHandle& p1, const BezierHandle& p2, const BezierHandle& p3, double t);
    double find_parameter_for_time(const BezierHandle& p0, const BezierHandle& p1, const BezierHandle& p2, const BezierHandle& p3, double target_time, double precision = 1e-6, int max_iterations = 30);
    void create_linear_bezier_handles(const BezierHandle& p0, const BezierHandle& p3, BezierHandle& out_p1, BezierHandle& out_p2);
    void create_flat_bezier_handles(const BezierHandle& keyframe_point, double time_offset, BezierHandle& out_in_handle, BezierHandle& out_out_handle);
    
} // namespace bezier_utils

} // namespace anim

#endif // ANIM_BEZIER_UTILS_HPP
