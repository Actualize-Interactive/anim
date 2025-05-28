#include "anim/handle_utils.hpp"
#include <cmath>         
#include <stdexcept>     
#include <algorithm>     
#include <limits>
#include <numbers> 

#include <iostream>


namespace anim {

    bool nearly_equal(double a, double b, double epsilon) { 
        return std::abs(a - b) < epsilon;
    }

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
    
    Point constrain_point_time_preserve_slope(const Point& origin, const Point& p, double min_time, double max_time) {
        if (p.time >= min_time && p.time <= max_time) {
            return p;
        }

        if (nearly_equal(p.time, origin.time)) {
            if (p.time < min_time) return Point(min_time, p.value);
            return Point(max_time, p.value);
        }

        double slope = (p.value - origin.value) / (p.time - origin.time);

        // if (p.time < min_time) {
        //     return Point(min_time, origin.value + slope * (min_time - origin.time));
        // } else {
        //     return Point(max_time, origin.value + slope * (max_time - origin.time));
        // }
        if (p.time < min_time) {
            return Point(min_time, p.value);
        } else {
            return Point(max_time, p.value);
        }
    }

    void constrain_in_handle_time(Keyframe& keyframe, const Keyframe& prev_keyframe) {
        keyframe.in_handle = constrain_point_time_preserve_slope(
            keyframe.position, 
            keyframe.in_handle, 
            prev_keyframe.position.time, 
            keyframe.position.time
        );
    }

    void constrain_out_handle_time(Keyframe& keyframe, const Keyframe& next_keyframe) {
        keyframe.out_handle = constrain_point_time_preserve_slope(
            keyframe.position, 
            keyframe.out_handle, 
            keyframe.position.time, 
            next_keyframe.position.time
        );
    }

    void constrain_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr) {
            constrain_in_handle_time(keyframe, *prev_keyframe_ptr);
        }
        if (next_keyframe_ptr) {
            constrain_out_handle_time(keyframe, *next_keyframe_ptr);
        }
    }

    void calculate_flat_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
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

    void calculate_smooth_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr, double smooth_factor) {
        if (prev_keyframe_ptr && next_keyframe_ptr) { // keyframe is in the middle of two keyframes
            Vector vec_prev_to_curr = vector(prev_keyframe_ptr->position, keyframe.position);
            Vector vec_curr_to_next = vector(keyframe.position, next_keyframe_ptr->position);
            Vector tangent_dir_vec = vector(prev_keyframe_ptr->position, next_keyframe_ptr->position);
            
            if (nearly_equal(length_squared(tangent_dir_vec), 0.0)) {
                calculate_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(tangent_dir_vec);
            double dist_prev = length(vec_prev_to_curr);
            double dist_next = length(vec_curr_to_next);
            
            Vector in_handle_offset = normalized_tangent * (dist_prev * smooth_factor);
            Vector out_handle_offset = normalized_tangent * (dist_next * smooth_factor);

            keyframe.in_handle  = keyframe.position - in_handle_offset;
            keyframe.out_handle = keyframe.position + out_handle_offset;

            std::cout << "Both Smooth handles calculated: "
                      << "in_handle (" << keyframe.in_handle.time << ", " << keyframe.in_handle.value << "), "
                      << "out_handle (" << keyframe.out_handle.time << ", " << keyframe.out_handle.value << ")" << std::endl;

        } else if (prev_keyframe_ptr) { // keyframe is the last keyframe
            Vector vec_prev_to_curr = vector(prev_keyframe_ptr->position, keyframe.position);
            if (nearly_equal(length_squared(vec_prev_to_curr), 0.0)) {
                calculate_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(vec_prev_to_curr);
            double dist_prev = length(vec_prev_to_curr);

            Vector handle_offset = normalized_tangent * (dist_prev * smooth_factor);
            keyframe.in_handle  = keyframe.position - handle_offset;
            keyframe.out_handle = keyframe.position + handle_offset;
            std::cout << "In handle offset calculated: " << "offset: " << handle_offset.time << ", " << handle_offset.value << ", "
                      << "in_handle (" << keyframe.in_handle.time << ", " << keyframe.in_handle.value << "), "
                      << "out_handle (" << keyframe.out_handle.time << ", " << keyframe.out_handle.value << ")" << std::endl;

        } else if (next_keyframe_ptr) { // keyframe is the first keyframe
            Vector vec_curr_to_next = vector(keyframe.position, next_keyframe_ptr->position);
            if (nearly_equal(length_squared(vec_curr_to_next), 0.0)) {
                calculate_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
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

    double get_smooth_factor_from_handle(
        const Keyframe& keyframe, 
        const Keyframe* prev_keyframe_ptr, 
        const Keyframe* next_keyframe_ptr,
        bool source_is_out_handle) 
    {
        Vector tangent_dir_vec;
        double relevant_dist = 0.0;
        Point grabbed_handle_pos = source_is_out_handle ? keyframe.out_handle : keyframe.in_handle;

        // Determine tangent and relevant distance based on keyframe position
        if (prev_keyframe_ptr && next_keyframe_ptr) {
            tangent_dir_vec = vector(prev_keyframe_ptr->position, next_keyframe_ptr->position);
            if (!source_is_out_handle) {
                relevant_dist = length(vector(prev_keyframe_ptr->position, keyframe.position));
            } else {
                relevant_dist = length(vector(keyframe.position, next_keyframe_ptr->position));
            }
        } else if (prev_keyframe_ptr) { // Last keyframe
            tangent_dir_vec = vector(prev_keyframe_ptr->position, keyframe.position);
            relevant_dist = length(tangent_dir_vec);
        } else if (next_keyframe_ptr) { // First keyframe
            tangent_dir_vec = vector(keyframe.position, next_keyframe_ptr->position);
            relevant_dist = length(tangent_dir_vec);
        } else {
            return -1.0; // Cannot determine factor for a single keyframe this way
        }

        // Check for zero-length vectors to avoid division by zero
        if (nearly_equal(length_squared(tangent_dir_vec), 0.0) || nearly_equal(relevant_dist, 0.0)) {
            return -1.0; // Cannot calculate factor if points are coincident
        }

        // Calculate the distance of the grabbed handle from the keyframe
        double handle_dist = length(vector(keyframe.position, grabbed_handle_pos));

        // Calculate the smooth factor
        double smooth_factor = handle_dist / relevant_dist;

        return smooth_factor;
    }


    void apply_manual_smooth_handles(Keyframe& keyframe, bool source_is_out_handle) 
    {

        Point* source_handle_actual_ptr;
        Point* target_handle_actual_ptr;

        if (source_is_out_handle) {
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
        bool source_is_out_handle) 
    {
        Point* source_handle_ptr = source_is_out_handle ? &(keyframe.out_handle) : &(keyframe.in_handle);
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
        if (source_is_out_handle) {
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
            if (source_is_out_handle) {
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

        // std::cout << "Applied Proportional Handles: Smooth Factor = " << smooth_factor << std::endl;
        // std::cout << "  In Handle : (" << keyframe.in_handle.time << ", " << keyframe.in_handle.value << "), Mag: " << length(in_handle_offset) << std::endl;
        // std::cout << "  Out Handle: (" << keyframe.out_handle.time << ", " << keyframe.out_handle.value << "), Mag: " << length(out_handle_offset) << std::endl;
    }


    void enforce_aligned_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr, bool source_is_out_handle) {
        if (source_is_out_handle) {
            Vector out_tangent_vec = vector(keyframe.position, keyframe.out_handle);
            double out_length = length(out_tangent_vec);

            if (!nearly_equal(out_length, 0.0)) {
                // we want to make the in_handle have inverted direction of the out_handle but also the same relative
                // x/time distance from the pre
                Vector in_tangent_dir = normalize(invert(out_tangent_vec));
                if(prev_keyframe_ptr) {
                    if(next_keyframe_ptr) {
                        auto next_time_delta = distance(keyframe.position, next_keyframe_ptr->position);
                        auto handle_delta_ratio = (keyframe.out_handle.time - keyframe.position.time) / next_time_delta;
                        auto prev_handle_new_time_dist = distance(prev_keyframe_ptr->position, keyframe.position) * handle_delta_ratio;
                        keyframe.in_handle = keyframe.position + (in_tangent_dir * prev_handle_new_time_dist);        
                    } else {
                        auto prev_handle_new_time_dist = std::min(keyframe.position.time - keyframe.out_handle.time, 
                                                                keyframe.position.time - prev_keyframe_ptr->position.time);
                        keyframe.in_handle = keyframe.position + (in_tangent_dir * prev_handle_new_time_dist);                                               
                    }
                } else {
                    double in_handle_current_dist = distance(keyframe.position, keyframe.in_handle);
                    keyframe.in_handle = keyframe.position + (in_tangent_dir * in_handle_current_dist);
                }
                
                return;
            }
        } else {
            Vector in_tangent_vec = vector(keyframe.position, keyframe.in_handle);
            double in_length = length(in_tangent_vec);
            if (!nearly_equal(in_length, 0.0)) {
                // we want to make the out_handle have inverted direction of the in_handle but also the same relative
                // x/time distance from the pre
                Vector out_tangent_dir = normalize(invert(in_tangent_vec));
                if(next_keyframe_ptr) {
                    if(prev_keyframe_ptr) {
                        auto prev_time_delta = distance(keyframe.position, prev_keyframe_ptr->position);
                        auto handle_delta_ratio = (keyframe.in_handle.time - keyframe.position.time) / prev_time_delta;
                        auto next_handle_new_time_dist = distance(next_keyframe_ptr->position, keyframe.position) * handle_delta_ratio;
                        keyframe.out_handle = keyframe.position + (invert(in_tangent_vec) * next_handle_new_time_dist);        
                    } else {
                        auto next_handle_new_time_dist = std::min(keyframe.position.time - keyframe.in_handle.time, 
                                                                   next_keyframe_ptr->position.time - keyframe.position.time);
                        keyframe.out_handle = keyframe.position + (invert(in_tangent_vec) * next_handle_new_time_dist);                                               
                    }
                } else {
                    double out_handle_current_dist = distance(keyframe.position, keyframe.out_handle);
                    keyframe.out_handle = keyframe.position + (out_tangent_dir * out_handle_current_dist);
                }
                return;
            }
        }
    }
    
    // void enforce_aligned_handles(Keyframe& keyframe, bool source_is_out_handle) {
    //     if (source_is_out_handle) {
    //         Vector out_tangent_vec = vector(keyframe.position, keyframe.out_handle);
    //         double out_length = length(out_tangent_vec);

    //         if (!nearly_equal(out_length, 0.0)) {
    //             // we want to make the in_handle have inverted direction of the out_handle but also the same relative
    //             // x/time distance from the pre

    //             Vector in_tangent_dir = normalize(invert(out_tangent_vec));
    //             double in_handle_current_dist = distance(keyframe.position, keyframe.in_handle);
    //             keyframe.in_handle = keyframe.position + (in_tangent_dir * in_handle_current_dist);
    //             return;
    //         }
    //     } else {
    //         Vector in_tangent_vec = vector(keyframe.position, keyframe.in_handle);
    //         double in_length = length(in_tangent_vec);
    //         if (!nearly_equal(in_length, 0.0)) {
    //             Vector out_tangent_dir = normalize(invert(in_tangent_vec));
    //             double out_handle_current_dist = distance(keyframe.position, keyframe.out_handle);
    //             keyframe.out_handle = keyframe.position + (out_tangent_dir * out_handle_current_dist);
    //             return;
    //         }
    //     }
    // }


    void initialize_handles_for_aligned_mode(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        calculate_smooth_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    }

} // namespace anim

