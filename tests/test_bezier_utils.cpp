#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/bezier_utils.hpp>

using namespace anim;

TEST_CASE("Bezier curve evaluation", "[bezier_utils]") {
    SECTION("Evaluate cubic Bézier at t=0") {
        Point p0(1.0, 2.0);
        Point p1(2.0, 3.0);
        Point p2(3.0, 4.0);
        Point p3(4.0, 5.0);
        
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, 0.0);
        REQUIRE(result.time == Catch::Approx(p0.time));
        REQUIRE(result.value == Catch::Approx(p0.value));
    }
    
    SECTION("Evaluate cubic Bézier at t=1") {
        Point p0(1.0, 2.0);
        Point p1(2.0, 3.0);
        Point p2(3.0, 4.0);
        Point p3(4.0, 5.0);
        
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, 1.0);
        REQUIRE(result.time == Catch::Approx(p3.time));
        REQUIRE(result.value == Catch::Approx(p3.value));
    }
    
    SECTION("Evaluate cubic Bézier at t=0.5") {
        Point p0(0.0, 0.0);
        Point p1(0.0, 1.0);
        Point p2(1.0, 1.0);
        Point p3(1.0, 0.0);
        
        // This forms a symmetric curve, so at t=0.5 we should be at (0.5, 0.75)
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, 0.5);
        REQUIRE(result.time == Catch::Approx(0.5));
        REQUIRE(result.value == Catch::Approx(0.75));
    }
    
    SECTION("Evaluate cubic Bézier with invalid t") {
        Point p0(0.0, 0.0);
        Point p1(0.0, 1.0);
        Point p2(1.0, 1.0);
        Point p3(1.0, 0.0);
        
        REQUIRE_THROWS_AS(bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, -0.1), std::invalid_argument);
        REQUIRE_THROWS_AS(bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, 1.1), std::invalid_argument);
    }

    SECTION("Evaluate with coincident control points") {
        Point p_start(0.0, 0.0);
        Point p_end(3.0, 3.0);
        Point mid(1.5, 1.5);

        // Case 1: p0 == p1
        Point r1 = bezier_utils::evaluate_cubic_bezier(p_start, p_start, mid, p_end, 0.5);
        // Exact value depends on p2, p3. Just ensure it runs.
        // For p0=p1, curve starts moving towards p2. (1-t)^3 p0 + 3t(1-t)^2 p0 + 3t^2(1-t)p2 + t^3 p3
        // = ( (1-t)^3 + 3t(1-t)^2 ) p0 + 3t^2(1-t)p2 + t^3 p3
        // = (1-t)^2 (1-t+3t) p0 + ... = (1-t)^2 (1+2t) p0 + ...
        // At t=0.5: (0.25 * 2)p0 + (3*0.25*0.5)mid + 0.125*p_end = 0.5*p_start + 0.375*mid + 0.125*p_end
        Point expected_r1 = p_start * 0.5 + mid * 0.375 + p_end * 0.125;
        REQUIRE(r1.time == Catch::Approx(expected_r1.time));
        REQUIRE(r1.value == Catch::Approx(expected_r1.value));

        // Case 2: p1 == p2
        Point r2 = bezier_utils::evaluate_cubic_bezier(p_start, mid, mid, p_end, 0.5);
        // (1-t)^3 p0 + (3t(1-t)^2 + 3t^2(1-t)) p1 + t^3 p3
        // = (1-t)^3 p0 + 3t(1-t)( (1-t) + t ) p1 + t^3 p3 = (1-t)^3 p0 + 3t(1-t)p1 + t^3 p3
        // At t=0.5: 0.125*p_start + (3*0.5*0.5)*mid + 0.125*p_end = 0.125*p_start + 0.75*mid + 0.125*p_end
        Point expected_r2 = p_start * 0.125 + mid * 0.75 + p_end * 0.125;
        REQUIRE(r2.time == Catch::Approx(expected_r2.time));
        REQUIRE(r2.value == Catch::Approx(expected_r2.value));
        
        // Case 3: All points coincident
        Point p(1.0, 1.0);
        Point r3 = bezier_utils::evaluate_cubic_bezier(p, p, p, p, 0.5);
        REQUIRE(r3.time == Catch::Approx(p.time));
        REQUIRE(r3.value == Catch::Approx(p.value));
    }
}

TEST_CASE("Finding parameter for time", "[bezier_utils]") {
    SECTION("Parameter at endpoints") {
        Point p0(1.0, 2.0);
        Point p1(2.0, 3.0);
        Point p2(3.0, 4.0);
        Point p3(4.0, 5.0);
        
        double t_start = bezier_utils::find_parameter_for_time(p0, p1, p2, p3, 1.0);
        REQUIRE(t_start == Catch::Approx(0.0));
        
        double t_end = bezier_utils::find_parameter_for_time(p0, p1, p2, p3, 4.0);
        REQUIRE(t_end == Catch::Approx(1.0));
    }
    
    SECTION("Parameter for linear segment") {
        // Create a linear Bézier (p1 and p2 on the line between p0 and p3)
        Point p0(0.0, 0.0);
        Point p3(3.0, 3.0);
        Point p1(1.0, 1.0);
        Point p2(2.0, 2.0);
        
        double t_mid = bezier_utils::find_parameter_for_time(p0, p1, p2, p3, 1.5);
        REQUIRE(t_mid == Catch::Approx(0.5).margin(0.01));
        
        // Verify the value at this parameter
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t_mid);
        REQUIRE(result.time == Catch::Approx(1.5).margin(0.01));
        REQUIRE(result.value == Catch::Approx(1.5).margin(0.01));
    }
    
    SECTION("Parameter for curved segment") {
        // Create a curve where time and parameter don't have a linear relationship
        Point p0(0.0, 0.0);
        Point p1(0.0, 1.0);  // Vertical handle
        Point p2(3.0, 2.0);  // Handle closer to p3
        Point p3(3.0, 3.0);
        
        // Find the parameter for a specific time
        double t = bezier_utils::find_parameter_for_time(p0, p1, p2, p3, 1.5);
        
        // Evaluate to verify
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
        REQUIRE(result.time == Catch::Approx(1.5).margin(0.01));
    }
    
    SECTION("Out of range time") {
        Point p0(1.0, 2.0);
        Point p1(2.0, 3.0);
        Point p2(3.0, 4.0);
        Point p3(4.0, 5.0);
        
        REQUIRE_THROWS_AS(bezier_utils::find_parameter_for_time(p0, p1, p2, p3, 0.5), std::out_of_range);
        REQUIRE_THROWS_AS(bezier_utils::find_parameter_for_time(p0, p1, p2, p3, 4.5), std::out_of_range);
    }

    SECTION("Parameter when p0.time == p3.time") {
        Point p_const_time(1.0, 0.0);
        Point p1(1.0, 1.0); // All points have same time
        Point p2(1.0, 2.0);
        Point p3_const_time(1.0, 3.0);

        // Target time is the constant time of the segment
        double t_param = bezier_utils::find_parameter_for_time(p_const_time, p1, p2, p3_const_time, 1.0);
        // The function should return a valid t, e.g. 0 or 1, or something in between.
        // The actual value of t might vary depending on implementation details if dx/dt is always 0.
        // We check that the evaluated time at returned t is correct.
        Point result = bezier_utils::evaluate_cubic_bezier(p_const_time, p1, p2, p3_const_time, t_param);
        REQUIRE(result.time == Catch::Approx(1.0));

        // Target time is different from the constant time
        REQUIRE_THROWS_AS(bezier_utils::find_parameter_for_time(p_const_time, p1, p2, p3_const_time, 2.0), std::out_of_range);
    }
}

TEST_CASE("Creating Bezier handles", "[bezier_utils]") {
    SECTION("Linear Bézier handles") {
        Point p0(1.0, 2.0);
        Point p3(4.0, 5.0);
        Point p1, p2;
        
        bezier_utils::create_linear_bezier_handles(p0, p3, p1, p2);
        
        // Handles should be at 1/3 intervals
        REQUIRE(p1.time == Catch::Approx(p0.time + (p3.time - p0.time) / 3.0));
        REQUIRE(p1.value == Catch::Approx(p0.value + (p3.value - p0.value) / 3.0));
        REQUIRE(p2.time == Catch::Approx(p3.time - (p3.time - p0.time) / 3.0));
        REQUIRE(p2.value == Catch::Approx(p3.value - (p3.value - p0.value) / 3.0));
        
        // Verify that this does indeed create a nearly linear curve
        for (double t = 0.1; t < 1.0; t += 0.1) {
            double expected_time = p0.time + t * (p3.time - p0.time);
            double expected_value = p0.value + t * (p3.value - p0.value);
            
            Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
            REQUIRE(result.time == Catch::Approx(expected_time).margin(0.01));
            REQUIRE(result.value == Catch::Approx(expected_value).margin(0.01));
        }
    }
    
    SECTION("Linear Bézier handles with p0 == p3") {
        Point p0(1.0, 2.0);
        Point p3 = p0; // p0 and p3 are the same
        Point p1, p2;
        
        bezier_utils::create_linear_bezier_handles(p0, p3, p1, p2);
        
        REQUIRE(p1.time == Catch::Approx(p0.time));
        REQUIRE(p1.value == Catch::Approx(p0.value));
        REQUIRE(p2.time == Catch::Approx(p0.time));
        REQUIRE(p2.value == Catch::Approx(p0.value));
    }

    SECTION("Flat Bézier handles") {
        Point kf(2.0, 3.0);
        Point in_handle, out_handle;
        double time_offset = 0.5;
        
        bezier_utils::create_flat_bezier_handles(kf, time_offset, in_handle, out_handle);
        
        // Handles should be horizontally offset from the keyframe
        REQUIRE(in_handle.time == Catch::Approx(kf.time - time_offset));
        REQUIRE(in_handle.value == Catch::Approx(kf.value));
        REQUIRE(out_handle.time == Catch::Approx(kf.time + time_offset));
        REQUIRE(out_handle.value == Catch::Approx(kf.value));
    }

    SECTION("Flat Bézier handles with zero time_offset") {
        Point kf(2.0, 3.0);
        Point in_handle, out_handle;
        double time_offset = 0.0;
        
        bezier_utils::create_flat_bezier_handles(kf, time_offset, in_handle, out_handle);
        
        REQUIRE(in_handle.time == Catch::Approx(kf.time));
        REQUIRE(in_handle.value == Catch::Approx(kf.value));
        REQUIRE(out_handle.time == Catch::Approx(kf.time));
        REQUIRE(out_handle.value == Catch::Approx(kf.value));
    }

    SECTION("Flat Bézier handles with negative time_offset") {
        Point kf(2.0, 3.0);
        Point in_handle, out_handle;
        double time_offset = -0.5; // Negative offset
        
        bezier_utils::create_flat_bezier_handles(kf, time_offset, in_handle, out_handle);
        
        // Assuming current behavior: in_handle is kf.time - time_offset, out_handle is kf.time + time_offset
        REQUIRE(in_handle.time == Catch::Approx(kf.time - time_offset)); // kf.time + 0.5
        REQUIRE(in_handle.value == Catch::Approx(kf.value));
        REQUIRE(out_handle.time == Catch::Approx(kf.time + time_offset)); // kf.time - 0.5
        REQUIRE(out_handle.value == Catch::Approx(kf.value));

        // Verify relative positions
        REQUIRE(in_handle.time > kf.time); // In handle is to the right
        REQUIRE(out_handle.time < kf.time); // Out handle is to the left
    }
}
