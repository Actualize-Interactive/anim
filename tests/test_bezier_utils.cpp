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
        Point expected_r1 = p_start * 0.5 + mid * 0.375 + p_end * 0.125;
        REQUIRE(r1.time == Catch::Approx(expected_r1.time));
        REQUIRE(r1.value == Catch::Approx(expected_r1.value));

        // Case 2: p1 == p2
        Point r2 = bezier_utils::evaluate_cubic_bezier(p_start, mid, mid, p_end, 0.5);
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

TEST_CASE("Solving parameter for time", "[bezier_utils]") {
    SECTION("Parameter at endpoints using bisection") {
        Point p0(1.0, 2.0);
        Point p1(2.0, 3.0);
        Point p2(3.0, 4.0);
        Point p3(4.0, 5.0);
        
        double t_start = bezier_utils::solve_t_for_time_bisection(p0, p1, p2, p3, 1.0);
        REQUIRE(t_start == Catch::Approx(0.0));
        
        double t_end = bezier_utils::solve_t_for_time_bisection(p0, p1, p2, p3, 4.0);
        REQUIRE(t_end == Catch::Approx(1.0));
    }
    
    SECTION("Parameter for linear segment using bisection") {
        // Create a linear Bézier (p1 and p2 on the line between p0 and p3)
        Point p0(0.0, 0.0);
        Point p3(3.0, 3.0);
        Point p1(1.0, 1.0);
        Point p2(2.0, 2.0);
        
        double t_mid = bezier_utils::solve_t_for_time_bisection(p0, p1, p2, p3, 1.5);
        REQUIRE(t_mid == Catch::Approx(0.5).margin(0.01));
        
        // Verify the value at this parameter
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t_mid);
        REQUIRE(result.time == Catch::Approx(1.5).margin(0.01));
        REQUIRE(result.value == Catch::Approx(1.5).margin(0.01));
    }
    
    SECTION("Parameter for curved segment using Newton-Raphson") {
        // Create a curve where time and parameter don't have a linear relationship
        Point p0(0.0, 0.0);
        Point p1(0.0, 1.0);  // Vertical handle
        Point p2(3.0, 2.0);  // Handle closer to p3
        Point p3(3.0, 3.0);
        
        // Find the parameter for a specific time using Newton-Raphson
        double t = bezier_utils::solve_t_for_time(p0, p1, p2, p3, 1.5);
        
        // Evaluate to verify
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
        REQUIRE(result.time == Catch::Approx(1.5).margin(0.01));
    }
    
    SECTION("Newton-Raphson with initial guess") {
        Point p0(0.0, 0.0);
        Point p1(1.0, 1.0);
        Point p2(2.0, 2.0);
        Point p3(3.0, 3.0);
        
        double initial_guess = 0.4;
        double t = bezier_utils::solve_t_for_time(p0, p1, p2, p3, 1.2, &initial_guess);
        
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
        REQUIRE(result.time == Catch::Approx(1.2).margin(0.01));
    }

    SECTION("Parameter when p0.time == p3.time using bisection") {
        Point p_const_time(1.0, 0.0);
        Point p1(1.0, 1.0); // All points have same time
        Point p2(1.0, 2.0);
        Point p3_const_time(1.0, 3.0);

        // Target time is the constant time of the segment
        double t_param = bezier_utils::solve_t_for_time_bisection(p_const_time, p1, p2, p3_const_time, 1.0);
        Point result = bezier_utils::evaluate_cubic_bezier(p_const_time, p1, p2, p3_const_time, t_param);
        REQUIRE(result.time == Catch::Approx(1.0));

        // Target time is different from the constant time should be handled gracefully
        // Note: This might throw or return a boundary value depending on implementation
    }
}

TEST_CASE("Time component evaluation", "[bezier_utils]") {
    SECTION("Evaluate time component") {
        Point p0(1.0, 2.0);
        Point p1(2.0, 3.0);
        Point p2(3.0, 4.0);
        Point p3(4.0, 5.0);
        
        double time_at_0 = bezier_utils::evaluate_bezier_time_component(p0, p1, p2, p3, 0.0);
        REQUIRE(time_at_0 == Catch::Approx(p0.time));
        
        double time_at_1 = bezier_utils::evaluate_bezier_time_component(p0, p1, p2, p3, 1.0);
        REQUIRE(time_at_1 == Catch::Approx(p3.time));
        
        double time_at_half = bezier_utils::evaluate_bezier_time_component(p0, p1, p2, p3, 0.5);
        // Should be between p0.time and p3.time
        REQUIRE(time_at_half >= p0.time);
        REQUIRE(time_at_half <= p3.time);
    }
}

TEST_CASE("Time derivative evaluation", "[bezier_utils]") {
    SECTION("Evaluate time derivative") {
        Point p0(0.0, 0.0);
        Point p1(1.0, 1.0);
        Point p2(2.0, 2.0);
        Point p3(3.0, 3.0);
        
        double derivative_at_0 = bezier_utils::evaluate_bezier_time_derivative(p0, p1, p2, p3, 0.0);
        double derivative_at_half = bezier_utils::evaluate_bezier_time_derivative(p0, p1, p2, p3, 0.5);
        double derivative_at_1 = bezier_utils::evaluate_bezier_time_derivative(p0, p1, p2, p3, 1.0);
        
        // For a monotonic time progression, derivatives should be positive
        REQUIRE(derivative_at_0 > 0.0);
        REQUIRE(derivative_at_half > 0.0);
        REQUIRE(derivative_at_1 > 0.0);
    }
    
    SECTION("Derivative for constant time") {
        Point p_const(1.0, 0.0);
        Point p1(1.0, 1.0);
        Point p2(1.0, 2.0);
        Point p3_const(1.0, 3.0);
        
        double derivative = bezier_utils::evaluate_bezier_time_derivative(p_const, p1, p2, p3_const, 0.5);
        REQUIRE(derivative == Catch::Approx(0.0));
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
        
        REQUIRE(in_handle.time == Catch::Approx(kf.time - time_offset)); // kf.time + 0.5
        REQUIRE(in_handle.value == Catch::Approx(kf.value));
        REQUIRE(out_handle.time == Catch::Approx(kf.time + time_offset)); // kf.time - 0.5
        REQUIRE(out_handle.value == Catch::Approx(kf.value));

        // Verify relative positions
        REQUIRE(in_handle.time > kf.time); // In handle is to the right
        REQUIRE(out_handle.time < kf.time); // Out handle is to the left
    }
}

TEST_CASE("Bezier time solving edge cases", "[bezier_utils]") {
    SECTION("solve_t_for_time with fallback to bisection") {
        // Create a curve that might cause Newton-Raphson to fail
        Point p0(0.0, 0.0);
        Point p1(0.1, 10.0);  // Sharp curve up
        Point p2(3.9, -5.0);  // Sharp curve down
        Point p3(4.0, 5.0);
        
        // Test various target times
        std::vector<double> target_times = {0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5};
        
        for (double target_time : target_times) {
            if (target_time >= p0.time && target_time <= p3.time) {
                double t = bezier_utils::solve_t_for_time(p0, p1, p2, p3, target_time);
                REQUIRE(t >= 0.0);
                REQUIRE(t <= 1.0);
                
                // Verify the solution
                Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
                REQUIRE(result.time == Catch::Approx(target_time).margin(0.01));
            }
        }
    }
    
    SECTION("solve_t_for_time_bisection with valid bounds") {
        Point p0(0.0, 0.0);
        Point p1(1.0, 1.0);
        Point p2(2.0, 2.0);
        Point p3(3.0, 3.0);
        
        // Test with explicit bounds
        double t = bezier_utils::solve_t_for_time_bisection(p0, p1, p2, p3, 1.5, 0.0, 1.0);
        REQUIRE(t >= 0.0);
        REQUIRE(t <= 1.0);
        
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
        REQUIRE(result.time == Catch::Approx(1.5).margin(0.01));
    }
    
    SECTION("solve_t_for_time_bisection with invalid initial bounds should clamp") {
        Point p0(0.0, 0.0);
        Point p1(1.0, 1.0);
        Point p2(2.0, 2.0);
        Point p3(3.0, 3.0);
        
        // This should not throw even with invalid bounds - function should handle it
        REQUIRE_NOTHROW(bezier_utils::solve_t_for_time_bisection(p0, p1, p2, p3, 1.5, -0.5, 1.5));
    }
    
    SECTION("Complex curve that might trigger bisection fallback") {
        // Create a curve where the time component has an inflection point
        Point p0(0.0, 0.0);
        Point p1(2.0, 10.0);   // Handle goes forward in time
        Point p2(1.0, 15.0);   // Handle goes backward in time (creates time loop)
        Point p3(4.0, 20.0);
        
        // Even with this complex curve, solving should work
        double target_time = 2.0;
        if (target_time >= p0.time && target_time <= p3.time) {
            double t = bezier_utils::solve_t_for_time(p0, p1, p2, p3, target_time);
            REQUIRE(t >= 0.0);
            REQUIRE(t <= 1.0);
        }
    }

    SECTION("Newton-Raphson fallback to bisection when derivative is zero") {
        // Create a curve where the time derivative becomes zero, forcing fallback
        Point p0(0.0, 0.0);
        Point p1(1.0, 5.0);   // Creates a curve where derivative might be zero
        Point p2(1.0, 10.0);  // Same time as p1, creates zero derivative region
        Point p3(2.0, 15.0);
        
        // Test various target times that might trigger the fallback
        std::vector<double> target_times = {0.5, 1.0, 1.5};
        
        for (double target_time : target_times) {
            if (target_time >= p0.time && target_time <= p3.time) {
                // This should not throw, even when falling back to bisection
                REQUIRE_NOTHROW(bezier_utils::solve_t_for_time(p0, p1, p2, p3, target_time));
                
                double t = bezier_utils::solve_t_for_time(p0, p1, p2, p3, target_time);
                REQUIRE(t >= 0.0);
                REQUIRE(t <= 1.0);
                
                // Verify the solution is reasonably accurate
                Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
                REQUIRE(result.time == Catch::Approx(target_time).margin(0.05));
            }
        }
    }
    
    SECTION("Newton-Raphson fallback to bisection after max iterations") {
        // Create a curve that might be hard for Newton-Raphson to converge on
        Point p0(0.0, 0.0);
        Point p1(0.001, 100.0);  // Very steep initial slope
        Point p2(1.999, -50.0);  // Very steep final slope
        Point p3(2.0, 10.0);
        
        // Use a very low max_iterations to force fallback
        double target_time = 1.0;
        double t = bezier_utils::solve_t_for_time(p0, p1, p2, p3, target_time, nullptr, 1e-6, 2); // Only 2 iterations
        
        REQUIRE(t >= 0.0);
        REQUIRE(t <= 1.0);
        
        // Even with forced fallback, should get reasonable result
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
        REQUIRE(result.time == Catch::Approx(target_time).margin(0.1));
    }
    
    SECTION("Bisection fallback with invalid initial guess") {
        Point p0(0.0, 0.0);
        Point p1(1.0, 1.0);
        Point p2(2.0, 2.0);
        Point p3(3.0, 3.0);
        
        // Provide an invalid initial guess that would cause bisection to get invalid bounds
        double invalid_guess = -0.5; // Outside [0,1] range
        double target_time = 1.5;
        
        // This should not throw even with invalid initial guess
        REQUIRE_NOTHROW(bezier_utils::solve_t_for_time(p0, p1, p2, p3, target_time, &invalid_guess));
        
        double t = bezier_utils::solve_t_for_time(p0, p1, p2, p3, target_time, &invalid_guess);
        REQUIRE(t >= 0.0);
        REQUIRE(t <= 1.0);
        
        Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
        REQUIRE(result.time == Catch::Approx(target_time).margin(0.01));
    }
    
    SECTION("Direct test of problematic curve that triggers bisection bounds issue") {
        // Create a curve similar to what might be generated in the Python test
        Point p0(0.0, 0.0);
        Point p1(0.33, 3.33);   // Approximates smooth handles
        Point p2(0.67, 6.67);   // Approximates smooth handles  
        Point p3(1.0, 10.0);
        
        // Test with many sample points like evaluate_range_by_rate would do
        for (int i = 0; i <= 100; ++i) {
            double target_time = 0.0 + (1.0 * i) / 100.0;
            
            REQUIRE_NOTHROW(bezier_utils::solve_t_for_time(p0, p1, p2, p3, target_time));
            
            double t = bezier_utils::solve_t_for_time(p0, p1, p2, p3, target_time);
            REQUIRE(t >= 0.0);
            REQUIRE(t <= 1.0);
            
            Point result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t);
            REQUIRE(result.time == Catch::Approx(target_time).margin(0.01));
        }
    }
}

TEST_CASE("Bezier endpoint and robustness edge cases", "[bezier_utils]") {
    SECTION("evaluate_cubic_bezier returns exact endpoints at t=0 and t=1") {
        Point p0(1.0, 2.0), p1(2.0, 5.0), p2(4.0, -1.0), p3(5.0, 3.0);
        Point at0 = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, 0.0);
        Point at1 = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, 1.0);
        REQUIRE(at0.time == p0.time);
        REQUIRE(at0.value == p0.value);
        REQUIRE(at1.time == p3.time);
        REQUIRE(at1.value == p3.value);
    }

    SECTION("solve_t_for_time clamps targets outside the segment") {
        Point p0(1.0, 0.0), p1(2.0, 1.0), p2(3.0, 2.0), p3(4.0, 3.0);
        REQUIRE(bezier_utils::solve_t_for_time(p0, p1, p2, p3, 0.0) == 0.0);  // <= p0.time
        REQUIRE(bezier_utils::solve_t_for_time(p0, p1, p2, p3, 9.0) == 1.0);  // >= p3.time
    }

    SECTION("solve_t_for_time stays in [0,1] for a non-monotonic time curve") {
        // Endpoints advance in time, but the interior control points run time
        // backwards (P1.time > P2.time), making the time curve non-monotonic.
        // The solver must still return a usable parameter, never NaN/out-of-range.
        Point p0(0.0, 0.0), p1(3.0, 1.0), p2(-1.0, 2.0), p3(2.0, 3.0);
        double t = bezier_utils::solve_t_for_time(p0, p1, p2, p3, 1.0);
        REQUIRE(t >= 0.0); // NaN would fail this
        REQUIRE(t <= 1.0);
    }
}