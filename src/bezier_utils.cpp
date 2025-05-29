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

// Helper to evaluate just the time component
double evaluate_bezier_time_component(const Point& P0, const Point& P1, const Point& P2, const Point& P3, double t) {
    double one_minus_t = 1.0 - t;
    double t2 = t * t;
    double t3 = t2 * t;
    double omt2 = one_minus_t * one_minus_t;
    double omt3 = omt2 * one_minus_t;

    return (omt3 * P0.time) + 
           (3.0 * omt2 * t * P1.time) + 
           (3.0 * one_minus_t * t2 * P2.time) + 
           (t3 * P3.time);
}

// Helper to evaluate the derivative of the time component
double evaluate_bezier_time_derivative(const Point& P0, const Point& P1, const Point& P2, const Point& P3, double t) {
    double one_minus_t = 1.0 - t;
    double omt2 = one_minus_t * one_minus_t;
    double t2 = t * t;

    return 3.0 * omt2 * (P1.time - P0.time) +
           6.0 * one_minus_t * t * (P2.time - P1.time) +
           3.0 * t2 * (P3.time - P2.time);
}


double solve_t_for_time_bisection(
    const Point& p0, const Point& p1,
    const Point& p2, const Point& p3,
    double target_time,
    double t_min,
    double t_max,
    double precision,
    int max_iterations
) {
    
    // For efficiency, check if target_time is at one of the endpoints
    if (std::abs(target_time - p0.time) < precision) return 0.0;
    if (std::abs(target_time - p3.time) < precision) return 1.0;
    
    // Check if target_time is out of range
    if (target_time < p0.time || target_time > p3.time) {
        throw std::out_of_range("Target time is outside the curve segment");
    }
    
    // Binary search to find the parameter t
    double t = (t_min + t_max) / 2.0;
    if (t < 0.0 || t > 1.0) {
        throw std::invalid_argument("Initial t must be in range [0, 1]");
    }
    
    for (int i = 0; i < max_iterations; ++i) {
        auto mid_time = evaluate_bezier_time_component(p0, p1, p2, p3, t);
        double diff = mid_time - target_time;
        
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

// Function to find 't' for a given 'time' using Newton-Raphson
double solve_t_for_time(
    const Point& P0, const Point& P1, const Point& P2, const Point& P3, 
    double target_time,
    double* initial_guess,
    double precision,
    int max_iterations
) {
    
    // Check edge cases
    if (target_time <= P0.time) return 0.0;
    if (target_time >= P3.time) return 1.0;

    double current_t = 0.0;
    if (initial_guess && *initial_guess >= 0.0 && *initial_guess <= 1.0) {
        current_t = *initial_guess;
    } else {
        double t_diff = P3.time - P0.time;
        if (t_diff == 0.0) {
            throw std::invalid_argument("Invalid bezier segment: start and end times are equal");
        }
        current_t = (target_time - P0.time) / t_diff;
        current_t = std::max(0.0, std::min(1.0, current_t)); // Clamp to [0, 1]
    }

    for (int i = 0; i < max_iterations; ++i) {
        double current_time = evaluate_bezier_time_component(P0, P1, P2, P3, current_t);
        double error = current_time - target_time;

        if (std::abs(error) < precision) {
            return current_t; // Success!
        }

        double derivative = evaluate_bezier_time_derivative(P0, P1, P2, P3, current_t);

        if (std::abs(derivative) < 1e-6) {
            // Derivative is zero, can't proceed with Newton-Raphson
            return solve_t_for_time_bisection(P0, P1, P2, P3, 
                target_time, 
                initial_guess ? *initial_guess : 0.0, 1.0);
        }

        current_t -= error / derivative;
        current_t = std::max(0.0, std::min(1.0, current_t)); // Clamp t to [0, 1]
    }
    // Failed to converge, fallback to bisection method

    return solve_t_for_time_bisection(P0, P1, P2, P3, 
        target_time, 
        initial_guess ? *initial_guess : 0.0, 1.0);
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
