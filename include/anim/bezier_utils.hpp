#ifndef ANIM_BEZIER_UTILS_HPP
#define ANIM_BEZIER_UTILS_HPP

#include "anim/point.hpp"
#include <vector>
#include <stdexcept>

namespace anim {

namespace bezier_utils {
 
Point evaluate_cubic_bezier(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t);   
double evaluate_bezier_time_component(const Point& P0, const Point& P1, const Point& P2, const Point& P3, double t);
double evaluate_bezier_time_derivative(const Point& P0, const Point& P1, const Point& P2, const Point& P3, double t);

double solve_t_for_time_bisection(
    const Point& p0, const Point& p1, 
    const Point& p2, const Point& p3, 
    double target_time,
    double t_min = 0.0,
    double t_max = 1.0,
    double precision = 1e-6, 
    int max_iterations = 30
);

double solve_t_for_time(
    const Point& P0, const Point& P1, 
    const Point& P2, const Point& P3, 
    double target_time,
    double* initial_guess = nullptr,
    double precision = 1e-6,
    int max_iterations = 4
);

void create_linear_bezier_handles(const Point& p0, const Point& p3, Point& out_p1, Point& out_p2);
void create_flat_bezier_handles(const Point& keyframe_point, double time_offset, Point& out_in_handle, Point& out_out_handle);

} // namespace bezier_utils

} // namespace anim

#endif // ANIM_BEZIER_UTILS_HPP
