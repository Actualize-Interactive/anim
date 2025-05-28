#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/handle_utils.hpp>
#include <cmath>

using namespace anim;

TEST_CASE("Basic utility functions", "[handle_utils]") {
    Point p1(0.0, 0.0);
    Point p2(3.0, 4.0);
    
    SECTION("distance function") {
        double dist = distance(p1, p2);
        REQUIRE(dist == Catch::Approx(5.0));
        
        // Test with same points
        REQUIRE(distance(p1, p1) == Catch::Approx(0.0));
    }
    
    SECTION("vector function") {
        Vector vec = vector(p1, p2);
        REQUIRE(vec.time == 3.0);
        REQUIRE(vec.value == 4.0);
        
        // Test reverse direction
        Vector reverse_vec = vector(p2, p1);
        REQUIRE(reverse_vec.time == -3.0);
        REQUIRE(reverse_vec.value == -4.0);
    }
    
    SECTION("normalize function") {
        Vector vec(3.0, 4.0);
        Vector normalized = normalize(vec);
        REQUIRE(normalized.time == Catch::Approx(0.6));
        REQUIRE(normalized.value == Catch::Approx(0.8));
        
        // Test that normalized vector has length 1
        double length = std::sqrt(normalized.time * normalized.time + normalized.value * normalized.value);
        REQUIRE(length == Catch::Approx(1.0));
    }
    
    SECTION("normalize zero vector throws exception") {
        Vector zero_vec(0.0, 0.0);
        REQUIRE_THROWS_AS(normalize(zero_vec), std::domain_error);
    }
}

TEST_CASE("Geometric operations", "[handle_utils]") {
    Point p1(2.0, 3.0);
    Point p2(6.0, 7.0);
    
    SECTION("midpoint function") {
        Point mid = midpoint(p1, p2);
        REQUIRE(mid.time == 4.0);
        REQUIRE(mid.value == 5.0);
    }
    
    SECTION("scale function") {
        Point scaled = scale(p1, 2.5);
        REQUIRE(scaled.time == 5.0);
        REQUIRE(scaled.value == 7.5);
    }
    
    SECTION("translate function") {
        Vector translation(1.5, 2.5);
        Point translated = translate(p1, translation);
        REQUIRE(translated.time == 3.5);
        REQUIRE(translated.value == 5.5);
    }
    
    SECTION("rotate function") {
        Point p(1.0, 0.0);
        Point rotated_90 = rotate(p, 90.0);
        REQUIRE(rotated_90.time == Catch::Approx(0.0).margin(1e-10));
        REQUIRE(rotated_90.value == Catch::Approx(1.0));
        
        Point rotated_180 = rotate(p, 180.0);
        REQUIRE(rotated_180.time == Catch::Approx(-1.0));
        REQUIRE(rotated_180.value == Catch::Approx(0.0).margin(1e-10));
    }
}

TEST_CASE("Vector operations", "[handle_utils]") {
    Vector v1(2.0, 3.0);
    Vector v2(4.0, 1.0);
    
    SECTION("dot_product function") {
        double dot = dot_product(v1, v2);
        REQUIRE(dot == 11.0); // 2*4 + 3*1 = 11
        
        // Test perpendicular vectors
        Vector perp1(1.0, 0.0);
        Vector perp2(0.0, 1.0);
        REQUIRE(dot_product(perp1, perp2) == 0.0);
    }
    
    SECTION("cross_product function") {
        double cross = cross_product(v1, v2);
        REQUIRE(cross == -10.0); // 2*1 - 3*4 = -10
        
        // Test parallel vectors
        Vector parallel1(2.0, 3.0);
        Vector parallel2(4.0, 6.0);
        REQUIRE(cross_product(parallel1, parallel2) == 0.0);
    }
    
    SECTION("reflect function") {
        Vector incident(1.0, 1.0);
        Vector normal(0.0, 1.0); // Vertical normal
        Vector reflected = reflect(incident, normal);
        REQUIRE(reflected.time == 1.0);
        REQUIRE(reflected.value == -1.0);
    }
}

TEST_CASE("Constraint functions", "[handle_utils]") {
    SECTION("constrain_point_time_preserve_slope function") {
        Point origin(5.0, 5.0);
        Point p(8.0, 8.0); // slope = 1
        
        Point constrained = constrain_point_time_preserve_slope(origin, p, 0.0, 10.0);
        REQUIRE(constrained.time == 8.0); // Within bounds
        REQUIRE(constrained.value == 8.0);

        // Test out of bounds (too early)
        Point out_of_bounds(-1.0, -1.0); 
        Point constrained_min = constrain_point_time_preserve_slope(origin, out_of_bounds, 0.0, 10.0);
        REQUIRE(constrained_min.time == 0.0); 
        REQUIRE(constrained_min.value == Catch::Approx(5.0 - (5.0 - (-1.0)) * (5.0 - 0.0) / (5.0 - (-1.0)))); 
        REQUIRE(constrained_min.value == Catch::Approx(0.0));


        Point out_of_bounds2(12.0, 12.0); // slope = 1
        Point constrained_max = constrain_point_time_preserve_slope(origin, out_of_bounds2, 0.0, 10.0);
        REQUIRE(constrained_max.time == 10.0); // Clamped to max
                                               // (y' - 5)/(10 - 5) = 1 => y' - 5 = 5 => y' = 10.0
        REQUIRE(constrained_max.value == Catch::Approx(10.0));

        // Test with point already at min boundary
        Point at_min_boundary(0.0, 0.0); // slope = 1 relative to origin (5,5)
        Point constrained_at_min = constrain_point_time_preserve_slope(origin, at_min_boundary, 0.0, 10.0);
        REQUIRE(constrained_at_min.time == 0.0);
        REQUIRE(constrained_at_min.value == 0.0);

        // Test with point already at max boundary
        Point at_max_boundary(10.0, 10.0); // slope = 1 relative to origin (5,5)
        Point constrained_at_max = constrain_point_time_preserve_slope(origin, at_max_boundary, 0.0, 10.0);
        REQUIRE(constrained_at_max.time == 10.0);
        REQUIRE(constrained_at_max.value == 10.0);

        // Test with origin.time == p.time (vertical line)
        Point p_vertical(5.0, 10.0);
        Point constrained_vertical = constrain_point_time_preserve_slope(origin, p_vertical, 0.0, 10.0);
        REQUIRE(constrained_vertical.time == 5.0); // Time should not change
        REQUIRE(constrained_vertical.value == 10.0); // Value should not change

        // Test with origin.time == p.time and p.time out of bounds (should clamp time and keep value)
        Point p_vertical_out_min(2.0, 10.0); // origin (5,5)
        Point constrained_vertical_out_min = constrain_point_time_preserve_slope(origin, p_vertical_out_min, 3.0, 7.0);
        REQUIRE(constrained_vertical_out_min.time == 3.0);
        REQUIRE(constrained_vertical_out_min.value == Catch::Approx(5.0 + (10.0-5.0)*(3.0-5.0)/(2.0-5.0))); // 5 + 5*(-2)/(-3) = 5 + 10/3 = 8.333
        REQUIRE(constrained_vertical_out_min.value == Catch::Approx(5.0 + (10.0-5.0)*( (3.0-5.0) / (2.0-5.0) ) ));


        Point p_vertical_out_max(8.0, 10.0); // origin (5,5)
        Point constrained_vertical_out_max = constrain_point_time_preserve_slope(origin, p_vertical_out_max, 3.0, 7.0);
        REQUIRE(constrained_vertical_out_max.time == 7.0);
        REQUIRE(constrained_vertical_out_max.value == Catch::Approx(5.0 + (10.0-5.0)*( (7.0-5.0) / (8.0-5.0) ) ));


        // Test with min_time == max_time
        Point constrained_single_point_time = constrain_point_time_preserve_slope(origin, p, 6.0, 6.0);
        REQUIRE(constrained_single_point_time.time == 6.0);
        // Slope (8-5)/(8-5) = 1. (y' - 5)/(6-5) = 1 => y' - 5 = 1 => y' = 6
        REQUIRE(constrained_single_point_time.value == Catch::Approx(6.0));

        // Test with p.time == origin.time and min_time == max_time == origin.time
        Point constrained_origin_at_boundary = constrain_point_time_preserve_slope(origin, Point(5.0, 10.0), 5.0, 5.0);
        REQUIRE(constrained_origin_at_boundary.time == 5.0);
        REQUIRE(constrained_origin_at_boundary.value == 10.0); // Value should be preserved as slope is undefined but time is fixed.
    }
    
    SECTION("constrain_in_handle_time function") {
        Keyframe prev_kf(1.0, 2.0);
        Keyframe current_kf(5.0, 6.0);
        current_kf.in_handle = Point(3.0, 4.0); // Within bounds
        
        constrain_in_handle_time(current_kf, prev_kf);
        REQUIRE(current_kf.in_handle.time == 3.0); // Should remain unchanged
        
        current_kf.in_handle = Point(0.5, 4.0); // Out of bounds (too early)
        constrain_in_handle_time(current_kf, prev_kf);
        REQUIRE(current_kf.in_handle.time == 1.0); // Should be clamped to prev_kf time

        // Test when in_handle.time is already at prev_kf.position.time
        current_kf.in_handle = Point(1.0, 3.5);
        constrain_in_handle_time(current_kf, prev_kf);
        REQUIRE(current_kf.in_handle.time == 1.0);
        REQUIRE(current_kf.in_handle.value == 3.5);

        // Test when in_handle.time is greater than current_kf.position.time (should clamp to current_kf.position.time)
        current_kf.in_handle = Point(5.5, 4.0);
        constrain_in_handle_time(current_kf, prev_kf);
        REQUIRE(current_kf.in_handle.time == 5.0);

        // Test when in_handle.time is exactly current_kf.position.time
        current_kf.in_handle = Point(5.0, 4.0);
        constrain_in_handle_time(current_kf, prev_kf);
        REQUIRE(current_kf.in_handle.time == 5.0);
        REQUIRE(current_kf.in_handle.value == 4.0);
    }
    
    SECTION("constrain_out_handle_time function") {
        Keyframe current_kf(5.0, 6.0);
        Keyframe next_kf(10.0, 12.0);
        current_kf.out_handle = Point(7.0, 8.0); // Within bounds
        
        constrain_out_handle_time(current_kf, next_kf);
        REQUIRE(current_kf.out_handle.time == 7.0); // Should remain unchanged
        
        current_kf.out_handle = Point(12.0, 8.0); // Out of bounds (too late)
        constrain_out_handle_time(current_kf, next_kf);
        REQUIRE(current_kf.out_handle.time == 10.0); // Should be clamped to next_kf time

        // Test when out_handle.time is already at next_kf.position.time
        current_kf.out_handle = Point(10.0, 9.5);
        constrain_out_handle_time(current_kf, next_kf);
        REQUIRE(current_kf.out_handle.time == 10.0);
        REQUIRE(current_kf.out_handle.value == 9.5);

        // Test when out_handle.time is less than current_kf.position.time (should clamp to current_kf.position.time)
        current_kf.out_handle = Point(4.5, 8.0);
        constrain_out_handle_time(current_kf, next_kf);
        REQUIRE(current_kf.out_handle.time == 5.0);

        // Test when out_handle.time is exactly current_kf.position.time
        current_kf.out_handle = Point(5.0, 8.0);
        constrain_out_handle_time(current_kf, next_kf);
        REQUIRE(current_kf.out_handle.time == 5.0);
        REQUIRE(current_kf.out_handle.value == 8.0);
    }
}


TEST_CASE("Edge cases and special conditions", "[handle_utils]") {
    SECTION("Distance with identical points") {
        Point p(5.0, 3.0);
        REQUIRE(distance(p, p) == 0.0);
    }
    
    SECTION("Vector with identical points") {
        Point p(5.0, 3.0);
        Vector vec = vector(p, p);
        REQUIRE(vec.is_zero());
    }
    
    SECTION("Smooth handles with custom smooth_factor") {
        Keyframe prev_kf(0.0, 0.0);
        Keyframe current_kf(5.0, 5.0);
        Keyframe next_kf(10.0, 10.0);
        
        calculate_smooth_handles(current_kf, &prev_kf, &next_kf, 0.5);
        
        // With larger smooth factor, handles should be further from keyframe
        double handle_distance = distance(current_kf.position, current_kf.in_handle);
        REQUIRE(handle_distance > 0.0);
    }

    SECTION("Smooth handles with nullptr prev_kf") {
        Keyframe current_kf(5.0, 5.0);
        Keyframe next_kf(10.0, 0.0); // Different y to ensure non-horizontal slope
        double smooth_f = 0.333;
        
        calculate_smooth_handles(current_kf, nullptr, &next_kf, smooth_f);
        
        // In-handle should be to the left of current_kf, aligned with current_kf and next_kf
        // Out-handle should be to the right, also aligned
        REQUIRE(current_kf.in_handle.time < current_kf.position.time);
        REQUIRE(current_kf.out_handle.time > current_kf.position.time);

        // Check alignment and distance
        Vector v_segment = vector(current_kf.position, next_kf.position); // (5, -5)
        Vector v_in_handle = vector(current_kf.in_handle, current_kf.position);
        Vector v_out_handle = vector(current_kf.position, current_kf.out_handle);

        REQUIRE(cross_product(normalize(v_segment), normalize(v_in_handle)) == Catch::Approx(0.0).margin(1e-9));
        REQUIRE(cross_product(normalize(v_segment), normalize(v_out_handle)) == Catch::Approx(0.0).margin(1e-9));
        
        double expected_dist = distance(current_kf.position, next_kf.position) * smooth_f; // Removed division by 3.0
        REQUIRE(distance(current_kf.position, current_kf.out_handle) == Catch::Approx(expected_dist));
        REQUIRE(distance(current_kf.position, current_kf.in_handle) == Catch::Approx(expected_dist));
    }

    SECTION("Smooth handles with nullptr next_kf") {
        Keyframe prev_kf(0.0, 10.0); // Different y
        Keyframe current_kf(5.0, 5.0);
        double smooth_f = 0.333;
        
        calculate_smooth_handles(current_kf, &prev_kf, nullptr, smooth_f);

        REQUIRE(current_kf.in_handle.time < current_kf.position.time);
        REQUIRE(current_kf.out_handle.time > current_kf.position.time);

        Vector v_segment = vector(prev_kf.position, current_kf.position); // (5, -5)
        Vector v_in_handle = vector(current_kf.in_handle, current_kf.position);
        Vector v_out_handle = vector(current_kf.position, current_kf.out_handle);

        REQUIRE(cross_product(normalize(v_segment), normalize(v_in_handle)) == Catch::Approx(0.0).margin(1e-9));
        REQUIRE(cross_product(normalize(v_segment), normalize(v_out_handle)) == Catch::Approx(0.0).margin(1e-9));

        double expected_dist = distance(prev_kf.position, current_kf.position) * smooth_f; // Removed division by 3.0
        REQUIRE(distance(current_kf.position, current_kf.in_handle) == Catch::Approx(expected_dist));
        REQUIRE(distance(current_kf.position, current_kf.out_handle) == Catch::Approx(expected_dist));
    }

    SECTION("Smooth handles with nullptr prev_kf and next_kf (first/last keyframe)") {
        Keyframe current_kf(5.0, 5.0);
        
        calculate_smooth_handles(current_kf, nullptr, nullptr, 0.333333333);
        
        // For an isolated keyframe, the current implementation sets handles to the keyframe's position.
        REQUIRE(current_kf.in_handle.time == Catch::Approx(current_kf.position.time));
        REQUIRE(current_kf.in_handle.value == Catch::Approx(current_kf.position.value));
        REQUIRE(current_kf.out_handle.time == Catch::Approx(current_kf.position.time));
        REQUIRE(current_kf.out_handle.value == Catch::Approx(current_kf.position.value));
    }
    
    SECTION("Smooth handles with smooth_factor = 0") {
        Keyframe prev_kf(0.0, 0.0);
        Keyframe current_kf(5.0, 5.0);
        Keyframe next_kf(10.0, 10.0);
        
        calculate_smooth_handles(current_kf, &prev_kf, &next_kf, 0.0);
        
        // Handles should be at the keyframe position
        REQUIRE(current_kf.in_handle.time == Catch::Approx(current_kf.position.time));
        REQUIRE(current_kf.in_handle.value == Catch::Approx(current_kf.position.value));
        REQUIRE(current_kf.out_handle.time == Catch::Approx(current_kf.position.time));
        REQUIRE(current_kf.out_handle.value == Catch::Approx(current_kf.position.value));
    }
}

TEST_CASE("enforce_aligned_handles function", "[handle_utils]") {
    SECTION("Aligning in_handle from out_handle (out_handle is source)") {
        Keyframe kf(5.0, 5.0);
        kf.out_handle = Point(7.0, 7.0); // Source/Reference: Vector (2,2) from kf.position
        Point initial_in_handle_pos = Point(4.0, 3.0); // Target: Arbitrary initial position
        kf.in_handle = initial_in_handle_pos;

        enforce_aligned_handles(kf, nullptr, nullptr, true); // true means out_handle is source

        // in_handle should be on the line extending from out_handle through kf.position,
        // on the opposite side, preserving its original distance from kf.position.
        Vector original_in_vec = vector(kf.position, initial_in_handle_pos); // (-1, -2)
        double original_in_dist = length(original_in_vec); // sqrt(5)

        Vector out_vec_dir_from_kf = normalize(vector(kf.position, kf.out_handle)); // (1/sqrt(2), 1/sqrt(2))
        
        REQUIRE(kf.in_handle.time == Catch::Approx(kf.position.time - out_vec_dir_from_kf.time * original_in_dist));
        REQUIRE(kf.in_handle.value == Catch::Approx(kf.position.value - out_vec_dir_from_kf.value * original_in_dist));

        // Check alignment: vector from new in_handle to kf should be parallel to vector from kf to out_handle
        if (original_in_dist > 1e-9) { // Avoid normalizing zero vector if in_handle was at kf.position
            Vector final_in_vec_to_kf = vector(kf.in_handle, kf.position);
            Vector final_out_vec_from_kf = vector(kf.position, kf.out_handle);
            REQUIRE(cross_product(normalize(final_in_vec_to_kf), normalize(final_out_vec_from_kf)) == Catch::Approx(0.0).margin(1e-9));
        }
    }

    SECTION("Aligning out_handle from in_handle (in_handle is source)") {
        Keyframe kf(5.0, 5.0);
        kf.in_handle = Point(3.0, 3.0);  // Source/Reference: Vector (-2,-2) from kf.position
        Point initial_out_handle_pos = Point(6.0, 7.0); // Target: Arbitrary initial position
        kf.out_handle = initial_out_handle_pos;

        enforce_aligned_handles(kf, nullptr, nullptr, false); // false means in_handle is source

        // out_handle should be on the line extending from in_handle through kf.position,
        // on the opposite side, preserving its original distance from kf.position.
        Vector original_out_vec = vector(kf.position, initial_out_handle_pos); // (1, 2)
        double original_out_dist = length(original_out_vec); // sqrt(5)
        
        Vector in_vec_dir_to_kf = normalize(vector(kf.in_handle, kf.position)); // (1/sqrt(2), 1/sqrt(2))

        REQUIRE(kf.out_handle.time == Catch::Approx(kf.position.time + in_vec_dir_to_kf.time * original_out_dist));
        REQUIRE(kf.out_handle.value == Catch::Approx(kf.position.value + in_vec_dir_to_kf.value * original_out_dist));
        
        // Check alignment
        if (original_out_dist > 1e-9) { // Avoid normalizing zero vector if out_handle was at kf.position
            Vector final_in_vec_to_kf = vector(kf.in_handle, kf.position);
            Vector final_out_vec_from_kf = vector(kf.position, kf.out_handle);
            REQUIRE(cross_product(normalize(final_in_vec_to_kf), normalize(final_out_vec_from_kf)) == Catch::Approx(0.0).margin(1e-9));
        }
    }

    SECTION("Source handle at keyframe position") {
        Keyframe kf(5.0, 5.0);

        // These are wrong
        // SECTION("out_handle (source) at keyframe position, in_handle (target) is modified") {
        //     kf.out_handle = kf.position; 
        //     kf.in_handle = Point(3.0, 4.0);  // Initial position of target handle
            
        //     enforce_aligned_handles(kf, true); // out_handle is source

        //     // in_handle should be moved to keyframe's position
        //     REQUIRE(kf.in_handle.time == Catch::Approx(kf.position.time));
        //     REQUIRE(kf.in_handle.value == Catch::Approx(kf.position.value));
        // }

        // SECTION("in_handle (source) at keyframe position, out_handle (target) is modified") {
        //     kf.in_handle = kf.position;
        //     kf.out_handle = Point(7.0, 6.0); // Initial position of target handle

        //     enforce_aligned_handles(kf, false); // in_handle is source
            
        //     // out_handle should be moved to keyframe's position
        //     REQUIRE(kf.out_handle.time == Catch::Approx(kf.position.time));
        //     REQUIRE(kf.out_handle.value == Catch::Approx(kf.position.value));
        // }
    }

    SECTION("Target handle at keyframe position (source handle is not)") {
        Keyframe kf(5.0, 5.0);

        SECTION("in_handle (target) at keyframe position, out_handle (source) is not") {
            kf.out_handle = Point(7.0, 7.0); // Source handle is not at kf.position
            kf.in_handle = kf.position;      // Target handle is already at kf.position
            
            enforce_aligned_handles(kf, nullptr, nullptr, true); // out_handle is source, in_handle is target

            // in_handle should remain at keyframe's position (distance is 0)
            REQUIRE(kf.in_handle.time == Catch::Approx(kf.position.time));
            REQUIRE(kf.in_handle.value == Catch::Approx(kf.position.value));
        }

        SECTION("out_handle (target) at keyframe position, in_handle (source) is not") {
            kf.in_handle = Point(3.0, 3.0);   // Source handle is not at kf.position
            kf.out_handle = kf.position;     // Target handle is already at kf.position

            enforce_aligned_handles(kf, nullptr, nullptr, false); // in_handle is source, out_handle is target
            
            // out_handle should remain at keyframe's position (distance is 0)
            REQUIRE(kf.out_handle.time == Catch::Approx(kf.position.time));
            REQUIRE(kf.out_handle.value == Catch::Approx(kf.position.value));
        }
    }
}
