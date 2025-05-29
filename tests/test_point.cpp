#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/point.hpp>

using namespace anim;

TEST_CASE("Point construction", "[point]") {
    SECTION("Default constructor") {
        Point p;
        REQUIRE(p.time == 0.0);
        REQUIRE(p.value == 0.0);
    }
    
    SECTION("Parameterized constructor") {
        Point p(1.5, 2.5);
        REQUIRE(p.time == 1.5);
        REQUIRE(p.value == 2.5);
    }
}

TEST_CASE("Point arithmetic operators", "[point]") {
    Point p1(2.0, 3.0);
    Point p2(1.0, 1.5);
    
    SECTION("Addition") {
        Point result = p1 + p2;
        REQUIRE(result.time == 3.0);
        REQUIRE(result.value == 4.5);
    }
    
    SECTION("Subtraction") {
        Point result = p1 - p2;
        REQUIRE(result.time == 1.0);
        REQUIRE(result.value == 1.5);
    }
    
    SECTION("Scalar multiplication") {
        Point result = p1 * 2.0;
        REQUIRE(result.time == 4.0);
        REQUIRE(result.value == 6.0);
    }
    
    SECTION("Scalar division") {
        Point result = p1 / 2.0;
        REQUIRE(result.time == 1.0);
        REQUIRE(result.value == 1.5);
    }
    
    SECTION("Division by zero throws exception") {
        REQUIRE_THROWS_AS(p1 / 0.0, std::domain_error);
    }
}

TEST_CASE("Point comparison operators", "[point]") {
    Point p1(2.0, 3.0);
    Point p2(2.0, 3.0);
    Point p3(1.0, 2.0);
    
    SECTION("Equality") {
        REQUIRE(p1 == p2);
        REQUIRE_FALSE(p1 == p3);
    }
    
    SECTION("Inequality") {
        REQUIRE(p1 != p3);
        REQUIRE_FALSE(p1 != p2);
    }
}

TEST_CASE("Point compound assignment operators", "[point]") {
    SECTION("Addition assignment") {
        Point p1(2.0, 3.0);
        Point p2(1.0, 1.5);
        p1 += p2;
        REQUIRE(p1.time == 3.0);
        REQUIRE(p1.value == 4.5);
    }
    
    SECTION("Subtraction assignment") {
        Point p1(2.0, 3.0);
        Point p2(1.0, 1.5);
        p1 -= p2;
        REQUIRE(p1.time == 1.0);
        REQUIRE(p1.value == 1.5);
    }
    
    SECTION("Scalar multiplication assignment") {
        Point p(2.0, 3.0);
        p *= 2.0;
        REQUIRE(p.time == 4.0);
        REQUIRE(p.value == 6.0);
    }
    
    SECTION("Scalar division assignment") {
        Point p(4.0, 6.0);
        p /= 2.0;
        REQUIRE(p.time == 2.0);
        REQUIRE(p.value == 3.0);
    }
    
    SECTION("Division assignment by zero throws exception") {
        Point p(2.0, 3.0);
        REQUIRE_THROWS_AS(p /= 0.0, std::domain_error);
    }
}

TEST_CASE("Point utility methods", "[point]") {
    SECTION("is_zero method") {
        Point zero_point;
        Point non_zero_point(1.0, 2.0);
        
        REQUIRE(zero_point.is_zero());
        REQUIRE_FALSE(non_zero_point.is_zero());
    }
    
    SECTION("reset method") {
        Point p(5.0, 10.0);
        p.reset();
        REQUIRE(p.time == 0.0);
        REQUIRE(p.value == 0.0);
        REQUIRE(p.is_zero());
    }
    
    SECTION("set method") {
        Point p;
        p.set(3.5, 7.2);
        REQUIRE(p.time == 3.5);
        REQUIRE(p.value == 7.2);
    }
}

TEST_CASE("Vector alias", "[point]") {
    Vector v(1.0, 2.0);
    REQUIRE(v.time == 1.0);
    REQUIRE(v.value == 2.0);
    
    Vector v2 = v * 2.0;
    REQUIRE(v2.time == 2.0);
    REQUIRE(v2.value == 4.0);
}