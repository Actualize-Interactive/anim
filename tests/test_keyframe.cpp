#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/keyframe.h>
#include <algorithm> // Added for std::sort

using namespace anim;

TEST_CASE("Keyframe constructor and accessors", "[keyframe]") {
    BezierHandle in_tangent(1.5, 2.5);
    BezierHandle out_tangent(2.5, 3.5);
    TangentMode mode = TangentMode::smoothAuto;
    
    Keyframe kf(2.0, 3.0, in_tangent, out_tangent, mode);
    
    SECTION("Getter methods") {
        REQUIRE(kf.time() == Catch::Approx(2.0));
        REQUIRE(kf.value() == Catch::Approx(3.0));
        REQUIRE(kf.in_tangent().time == Catch::Approx(1.5));
        REQUIRE(kf.in_tangent().value == Catch::Approx(2.5));
        REQUIRE(kf.out_tangent().time == Catch::Approx(2.5));
        REQUIRE(kf.out_tangent().value == Catch::Approx(3.5));
        REQUIRE(kf.mode() == TangentMode::smoothAuto);
    }
}

TEST_CASE("Keyframe comparison operators", "[keyframe]") {
    BezierHandle in_tangent1(1.5, 2.5);
    BezierHandle out_tangent1(2.5, 3.5);
    TangentMode mode1 = TangentMode::smoothAuto;
    
    Keyframe kf1(2.0, 3.0, in_tangent1, out_tangent1, mode1);
    Keyframe kf2(2.0, 3.0, in_tangent1, out_tangent1, mode1);
    
    SECTION("Equality") {
        REQUIRE(kf1 == kf2);
    }
    
    SECTION("Inequality - different time") {
        Keyframe kf3(2.1, 3.0, in_tangent1, out_tangent1, mode1);
        REQUIRE(kf1 != kf3);
    }
    
    SECTION("Inequality - different value") {
        Keyframe kf3(2.0, 3.1, in_tangent1, out_tangent1, mode1);
        REQUIRE(kf1 != kf3);
    }
    
    SECTION("Inequality - different in_tangent") {
        BezierHandle in_tangent2(1.6, 2.5);
        Keyframe kf3(2.0, 3.0, in_tangent2, out_tangent1, mode1);
        REQUIRE(kf1 != kf3);
    }
    
    SECTION("Inequality - different out_tangent") {
        BezierHandle out_tangent2(2.6, 3.5);
        Keyframe kf3(2.0, 3.0, in_tangent1, out_tangent2, mode1);
        REQUIRE(kf1 != kf3);
    }
    
    SECTION("Inequality - different mode") {
        Keyframe kf3(2.0, 3.0, in_tangent1, out_tangent1, TangentMode::flat);
        REQUIRE(kf1 != kf3);
    }
}

TEST_CASE("Keyframe comparison for sorting", "[keyframe]") {
    BezierHandle in_tangent(1.5, 2.5);
    BezierHandle out_tangent(2.5, 3.5);
    TangentMode mode = TangentMode::smoothAuto;
    
    Keyframe kf1(1.0, 3.0, in_tangent, out_tangent, mode);
    Keyframe kf2(2.0, 3.0, in_tangent, out_tangent, mode);
    
    SECTION("Less than operator") {
        REQUIRE(kf1 < kf2);
        REQUIRE_FALSE(kf2 < kf1);
    }
    
    SECTION("Time-based sorting") {
        std::vector<Keyframe> keyframes = {kf2, kf1};
        std::sort(keyframes.begin(), keyframes.end());
        REQUIRE(keyframes[0].time() == Catch::Approx(1.0));
        REQUIRE(keyframes[1].time() == Catch::Approx(2.0));
    }
}
