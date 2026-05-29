#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/handle_utils.hpp>
#include <cmath>
#include <limits>

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
        double len = length(normalized);
        REQUIRE(len == Catch::Approx(1.0));
    }
    
    SECTION("normalize zero vector throws exception") {
        Vector zero_vec(0.0, 0.0);
        REQUIRE_THROWS_AS(normalize(zero_vec), std::domain_error);
    }

    SECTION("length function") {
        Vector vec(3.0, 4.0);
        REQUIRE(length(vec) == Catch::Approx(5.0));
        
        Vector zero_vec(0.0, 0.0);
        REQUIRE(length(zero_vec) == Catch::Approx(0.0));
    }

    SECTION("length_squared function") {
        Vector vec(3.0, 4.0);
        REQUIRE(length_squared(vec) == Catch::Approx(25.0));
        
        Vector zero_vec(0.0, 0.0);
        REQUIRE(length_squared(zero_vec) == Catch::Approx(0.0));
    }

    SECTION("nearly_equal function") {
        REQUIRE(nearly_equal(1.0, 1.0));
        REQUIRE(nearly_equal(1.0, 1.0 + std::numeric_limits<double>::epsilon()));
        REQUIRE_FALSE(nearly_equal(1.0, 1.0 + 1e-6));
        REQUIRE_FALSE(nearly_equal(1.0, 1.1));
        REQUIRE(nearly_equal(0.0, 0.0));
        REQUIRE(nearly_equal(0.0, std::numeric_limits<double>::epsilon()));
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

    SECTION("invert function") {
        Vector vec(3.0, -4.0);
        Vector inverted = invert(vec);
        REQUIRE(inverted.time == -3.0);
        REQUIRE(inverted.value == 4.0);
    }
}

TEST_CASE("Constraint functions", "[handle_utils]") {
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

    SECTION("constrain_handles_time function") {
        Keyframe prev_kf(1.0, 2.0);
        Keyframe current_kf(5.0, 6.0);
        Keyframe next_kf(10.0, 12.0);
        
        current_kf.in_handle = Point(0.5, 4.0);  // Out of bounds
        current_kf.out_handle = Point(12.0, 8.0); // Out of bounds
        
        constrain_handles_time(current_kf, &prev_kf, &next_kf);
        
        REQUIRE(current_kf.in_handle.time == 1.0);  // Clamped to prev_kf time
        REQUIRE(current_kf.out_handle.time == 10.0); // Clamped to next_kf time
    }
}

TEST_CASE("Handle mode functions", "[handle_utils]") {
    SECTION("apply_flat_handles function") {
        Keyframe prev_kf(0.0, 0.0);
        Keyframe current_kf(5.0, 10.0);
        Keyframe next_kf(10.0, 5.0);

        // Flat handles are adjustable in time, so apply_flat_handles preserves
        // each handle's existing time and only flattens its value. Give the
        // handles meaningful times within their valid bounds up front.
        current_kf.in_handle  = Point(4.0, 0.0);
        current_kf.out_handle = Point(6.0, 0.0);

        apply_flat_handles(current_kf, &prev_kf, &next_kf);

        // Handles should be flattened to the keyframe value while keeping time
        REQUIRE(current_kf.in_handle.value == current_kf.position.value);
        REQUIRE(current_kf.out_handle.value == current_kf.position.value);
        REQUIRE(current_kf.in_handle.time == Catch::Approx(4.0));
        REQUIRE(current_kf.out_handle.time == Catch::Approx(6.0));
    }
    
    SECTION("apply_smooth_handles function") {
        Keyframe prev_kf(0.0, 0.0);
        Keyframe current_kf(5.0, 5.0);
        Keyframe next_kf(10.0, 10.0);
        
        apply_smooth_handles(current_kf, &prev_kf, &next_kf);
        
        // Handles should be positioned to create smooth transitions
        REQUIRE(current_kf.in_handle.time < current_kf.position.time);
        REQUIRE(current_kf.out_handle.time > current_kf.position.time);
        
        // Test with custom smooth factor
        apply_smooth_handles(current_kf, &prev_kf, &next_kf, 0.5);
        double handle_distance = distance(current_kf.position, current_kf.in_handle);
        REQUIRE(handle_distance > 0.0);
    }

    SECTION("apply_aligned_handles function") {
        Keyframe kf(5.0, 5.0);
        kf.out_handle = Point(7.0, 7.0);
        kf.in_handle = Point(3.0, 4.0);
        
        apply_aligned_handles(kf, nullptr, nullptr, GrabbedHandle::out_handle); // out_handle is source
        
        // in_handle should be aligned with out_handle
        Vector in_vec = vector(kf.in_handle, kf.position);
        Vector out_vec = vector(kf.position, kf.out_handle);
        REQUIRE(std::abs(cross_product(normalize(in_vec), normalize(out_vec))) < 1e-9);
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
    
    SECTION("Smooth handles with nullptr prev_kf") {
        Keyframe current_kf(5.0, 5.0);
        Keyframe next_kf(10.0, 0.0);
        
        apply_smooth_handles(current_kf, nullptr, &next_kf, 0.333);
        
        REQUIRE(current_kf.in_handle.time < current_kf.position.time);
        REQUIRE(current_kf.out_handle.time > current_kf.position.time);
    }

    SECTION("Smooth handles with nullptr next_kf") {
        Keyframe prev_kf(0.0, 10.0);
        Keyframe current_kf(5.0, 5.0);
        
        apply_smooth_handles(current_kf, &prev_kf, nullptr, 0.333);

        REQUIRE(current_kf.in_handle.time < current_kf.position.time);
        REQUIRE(current_kf.out_handle.time > current_kf.position.time);
    }

    SECTION("Smooth handles with nullptr prev_kf and next_kf") {
        Keyframe current_kf(5.0, 5.0);
        
        apply_smooth_handles(current_kf, nullptr, nullptr, 0.333333333);
        
        // For an isolated keyframe, handles should be flat with an offset of 1.0
        REQUIRE(current_kf.in_handle.time == Catch::Approx(current_kf.position.time - 1.0));
        REQUIRE(current_kf.in_handle.value == Catch::Approx(current_kf.position.value));
        REQUIRE(current_kf.out_handle.time == Catch::Approx(current_kf.position.time + 1.0));
        REQUIRE(current_kf.out_handle.value == Catch::Approx(current_kf.position.value));
    }
    
    SECTION("Smooth handles with smooth_factor = 0") {
        Keyframe prev_kf(0.0, 0.0);
        Keyframe current_kf(5.0, 5.0);
        Keyframe next_kf(10.0, 10.0);
        
        apply_smooth_handles(current_kf, &prev_kf, &next_kf, 0.0);
        
        // Handles should be at the keyframe position
        REQUIRE(current_kf.in_handle.time == Catch::Approx(current_kf.position.time));
        REQUIRE(current_kf.in_handle.value == Catch::Approx(current_kf.position.value));
        REQUIRE(current_kf.out_handle.time == Catch::Approx(current_kf.position.time));
        REQUIRE(current_kf.out_handle.value == Catch::Approx(current_kf.position.value));
    }
}

TEST_CASE("Function and HandleMode constraints for in_handle time", "[handle_utils]") {
    SECTION("Function::Linear must constrain in_handle by keyframe time") {
        Keyframe prev_kf(1.0, 2.0, Function::Bezier, HandleMode::Free);
        Keyframe current_kf(5.0, 6.0, Function::Linear);
        
        // Set in_handle beyond the keyframe time (violates constraint)
        current_kf.in_handle = Point(6.0, 4.0); // time > keyframe.position.time
        
        // Apply linear function constraints - this should clamp in_handle.time
        ensure_linear_handles_time_boundary(current_kf, &prev_kf, nullptr);
        
        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
    }
    
    SECTION("Function::Constant must constrain in_handle by keyframe time") {
        Keyframe prev_kf(1.0, 2.0, Function::Bezier, HandleMode::Free);
        Keyframe current_kf(5.0, 6.0, Function::Constant);
        
        // Set in_handle beyond the keyframe time (violates constraint)
        current_kf.in_handle = Point(5.5, 4.0); // time > keyframe.position.time
        
        // Apply constant function constraints - this should clamp in_handle.time
        ensure_linear_handles_time_boundary(current_kf, &prev_kf, nullptr);
        
        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
    }
    
    SECTION("HandleMode::Flat must constrain in_handle by keyframe time") {
        Keyframe prev_kf(1.0, 2.0, Function::Bezier, HandleMode::Free);
        Keyframe current_kf(5.0, 6.0, Function::Bezier, HandleMode::Flat);
        
        // Set in_handle beyond the keyframe time (violates constraint)
        current_kf.in_handle = Point(5.2, 6.0); // time > keyframe.position.time
        
        apply_flat_handles(current_kf, &prev_kf, nullptr);
        
        // This should already work but let's test the additional constraint
        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
        
        // Also test that if we manually set a bad in_handle after apply_flat_handles,
        // we need a way to constrain it
        current_kf.in_handle = Point(5.3, 6.0); // violates constraint again
        constrain_in_handle_time(current_kf, prev_kf);
        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
    }
    
    SECTION("HandleMode::Smooth must constrain in_handle by keyframe time") {
        Keyframe prev_kf(1.0, 2.0, Function::Bezier, HandleMode::Free);
        Keyframe current_kf(5.0, 6.0, Function::Bezier, HandleMode::Smooth);
        
        // Set in_handle beyond the keyframe time (violates constraint)
        current_kf.in_handle = Point(5.1, 5.5); // time > keyframe.position.time
        
        apply_smooth_handles(current_kf, &prev_kf, nullptr);
        
        // This should already work but let's test the additional constraint
        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
        
        // Also test that if we manually set a bad in_handle after apply_smooth_handles,
        // we need a way to constrain it
        current_kf.in_handle = Point(5.2, 5.5); // violates constraint again
        constrain_in_handle_time(current_kf, prev_kf);
        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
    }
    
    SECTION("Preceding keyframe with adjustable modes should not affect constraints") {
        // Test case where preceding keyframe has HandleMode::Free (adjustable)
        Keyframe prev_kf(1.0, 2.0, Function::Bezier, HandleMode::Free);
        prev_kf.out_handle = Point(6.0, 10.0); // out_handle beyond current keyframe time
        
        Keyframe current_kf(5.0, 6.0, Function::Linear);
        current_kf.in_handle = Point(5.5, 4.0); // violates constraint
        
        // Apply constraints - should constrain in_handle regardless of prev_kf.out_handle
        constrain_in_handle_time(current_kf, prev_kf);
        
        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
        REQUIRE(current_kf.in_handle.time == 5.0); // Should be clamped to keyframe time
    }
    
    SECTION("Function::Linear keyframe with preceding adjustable mode") {
        // Preceding keyframe with HandleMode::AlignAdjustable (adjustable mode)
        Keyframe prev_kf(2.0, 1.0, Function::Bezier, HandleMode::AlignAdjustable);
        Keyframe current_kf(10.0, 5.0, Function::Linear);
        
        // Simulate handles being set incorrectly
        current_kf.in_handle = Point(10.5, 4.0); // time > keyframe.position.time
        
        // Apply the constraints via the update_handles logic
        ensure_linear_handles_time_boundary(current_kf, &prev_kf, nullptr);
        
        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
    }
    
    SECTION("Function::Constant keyframe with preceding adjustable mode") {
        // Preceding keyframe with HandleMode::Free (adjustable mode)
        Keyframe prev_kf(3.0, 2.0, Function::Bezier, HandleMode::Free);
        Keyframe current_kf(8.0, 7.0, Function::Constant);
        
        // Simulate handles being set incorrectly
        current_kf.in_handle = Point(8.2, 6.0); // time > keyframe.position.time
        
        // Apply the constraints via the update_handles logic
        ensure_linear_handles_time_boundary(current_kf, &prev_kf, nullptr);
        
        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
    }
    
    SECTION("HandleMode::Flat with preceding HandleMode::Aligned") {
        // Preceding keyframe with HandleMode::Aligned (adjustable mode)
        Keyframe prev_kf(1.5, 3.0, Function::Bezier, HandleMode::Aligned);
        Keyframe current_kf(6.0, 8.0, Function::Bezier, HandleMode::Flat);
        
        // Manually set invalid in_handle after applying flat handles
        apply_flat_handles(current_kf, &prev_kf, nullptr);
        current_kf.in_handle = Point(6.1, 8.0); // time > keyframe.position.time
        
        // Apply constraints again
        constrain_in_handle_time(current_kf, prev_kf);
        
        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
    }
    
    SECTION("HandleMode::Smooth with preceding HandleMode::AlignFlex") {
        // Preceding keyframe with HandleMode::AlignFlex (adjustable mode)
        Keyframe prev_kf(2.5, 4.0, Function::Bezier, HandleMode::AlignFlex);
        Keyframe current_kf(7.0, 9.0, Function::Bezier, HandleMode::Smooth);
        
        // Manually set invalid in_handle after applying smooth handles
        apply_smooth_handles(current_kf, &prev_kf, nullptr);
        current_kf.in_handle = Point(7.3, 8.5); // time > keyframe.position.time
        
        // Apply constraints again
        constrain_in_handle_time(current_kf, prev_kf);

        REQUIRE(current_kf.in_handle.time <= current_kf.position.time);
    }
}

TEST_CASE("apply_flat_handles isolated and degenerate cases", "[handle_utils]") {
    SECTION("Only keyframe (no neighbors) uses +/-1.0 time offset") {
        Keyframe kf(5.0, 10.0);
        apply_flat_handles(kf, nullptr, nullptr);
        REQUIRE(kf.in_handle.time == Catch::Approx(4.0));
        REQUIRE(kf.out_handle.time == Catch::Approx(6.0));
        REQUIRE(kf.in_handle.value == Catch::Approx(10.0));
        REQUIRE(kf.out_handle.value == Catch::Approx(10.0));
    }

    SECTION("Neighbor sharing the keyframe time does not crash and flattens value") {
        Keyframe prev_kf(5.0, 0.0);     // same time as current keyframe
        Keyframe current_kf(5.0, 10.0);
        Keyframe next_kf(7.0, 5.0);
        current_kf.in_handle = Point(4.0, 0.0);
        current_kf.out_handle = Point(6.0, 0.0);

        apply_flat_handles(current_kf, &prev_kf, &next_kf);

        // Values are flattened to the keyframe value
        REQUIRE(current_kf.in_handle.value == Catch::Approx(10.0));
        REQUIRE(current_kf.out_handle.value == Catch::Approx(10.0));
        // in_handle clamped into [prev.time, kf.time] == [5,5]
        REQUIRE(current_kf.in_handle.time == Catch::Approx(5.0));
    }
}

TEST_CASE("apply_aligned_handles source selection and modes", "[handle_utils]") {
    auto collinearity = [](const Keyframe& kf) {
        Vector in_vec = vector(kf.in_handle, kf.position);
        Vector out_vec = vector(kf.position, kf.out_handle);
        return std::abs(cross_product(normalize(in_vec), normalize(out_vec)));
    };

    SECTION("in_handle as source aligns the out_handle") {
        Keyframe kf(5.0, 5.0);
        kf.in_handle = Point(3.0, 4.0);
        kf.out_handle = Point(7.0, 7.0);
        apply_aligned_handles(kf, nullptr, nullptr, GrabbedHandle::in_handle);
        REQUIRE(collinearity(kf) < 1e-9);
    }

    SECTION("GrabbedHandle::none picks the larger handle as source and aligns") {
        Keyframe kf(5.0, 5.0);
        kf.in_handle = Point(4.5, 4.8);    // small magnitude
        kf.out_handle = Point(9.0, 9.0);   // large magnitude -> chosen as source
        apply_aligned_handles(kf, nullptr, nullptr, GrabbedHandle::none);
        REQUIRE(collinearity(kf) < 1e-9);
    }

    SECTION("AlignStrict makes both handle magnitudes equal") {
        Keyframe kf(5.0, 5.0);
        kf.handle_mode = HandleMode::AlignStrict;
        kf.out_handle = Point(7.0, 7.0);
        kf.in_handle = Point(4.0, 4.5);
        apply_aligned_handles(kf, nullptr, nullptr, GrabbedHandle::out_handle);

        double in_mag = distance(kf.position, kf.in_handle);
        double out_mag = distance(kf.position, kf.out_handle);
        REQUIRE(in_mag == Catch::Approx(out_mag));
        REQUIRE(collinearity(kf) < 1e-9);
    }
}

TEST_CASE("ensure_handle_time_boundary direct behavior", "[handle_utils]") {
    SECTION("in_handle crossing the prev boundary is scaled back onto it") {
        Point kf_pos(5.0, 5.0);
        Point in_handle(2.0, 2.0);     // time 2.0 is before boundary 3.0 -> violation
        Point boundary(3.0, 0.0);
        ensure_handle_time_boundary(kf_pos, in_handle, boundary, true);
        REQUIRE(in_handle.time == Catch::Approx(3.0));
        REQUIRE(in_handle.value == Catch::Approx(3.0)); // stays on the kf->handle line
    }

    SECTION("out_handle crossing the next boundary is scaled back onto it") {
        Point kf_pos(5.0, 5.0);
        Point out_handle(8.0, 8.0);    // time 8.0 is past boundary 7.0 -> violation
        Point boundary(7.0, 0.0);
        ensure_handle_time_boundary(kf_pos, out_handle, boundary, false);
        REQUIRE(out_handle.time == Catch::Approx(7.0));
        REQUIRE(out_handle.value == Catch::Approx(7.0));
    }

    SECTION("Vertical handle (zero time delta) is left unchanged") {
        Point kf_pos(5.0, 5.0);
        Point in_handle(5.0, 2.0);     // same time as keyframe -> zero time delta
        Point boundary(6.0, 0.0);      // would be a violation, but the guard bails out
        ensure_handle_time_boundary(kf_pos, in_handle, boundary, true);
        REQUIRE(in_handle.time == Catch::Approx(5.0));
        REQUIRE(in_handle.value == Catch::Approx(2.0));
    }
}
