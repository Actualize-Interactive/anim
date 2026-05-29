#ifndef ANIM_BEZIER_UTILS_HPP
#define ANIM_BEZIER_UTILS_HPP

#include "anim/point.hpp"
#include <vector>
#include <stdexcept>

namespace anim {

/// @brief Low-level cubic Bézier evaluation and parameter-solving helpers.
namespace bezier_utils {

/**
 * @brief Evaluates a cubic Bézier curve at parameter @p t.
 * @param p0 Start anchor point.
 * @param p1 First control point (start's out-handle).
 * @param p2 Second control point (end's in-handle).
 * @param p3 End anchor point.
 * @param t Curve parameter; returns @p p0 at 0 and @p p3 at 1.
 * @return The point on the curve at @p t.
 */
Point evaluate_cubic_bezier(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t);

/// @brief Evaluates only the time (x) component of a cubic Bézier at parameter @p t.
double evaluate_bezier_time_component(const Point& P0, const Point& P1, const Point& P2, const Point& P3, double t);

/// @brief Evaluates the derivative of the time (x) component with respect to @p t.
double evaluate_bezier_time_derivative(const Point& P0, const Point& P1, const Point& P2, const Point& P3, double t);

/**
 * @brief Finds the parameter @c t whose time component equals @p target_time, by bisection.
 *
 * Robust but slower fallback used when Newton-Raphson is unsuitable.
 * @param p0,p1,p2,p3 The cubic Bézier control points.
 * @param target_time The time to solve for.
 * @param t_min,t_max Search bounds for @c t.
 * @param precision Absolute time error at which to stop.
 * @param max_iterations Maximum bisection steps.
 * @return The parameter @c t in [@p t_min, @p t_max].
 */
double solve_t_for_time_bisection(
    const Point& p0, const Point& p1,
    const Point& p2, const Point& p3,
    double target_time,
    double t_min = 0.0,
    double t_max = 1.0,
    double precision = 1e-6,
    int max_iterations = 30
);

/**
 * @brief Finds the parameter @c t whose time component equals @p target_time.
 *
 * Uses Newton-Raphson seeded by @p initial_guess and falls back to bisection
 * if it fails to converge. Returns 0 or 1 for targets at or beyond the
 * segment's time range.
 * @param P0,P1,P2,P3 The cubic Bézier control points.
 * @param target_time The time to solve for.
 * @param initial_guess Optional starting estimate for @c t in [0,1]; used only
 *        as a convergence hint and never modified. Pass nullptr to seed from a
 *        linear estimate.
 * @param precision Absolute time error at which to stop.
 * @param max_iterations Maximum Newton-Raphson steps before falling back.
 * @return The parameter @c t in [0,1].
 * @throws std::invalid_argument if the segment's start and end times are equal.
 */
double solve_t_for_time(
    const Point& P0, const Point& P1,
    const Point& P2, const Point& P3,
    double target_time,
    double* initial_guess = nullptr,
    double precision = 1e-6,
    int max_iterations = 4
);

/**
 * @brief Produces handles that make the segment between @p p0 and @p p3 appear linear.
 * @param p0 Start anchor point.
 * @param p3 End anchor point.
 * @param out_p1 Receives the out-handle for @p p0 (one third along the segment).
 * @param out_p2 Receives the in-handle for @p p3 (two thirds along the segment).
 */
void create_linear_bezier_handles(const Point& p0, const Point& p3, Point& out_p1, Point& out_p2);

/**
 * @brief Produces flat (horizontal) handles around a keyframe point.
 * @param keyframe_point The keyframe position the handles surround.
 * @param time_offset Horizontal distance of each handle from the keyframe.
 * @param out_in_handle Receives the in-handle (keyframe value, time - offset).
 * @param out_out_handle Receives the out-handle (keyframe value, time + offset).
 */
void create_flat_bezier_handles(const Point& keyframe_point, double time_offset, Point& out_in_handle, Point& out_out_handle);

} // namespace bezier_utils

} // namespace anim

#endif // ANIM_BEZIER_UTILS_HPP
