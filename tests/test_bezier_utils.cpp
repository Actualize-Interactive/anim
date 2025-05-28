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
}
