#ifndef ANIM_HANDLE_UTILS_HPP
#define ANIM_HANDLE_UTILS_HPP

#include "point.hpp"    
#include "keyframe.hpp"  

namespace anim {

    bool nearly_equal(double a, double b, double epsilon = std::numeric_limits<double>::epsilon());

    // Calculates the Euclidean distance between two points.
    double distance(const Point& p1, const Point& p2);

    Vector vector(const Point& p1, const Point& p2);

    Vector normalize(const Vector& vec);
    
    double length_squared(const Vector& vec);

    double length(const Vector& vec);

    Point midpoint(const Point& p1, const Point& p2);

    Point scale(const Point& p, double scalar);

    Point translate(const Point& p, const Vector& vec);

    Point rotate(const Point& p, double angle_degrees);

    double dot_product(const Vector& v1, const Vector& v2);

    double cross_product(const Vector& v1, const Vector& v2);

    // 'normal' is assumed to be a unit vector.
    Vector reflect(const Vector& vec, const Vector& normal_unit_vector);

    Vector invert(const Vector& vec) ;
    
    // Constrains a point 'p' (typically a handle) to be within [min_time, max_time]
    // while maintaining its slope relative to an 'origin' point (typically the keyframe).
    Point constrain_point_time_preserve_slope(const Point& origin, const Point& p, double min_time, double max_time);

    // Constrains the in-handle's time of a keyframe.
    void constrain_in_handle_time(Keyframe& keyframe, const Keyframe& prev_keyframe);

    // Constrains the out-handle's time of a keyframe.
    void constrain_out_handle_time(Keyframe& keyframe, const Keyframe& next_keyframe);

    void constrain_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr);

    // Constraints the in and out handles of a keyframe based on its handle mode.

    // --- Handle Mode Calculation Functions ---
    // These functions modify the keyframe's in_handle and out_handle based on the mode.
    // They assume Point and Vector types have overloaded operators for basic arithmetic
    // (e.g., Point - Vector, Point + Vector, Vector * double, Vector + Vector)
    // if not directly using component-wise operations.

    // HandleMode::flat
    // Handles have the same 'value' as the keyframe's position (parallel to the time axis).
    // Their 'time' values are independently adjustable but clamped.
    void calculate_flat_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr);

    // HandleMode::smooth 
    // Handles are auto-calculated to create a smooth (C1 continuous) transition.
    // They are collinear, and their magnitude is typically a factor of the distance to adjacent keyframes.
    void calculate_smooth_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr, double smooth_factor = 1.0/3.0);

    // HandleMode::aligned
    // This function is intended to be called when a user adjusts one handle,
    // and the other handle needs to be updated to remain collinear and opposite.
    // It does NOT auto-calculate tangents from neighbors like 'smooth'.
    // `source_is_out_handle` = true if out_handle was just moved by user, false if in_handle was.
    // out_handle is the source when updating from state
    void enforce_aligned_handles(Keyframe& keyframe, bool source_is_out_handle = true);
    
    // Call this when switching TO aligned mode to set an initial state.
    // This uses the smooth calculation as a reasonable default.
    void initialize_handles_for_aligned_mode(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr);

} // namespace anim

#endif // ANIM_HANDLE_UTILS_HPP
