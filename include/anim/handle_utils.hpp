#ifndef ANIM_HANDLE_UTILS_HPP
#define ANIM_HANDLE_UTILS_HPP

#define ANIM_M_PI 3.14159265358979323846

#include "point.hpp"
#include "keyframe.hpp"
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace anim {

    double distance(const Point& p1, const Point& p2) {
        return std::sqrt((p1.time - p2.time) * (p1.time - p2.time) +
                         (p1.value - p2.value) * (p1.value - p2.value));
    }

    Vector vector(const Point& p1, const Point& p2) {
        return Vector(p2.time - p1.time, p2.value - p1.value);
    }

    Vector normalize(const Vector& vec) {
        auto length = std::sqrt(vec.time * vec.time + vec.value * vec.value);
        if (length == 0.0) {
            throw std::domain_error("Cannot normalize a zero-length vector");
        }
        return Vector(vec.time / length, vec.value / length);
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

    Point rotate(const Point& p, double angle) {
        auto rad = angle * ANIM_M_PI / 180.0; // Convert degrees to radians
        auto cos_angle = std::cos(rad);
        auto sin_angle = std::sin(rad);
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

    Vector reflect(const Vector& vec, const Vector& normal) {
        auto dot_product = vec.time * normal.time + vec.value * normal.value;
        return Vector(
            vec.time - 2 * dot_product * normal.time,
            vec.value - 2 * dot_product * normal.value
        );
    }

    // Constrain time - keep direction but clamp value
    Point constrain_time(const Point& origin, const Point& p, double min_time, double max_time) {
        if (p.time >= min_time && p.time <= max_time) {
            return p; // Already within bounds
        }
        auto direction = normalize(vector(origin, p));
        auto constrained_time = std::clamp(p.time, min_time, max_time);
        return Point(constrained_time, origin.value + direction.value * (constrained_time - origin.time));
    }

    void constrain_in_handle_time(Keyframe& keyframe, const Keyframe& prev_keyframe) {
        if (keyframe.in_handle.time >= prev_keyframe.position.time && 
            keyframe.in_handle.time <= keyframe.position.time) {
            // Already within bounds
            return;
        }

        auto direction = normalize(vector(prev_keyframe.position, keyframe.position));
        auto constrained_time = std::clamp(keyframe.in_handle.time, prev_keyframe.position.time, keyframe.position.time);
        keyframe.in_handle = Point(constrained_time, keyframe.position.value + direction.value * (constrained_time - keyframe.position.time));
    }

    void constrain_out_handle_time(Keyframe& keyframe, const Keyframe& next_keyframe) {
        if (keyframe.out_handle.time >= keyframe.position.time && 
            keyframe.out_handle.time <= next_keyframe.position.time) {
            // Already within bounds
            return;
        }

        auto direction = normalize(vector(keyframe.position, next_keyframe.position));
        auto constrained_time = std::clamp(keyframe.out_handle.time, keyframe.position.time, next_keyframe.position.time);
        keyframe.out_handle = Point(constrained_time, keyframe.position.value + direction.value * (constrained_time - keyframe.position.time));
    }

    void get_flat_handles(Keyframe& keyframe, Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr) {
            keyframe.in_handle = Point(std::clamp(keyframe.in_handle.time, prev_keyframe_ptr->position.time, keyframe.position.time), prev_keyframe_ptr->position.value);
        } else {
            keyframe.in_handle = Point(keyframe.position.time, keyframe.position.value);
        }

        if (next_keyframe_ptr) {
            keyframe.out_handle = Point(std::clamp(keyframe.out_handle.time, keyframe.position.time, next_keyframe_ptr->position.time), next_keyframe_ptr->position.value);
        } else {
            keyframe.out_handle = Point(keyframe.position.time, keyframe.position.value);
        }
    }


    // Adjust handles based on previous and next keyframes if they exist
    void get_smooth_handles(Keyframe& keyframe, Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr, double smooth_factor = 0.3333333) {
        if (prev_keyframe_ptr && next_keyframe_ptr) {
       
            auto prev_to_current = vector(prev_keyframe_ptr->position, keyframe.position);
            auto current_to_next = vector(keyframe.position, next_keyframe_ptr->position);

            auto prev_to_current_normalized = normalize(prev_to_current);
            auto current_to_next_normalized = normalize(current_to_next);

            auto average_direction = (prev_to_current_normalized + current_to_next_normalized) * .5;
            average_direction = normalize(average_direction);

            auto distance_prev = distance(prev_keyframe_ptr->position, keyframe.position);
            auto distance_next = distance(keyframe.position, next_keyframe_ptr->position);
            
            keyframe.in_handle = (keyframe.position - average_direction) * distance_prev * smooth_factor;
            keyframe.out_handle = (keyframe.position + average_direction) * distance_next * smooth_factor;
        } else if (prev_keyframe_ptr) {
            auto prev_to_current = vector(prev_keyframe_ptr->position, keyframe.position);
            auto prev_to_current_normalized = normalize(prev_to_current);
            auto distance_prev = distance(prev_keyframe_ptr->position, keyframe.position);
            
            keyframe.in_handle = (keyframe.position - prev_to_current_normalized) * distance_prev * smooth_factor;
            keyframe.out_handle = Point(keyframe.position.time + distance_prev * smooth_factor, keyframe.position.value);
        } else if (next_keyframe_ptr) {
            auto current_to_next = vector(keyframe.position, next_keyframe_ptr->position);
            auto current_to_next_normalized = normalize(current_to_next);
            auto distance_next = distance(keyframe.position, next_keyframe_ptr->position);
            
            keyframe.in_handle = Point(keyframe.position.time - distance_next * smooth_factor, keyframe.position.value);
            keyframe.out_handle = (keyframe.position + current_to_next_normalized) * distance_next * smooth_factor;
        }
        else {
            // No adjacent keyframes, set handles to zero
            keyframe.in_handle = Point();
            keyframe.out_handle = Point();
        }   
    }

    void get_aligned_handles(Keyframe& keyframe, Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr && next_keyframe_ptr) {
            auto prev_to_current = vector(prev_keyframe_ptr->position, keyframe.position);
            auto current_to_next = vector(keyframe.position, next_keyframe_ptr->position);

            auto prev_to_current_normalized = normalize(prev_to_current);
            auto current_to_next_normalized = normalize(current_to_next);

            keyframe.in_handle = Point(keyframe.position.time - prev_to_current_normalized.time, keyframe.position.value - prev_to_current_normalized.value);
            keyframe.out_handle = Point(keyframe.position.time + current_to_next_normalized.time, keyframe.position.value + current_to_next_normalized.value);
        } else if (prev_keyframe_ptr) {
            auto prev_to_current = vector(prev_keyframe_ptr->position, keyframe.position);
            auto prev_to_current_normalized = normalize(prev_to_current);

            keyframe.in_handle = Point(keyframe.position.time - prev_to_current_normalized.time, keyframe.position.value - prev_to_current_normalized.value);
            keyframe.out_handle = Point(keyframe.position.time + prev_to_current_normalized.time, keyframe.position.value + prev_to_current_normalized.value);
        } else if (next_keyframe_ptr) {
            auto current_to_next = vector(keyframe.position, next_keyframe_ptr->position);
            auto current_to_next_normalized = normalize(current_to_next);

            keyframe.in_handle = Point(keyframe.position.time - current_to_next_normalized.time, keyframe.position.value - current_to_next_normalized.value);
            keyframe.out_handle = Point(keyframe.position.time + current_to_next_normalized.time, keyframe.position.value + current_to_next_normalized.value);
        }
        else {
            // No adjacent keyframes, set handles to zero
            keyframe.in_handle = Point();
            keyframe.out_handle = Point();
        }   
    }

    void get_free_handles(Keyframe& keyframe, Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr) {
            constrain_in_handle_time(keyframe, *prev_keyframe_ptr);
        } else if (next_keyframe_ptr) {
            constrain_out_handle_time(keyframe, *next_keyframe_ptr);
        } 

    };







} // namespace anim

#endif // ANIM_HANDLE_UTILS_HPP
