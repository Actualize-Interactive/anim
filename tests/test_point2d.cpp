#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/point2d.h>

using namespace anim;

TEST_CASE("Point2D constructors and accessors", "[point2d]") {
    SECTION("Default constructor") {
        Point2D p;
        REQUIRE(p.time == Catch::Approx(0.0));
        REQUIRE(p.value == Catch::Approx(0.0));
    }
    
    SECTION("Parameterized constructor") {
        Point2D p(2.5, 3.7);
        REQUIRE(p.time == Catch::Approx(2.5));
        REQUIRE(p.value == Catch::Approx(3.7));
    }
}

TEST_CASE("Point2D arithmetic operations", "[point2d]") {
    Point2D p1(1.0, 2.0);
    Point2D p2(3.0, 4.0);
    
    SECTION("Addition") {
        Point2D result = p1 + p2;
        REQUIRE(result.time == Catch::Approx(4.0));
        REQUIRE(result.value == Catch::Approx(6.0));
    }
    
    SECTION("Subtraction") {
        Point2D result = p2 - p1;
        REQUIRE(result.time == Catch::Approx(2.0));
        REQUIRE(result.value == Catch::Approx(2.0));
    }
    
    SECTION("Multiplication by scalar") {
        Point2D result = p1 * 2.5;
        REQUIRE(result.time == Catch::Approx(2.5));
        REQUIRE(result.value == Catch::Approx(5.0));
    }
    
    SECTION("Division by scalar") {
        Point2D result = p2 / 2.0;
        REQUIRE(result.time == Catch::Approx(1.5));
        REQUIRE(result.value == Catch::Approx(2.0));
    }
    
    SECTION("Division by zero") {
        REQUIRE_THROWS_AS(p1 / 0.0, std::invalid_argument);
    }
}

TEST_CASE("Point2D comparison operators", "[point2d]") {
    Point2D p1(1.0, 2.0);
    Point2D p2(1.0, 2.0);
    Point2D p3(1.0, 3.0);
    Point2D p4(2.0, 2.0);
    
    SECTION("Equality") {
        REQUIRE(p1 == p2);
        REQUIRE_FALSE(p1 == p3);
        REQUIRE_FALSE(p1 == p4);
    }
    
    SECTION("Inequality") {
        REQUIRE_FALSE(p1 != p2);
        REQUIRE(p1 != p3);
        REQUIRE(p1 != p4);
    }
    
    SECTION("Equality with small differences") {
        Point2D p5(1.0 + 1e-11, 2.0 - 1e-11);
        REQUIRE(p1 == p5); // Should be equal within epsilon
        
        Point2D p6(1.0 + 1e-9, 2.0);
        REQUIRE_FALSE(p1 == p6); // Should not be equal (difference > epsilon)
    }
}

TEST_CASE("BezierHandle type alias", "[point2d]") {
    SECTION("BezierHandle is the same as Point2D") {
        BezierHandle h(1.0, 2.0);
        Point2D p(1.0, 2.0);
        REQUIRE(h.time == p.time);
        REQUIRE(h.value == p.value);
    }
}
