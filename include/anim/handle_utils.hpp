#ifndef ANIM_HANDLE_UTILS_HPP
#define ANIM_HANDLE_UTILS_HPP

#include "point.hpp"    
#include "keyframe.hpp"  

namespace anim {

    enum class GrabbedHandle {
        none,
        in_handle,
        out_handle
    };

    bool nearly_equal(double a, double b, double epsilon = 1e-9);

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
  
    // Constrains the in-handle's time of a keyframe.
    void constrain_in_handle_time(Keyframe& keyframe, const Keyframe& prev_keyframe);

    // Constrains the out-handle's time of a keyframe.
    void constrain_out_handle_time(Keyframe& keyframe, const Keyframe& next_keyframe);

    void constrain_handles_time(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr);

    void ensure_handle_time_boundary(
        const Point& keyframe_pos,
        Point& handle_to_adjust,
        const Point& boundary_pos,
        bool is_in_handle
    );

    void ensure_linear_handles_time_boundary(
        Keyframe& keyframe, 
        const Keyframe* prev_keyframe_ptr, 
        const Keyframe* next_keyframe_ptr);

    // HandleMode::flat
    // Handles have the same 'value' as the keyframe's position (parallel to the time axis).
    // Their 'time' values are independently adjustable but clamped.
    void apply_flat_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr);

    // HandleMode::smooth 
    // Handles are auto-calculated to create a smooth (C1 continuous) transition.
    // They are collinear, and their magnitude is typically a factor of the distance to adjacent keyframes.
    void apply_smooth_handles(
        Keyframe& keyframe, 
        const Keyframe* prev_keyframe_ptr, 
        const Keyframe* next_keyframe_ptr, 
        double smooth_factor = 1.0/3.0);

    void apply_aligned_handles(
            Keyframe& keyframe, 
            const Keyframe* prev_keyframe_ptr, 
            const Keyframe* next_keyframe_ptr,
            GrabbedHandle grabbed_handle = GrabbedHandle::none);

} // namespace anim

#endif // ANIM_HANDLE_UTILS_HPP
