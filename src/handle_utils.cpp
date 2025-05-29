#include "anim/handle_utils.hpp"
#include <cmath>         
#include <stdexcept>     
#include <algorithm>     
#include <limits>
#include <numbers> 

namespace anim {

    bool nearly_equal(double a, double b, double epsilon) { 
        return std::abs(a - b) < epsilon;
    }

    // Better for really small numbers, but we don't need it for our use case.
    // bool nearly_equal(double a, double b, double epsilon) { 
    //     if (epsilon <= 0.0) {
    //         epsilon = std::numeric_limits<double>::epsilon();
    //     }
        
    //     // For very small numbers, use absolute comparison
    //     if (std::abs(a) < 1e-12 && std::abs(b) < 1e-12) {
    //         return std::abs(a - b) < epsilon;
    //     }
        
    //     // For larger numbers, use relative comparison
    //     double max_val = std::max(std::abs(a), std::abs(b));
    //     return std::abs(a - b) < epsilon * max_val;
    // }

    double distance(const Point& p1, const Point& p2) {
        double dt = p1.time - p2.time;
        double dv = p1.value - p2.value;
        return std::sqrt(dt * dt + dv * dv);
    }

    Vector vector(const Point& p1, const Point& p2) {
        return Vector(p2.time - p1.time, p2.value - p1.value);
    }

    Vector normalize(const Vector& vec) {
        double length_sq = vec.time * vec.time + vec.value * vec.value;
        if (nearly_equal(length_sq, 0.0)) {
            throw std::domain_error("Cannot normalize a zero-length vector");
        }
        double length = std::sqrt(length_sq);
        return Vector(vec.time / length, vec.value / length);
    }
    
    double length_squared(const Vector& vec) {
        return vec.time * vec.time + vec.value * vec.value;
    }

    double length(const Vector& vec) {
        return std::sqrt(length_squared(vec));
    }

    Point midpoint(const Point& p1, const Point& p2) {
        return Point((p1.time + p2.time) / 2.0, (p1.value + p2.value) / 2.0);
    }

    Point scale(const Point& p, double scalar) {
        return Point(p.time * scalar, p.value * scalar);
    }

    Point translate(const Point& p, const Vector& vec) {
        return Point(p.time + vec.time, p.value + vec.value);
    }

    Point rotate(const Point& p, double angle_degrees) {
        double rad = angle_degrees * std::numbers::pi / 180.0; 
        double cos_angle = std::cos(rad);
        double sin_angle = std::sin(rad);
        return Point(
            p.time * cos_angle - p.value * sin_angle,
            p.time * sin_angle + p.value * cos_angle
        );
    }

    double dot_product(const Vector& v1, const Vector& v2) {
        return v1.time * v2.time + v1.value * v2.value;
    }

    double cross_product(const Vector& v1, const Vector& v2) {
        return v1.time * v2.value - v1.value * v2.time;
    }

    Vector reflect(const Vector& vec, const Vector& normal_unit_vector) {
        double dp = dot_product(vec, normal_unit_vector);
        return Vector(
            vec.time - 2 * dp * normal_unit_vector.time,
            vec.value - 2 * dp * normal_unit_vector.value
        );
    }

    Vector invert(const Vector& vec) {
        return Vector(-vec.time, -vec.value);
    }
    
    void constrain_in_handle_time(Keyframe& keyframe, const Keyframe& prev_keyframe) {
        keyframe.in_handle.time = std::clamp(
            keyframe.in_handle.time, 
            prev_keyframe.position.time, 
            keyframe.position.time
        );
    }

    void constrain_out_handle_time(Keyframe& keyframe, const Keyframe& next_keyframe) {
        keyframe.out_handle.time = std::clamp(
            keyframe.out_handle.time, 
            keyframe.position.time, 
            next_keyframe.position.time
        );
    }

    void constrain_handles_time(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr) {
            constrain_in_handle_time(keyframe, *prev_keyframe_ptr);
        }
        if (next_keyframe_ptr) {
            constrain_out_handle_time(keyframe, *next_keyframe_ptr);
        }
    }

    void apply_flat_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr && next_keyframe_ptr) { // keyframe is in the middle of two keyframes
            double in_time_offset = (prev_keyframe_ptr->position.time - keyframe.position.time) / 3.0;
            keyframe.in_handle  = Point(keyframe.position.time + in_time_offset, keyframe.position.value);
            double out_time_offset = (next_keyframe_ptr->position.time - keyframe.position.time) / 3.0;
            keyframe.out_handle = Point(keyframe.position.time + out_time_offset, keyframe.position.value);
        } else if (prev_keyframe_ptr) { // keyframe is the last keyframe
            double time_offset = (prev_keyframe_ptr->position.time - keyframe.position.time) / 3.0;
            keyframe.in_handle  = Point(keyframe.position.time + time_offset, keyframe.position.value);
            keyframe.out_handle = Point(keyframe.position.time - time_offset, keyframe.position.value);
        } else if (next_keyframe_ptr) { // keyframe is the first keyframe
            double time_offset = (next_keyframe_ptr->position.time - keyframe.position.time) / 3.0;
            keyframe.in_handle = Point(keyframe.position.time + time_offset, keyframe.position.value);
            keyframe.out_handle = Point(keyframe.position.time + time_offset, keyframe.position.value);
        } else { // keyframe is the only keyframe
            keyframe.in_handle = Point(keyframe.position.time - 1.0, keyframe.position.value);
            keyframe.out_handle = Point(keyframe.position.time + 1.0, keyframe.position.value);
        }
    }

    void apply_smooth_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr, double smooth_factor) {
        if (prev_keyframe_ptr && next_keyframe_ptr) { // keyframe is in the middle of two keyframes
            Vector vec_prev_to_curr = vector(prev_keyframe_ptr->position, keyframe.position);
            Vector vec_curr_to_next = vector(keyframe.position, next_keyframe_ptr->position);
            Vector tangent_dir_vec = vector(prev_keyframe_ptr->position, next_keyframe_ptr->position);
            
            if (nearly_equal(length_squared(tangent_dir_vec), 0.0)) {
                apply_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(tangent_dir_vec);
            double dist_prev = length(vec_prev_to_curr);
            double dist_next = length(vec_curr_to_next);
            
            Vector in_handle_offset = normalized_tangent * (dist_prev * smooth_factor);
            Vector out_handle_offset = normalized_tangent * (dist_next * smooth_factor);

            keyframe.in_handle  = keyframe.position - in_handle_offset;
            keyframe.out_handle = keyframe.position + out_handle_offset;

        } else if (prev_keyframe_ptr) { // keyframe is the last keyframe
            Vector vec_prev_to_curr = vector(prev_keyframe_ptr->position, keyframe.position);
            if (nearly_equal(length_squared(vec_prev_to_curr), 0.0)) {
                apply_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(vec_prev_to_curr);
            double dist_prev = length(vec_prev_to_curr);

            Vector handle_offset = normalized_tangent * (dist_prev * smooth_factor);
            keyframe.in_handle  = keyframe.position - handle_offset;
            keyframe.out_handle = keyframe.position + handle_offset;

        } else if (next_keyframe_ptr) { // keyframe is the first keyframe
            Vector vec_curr_to_next = vector(keyframe.position, next_keyframe_ptr->position);
            if (nearly_equal(length_squared(vec_curr_to_next), 0.0)) {
                apply_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(vec_curr_to_next);
            double dist_next = length(vec_curr_to_next);

            Vector handle_offset = normalized_tangent * (dist_next * smooth_factor);
            keyframe.in_handle  = keyframe.position - handle_offset;
            keyframe.out_handle = keyframe.position + handle_offset;
            
        } else { // keyframe is the only keyframe
            keyframe.in_handle  = Point(keyframe.position.time - 1.0, keyframe.position.value);
            keyframe.out_handle = Point(keyframe.position.time + 1.0, keyframe.position.value);
        }
    }

    void apply_manual_smooth_handles(            
            Keyframe& keyframe, 
            const Keyframe* prev_keyframe_ptr, 
            const Keyframe* next_keyframe_ptr,
            GrabbedHandle grabbed_handle
    ) {

        Point* source_handle_actual_ptr;
        Point* target_handle_actual_ptr;

        if (prev_keyframe_ptr) {
            if (grabbed_handle == GrabbedHandle::none && prev_keyframe_ptr->position.time >= keyframe.in_handle.time) {
                grabbed_handle = GrabbedHandle::in_handle; // Force in_handle if prev keyframe is before or at the same time
            }
            keyframe.in_handle.time = std::clamp(keyframe.in_handle.time, 
                prev_keyframe_ptr->position.time, 
                keyframe.position.time
            );
        } else {
            keyframe.in_handle.time = std::min(
                keyframe.in_handle.time, 
                keyframe.position.time
            );
        }

        if (next_keyframe_ptr) {
            if (grabbed_handle == GrabbedHandle::none && next_keyframe_ptr->position.time <= keyframe.out_handle.time) {
                grabbed_handle = GrabbedHandle::out_handle; // Force out_handle if next keyframe is after or at the same time
            }
            keyframe.out_handle.time = std::clamp(keyframe.out_handle.time, 
                keyframe.position.time, 
                next_keyframe_ptr->position.time
            );
        } else {
            keyframe.out_handle.time = std::max(
                keyframe.out_handle.time, 
                keyframe.position.time
            );
        }

        if (grabbed_handle == GrabbedHandle::out_handle || grabbed_handle == GrabbedHandle::none) {
            source_handle_actual_ptr = &(keyframe.out_handle);
            target_handle_actual_ptr = &(keyframe.in_handle);

        } else {
            source_handle_actual_ptr = &(keyframe.in_handle);
            target_handle_actual_ptr = &(keyframe.out_handle);

        }

        const Point& source_handle_current_pos = *source_handle_actual_ptr;
        Vector vec_kf_to_source = vector(keyframe.position, source_handle_current_pos);
        double L_sq_kf_to_source = length_squared(vec_kf_to_source);

        if (nearly_equal(L_sq_kf_to_source, 0.0)) {
            keyframe.in_handle  = keyframe.position;
            keyframe.out_handle = keyframe.position;
        } else {
            Vector dir_kf_to_source_normalized = normalize(vec_kf_to_source);
            Vector dir_kf_to_target_normalized = invert(dir_kf_to_source_normalized);
            double magnitude_kf_to_source = length(vec_kf_to_source);
            *target_handle_actual_ptr = keyframe.position + (dir_kf_to_target_normalized * magnitude_kf_to_source);

        }
    }

    void apply_manual_smooth_handles_proportional(
        Keyframe& keyframe, 
        const Keyframe* prev_keyframe_ptr, 
        const Keyframe* next_keyframe_ptr,
        GrabbedHandle grabbed_handle) 
    {
        Point* source_handle_ptr = grabbed_handle == GrabbedHandle::out_handle ? &(keyframe.out_handle) : &(keyframe.in_handle);
        const Point& source_handle_pos = *source_handle_ptr;

        // Vector from keyframe to the manually set handle
        Vector vec_kf_to_source = vector(keyframe.position, source_handle_pos);
        double L_sq_kf_to_source = length_squared(vec_kf_to_source);

        // --- Handle Zero-Length Drag (Set flat handles) ---
        if (nearly_equal(L_sq_kf_to_source, 0.0)) {
            keyframe.in_handle  = keyframe.position;
            keyframe.out_handle = keyframe.position;
            return;
        }

        double handle_dist = std::sqrt(L_sq_kf_to_source);

        // --- Calculate Base Tangent Direction (always points "out" or forward in time/value) ---
        Vector base_tangent;
        if (grabbed_handle == GrabbedHandle::out_handle) {
            base_tangent = vec_kf_to_source * (1.0 / handle_dist); // Normalize
        } else {
            // Source is IN handle, so its vector points "in". 
            // We want the opposite direction for the base tangent.
            base_tangent = vector(source_handle_pos, keyframe.position) * (1.0 / handle_dist); // Normalize
        }

        // Handle degenerate tangents (shouldn't happen due to L_sq check, but be safe)
        if (nearly_equal(length_squared(base_tangent), 0.0)) {
            keyframe.in_handle = Point(keyframe.position.time - 1.0, keyframe.position.value);
            keyframe.out_handle = Point(keyframe.position.time + 1.0, keyframe.position.value);
            return;
        }

        // --- Calculate Distances to Neighbors ---
        double dist_prev = 0.0;
        double dist_next = 0.0;
        bool has_prev = prev_keyframe_ptr != nullptr;
        bool has_next = next_keyframe_ptr != nullptr;

        if (has_prev) {
            dist_prev = length(vector(prev_keyframe_ptr->position, keyframe.position));
        }
        if (has_next) {
            dist_next = length(vector(keyframe.position, next_keyframe_ptr->position));
        }

        // --- Calculate Smooth Factor based on source handle and distances ---
        double smooth_factor = 0.333; // Default factor (e.g., Catmull-Rom like)

        if (has_prev && has_next) { // Middle keyframe
            if (grabbed_handle == GrabbedHandle::out_handle) {
                // If dist_next is ~0, keyframes overlap; factor can be 1 or any other default.
                smooth_factor = (dist_next > 1e-6) ? (handle_dist / dist_next) : 1.0;
            } else {
                smooth_factor = (dist_prev > 1e-6) ? (handle_dist / dist_prev) : 1.0;
            }
        } else if (has_prev) { // Last keyframe
            smooth_factor = (dist_prev > 1e-6) ? (handle_dist / dist_prev) : 1.0;
            dist_next = dist_prev; // Use dist_prev for both sides for symmetry
        } else if (has_next) { // First keyframe
            smooth_factor = (dist_next > 1e-6) ? (handle_dist / dist_next) : 1.0;
            dist_prev = dist_next; // Use dist_next for both sides for symmetry
        } else { // Single keyframe - Just mirror the drag (Symmetrical magnitude)
            keyframe.out_handle = keyframe.position + (base_tangent * handle_dist);
            keyframe.in_handle  = keyframe.position - (base_tangent * handle_dist);
            return; // We're done for the single keyframe case.
        }

        // Optional: Clamp smooth_factor if needed (e.g., to prevent extreme overshoots)
        // smooth_factor = std::max(0.0, std::min(smooth_factor, 2.0)); 

        // --- Calculate Handle Offsets and Set Handles ---
        // The key change: Magnitudes are scaled by *their* respective distances.
        Vector out_handle_offset = base_tangent * (dist_next * smooth_factor);
        Vector in_handle_offset  = base_tangent * (dist_prev * smooth_factor);

        keyframe.out_handle = keyframe.position + out_handle_offset;
        keyframe.in_handle  = keyframe.position - in_handle_offset;
    }
} // namespace anim

