#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/bezier_handle.hpp>

using namespace anim;

TEST_CASE("BezierHandle constructors and accessors", "[bezier_handle]") {
    SECTION("Default constructor") {
        BezierHandle p;
        REQUIRE(p.time == Catch::Approx(0.0));
        REQUIRE(p.value == Catch::Approx(0.0));
    }
    
    SECTION("Parameterized constructor") {
        BezierHandle p(2.5, 3.7);
        REQUIRE(p.time == Catch::Approx(2.5));
        REQUIRE(p.value == Catch::Approx(3.7));
    }
}

TEST_CASE("BezierHandle arithmetic operations", "[bezier_handle]") {
    BezierHandle p1(1.0, 2.0);
    BezierHandle p2(3.0, 4.0);
    
    SECTION("Addition") {
        BezierHandle result = p1 + p2;
        REQUIRE(result.time == Catch::Approx(4.0));
        REQUIRE(result.value == Catch::Approx(6.0));
    }
    
    SECTION("Subtraction") {
        BezierHandle result = p2 - p1;
        REQUIRE(result.time == Catch::Approx(2.0));
        REQUIRE(result.value == Catch::Approx(2.0));
    }
    
    SECTION("Multiplication by scalar") {
        BezierHandle result = p1 * 2.5;
        REQUIRE(result.time == Catch::Approx(2.5));
        REQUIRE(result.value == Catch::Approx(5.0));
    }
    
    SECTION("Division by scalar") {
        BezierHandle result = p2 / 2.0;
        REQUIRE(result.time == Catch::Approx(1.5));
        REQUIRE(result.value == Catch::Approx(2.0));
    }
    
    SECTION("Division by zero") {
        REQUIRE_THROWS_AS(p1 / 0.0, std::domain_error);
    }
}

TEST_CASE("BezierHandle comparison operators", "[bezier_handle]") {
    BezierHandle p1(1.0, 2.0);
    BezierHandle p2(1.0, 2.0);
    BezierHandle p3(1.0, 3.0);
    BezierHandle p4(2.0, 2.0);
    
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
}

TEST_CASE("BezierHandle vector operations", "[bezier_handle]") {
    BezierHandle p(3.0, 4.0);
    
    SECTION("Length") {
        REQUIRE(p.length() == Catch::Approx(5.0));
    }
    
    SECTION("Normalization") {
        BezierHandle n = p.normalized();
        REQUIRE(n.length() == Catch::Approx(1.0).margin(1e-10));
        REQUIRE(n.time == Catch::Approx(0.6));
        REQUIRE(n.value == Catch::Approx(0.8));
    }
    
    SECTION("Normalizing zero vector") {
        BezierHandle zero;
        REQUIRE_THROWS_AS(zero.normalized(), std::domain_error);
    }
}
