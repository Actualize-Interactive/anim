#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/keyframe.hpp>

using namespace anim;

TEST_CASE("Keyframe construction with Point objects", "[keyframe]") {
    Point pos(1.0, 2.0);
    Point in_handle(0.5, 1.5);
    Point out_handle(1.5, 2.5);
    
    SECTION("Full constructor with all parameters") {
        Keyframe kf(pos, Function::linear, HandleMode::free, in_handle, out_handle);
        
        REQUIRE(kf.position == pos);
        REQUIRE(kf.in_handle == in_handle);
        REQUIRE(kf.out_handle == out_handle);
        REQUIRE(kf.function == Function::linear);
        REQUIRE(kf.handle_mode == HandleMode::free);
    }
    
    SECTION("Constructor with default function and handle mode") {
        Keyframe kf(pos);
        
        REQUIRE(kf.position == pos);
        REQUIRE(kf.in_handle.is_zero());
        REQUIRE(kf.out_handle.is_zero());
        REQUIRE(kf.function == Function::bezier);
        REQUIRE(kf.handle_mode == HandleMode::smooth);
    }
}

TEST_CASE("Keyframe construction with time/value and Point handles", "[keyframe]") {
    Point in_handle(0.5, 1.5);
    Point out_handle(1.5, 2.5);
    
    SECTION("Full constructor") {
        Keyframe kf(1.0, 2.0, Function::constant, HandleMode::aligned, in_handle, out_handle);
        
        REQUIRE(kf.position.time == 1.0);
        REQUIRE(kf.position.value == 2.0);
        REQUIRE(kf.in_handle == in_handle);
        REQUIRE(kf.out_handle == out_handle);
        REQUIRE(kf.function == Function::constant);
        REQUIRE(kf.handle_mode == HandleMode::aligned);
    }
    
    SECTION("Constructor with default handles and types") {
        Keyframe kf(1.0, 2.0);
        
        REQUIRE(kf.position.time == 1.0);
        REQUIRE(kf.position.value == 2.0);
        REQUIRE(kf.in_handle.is_zero());
        REQUIRE(kf.out_handle.is_zero());
        REQUIRE(kf.function == Function::bezier);
        REQUIRE(kf.handle_mode == HandleMode::smooth);
    }
}

TEST_CASE("Keyframe construction with simplified constructor", "[keyframe]") {
    SECTION("Constructor with function type only") {
        Keyframe kf(1.0, 2.0, Function::linear);
        
        REQUIRE(kf.position.time == 1.0);
        REQUIRE(kf.position.value == 2.0);
        REQUIRE(kf.in_handle.is_zero());
        REQUIRE(kf.out_handle.is_zero());
        REQUIRE(kf.function == Function::linear);
        REQUIRE(kf.handle_mode == HandleMode::smooth);
    }
    
    SECTION("Default constructor") {
        Keyframe kf;
        
        REQUIRE(kf.position.is_zero());
        REQUIRE(kf.in_handle.is_zero());
        REQUIRE(kf.out_handle.is_zero());
        REQUIRE(kf.function == Function::bezier);
        REQUIRE(kf.handle_mode == HandleMode::smooth);
    }
}

TEST_CASE("Keyframe comparison operators", "[keyframe]") {
    Point pos(1.0, 2.0);
    Point in_handle(0.5, 1.5);
    Point out_handle(1.5, 2.5);
    
    Keyframe kf1(pos, Function::linear, HandleMode::free, in_handle, out_handle);
    Keyframe kf2(pos, Function::linear, HandleMode::free, in_handle, out_handle);
    Keyframe kf3(Point(2.0, 3.0), Function::bezier, HandleMode::smooth, Point(1.0, 2.0), Point(2.0, 3.0));
    
    SECTION("Equality") {
        REQUIRE(kf1 == kf2);
        REQUIRE_FALSE(kf1 == kf3);
    }
    
    SECTION("Inequality") {
        REQUIRE(kf1 != kf3);
        REQUIRE_FALSE(kf1 != kf2);
    }
    
    SECTION("Different functions are not equal") {
        Keyframe kf_diff_func(pos, Function::constant, HandleMode::free, in_handle, out_handle);
        REQUIRE(kf1 != kf_diff_func);
    }
    
    SECTION("Different handle modes are not equal") {
        Keyframe kf_diff_handle(pos, Function::linear, HandleMode::aligned, in_handle, out_handle);
        REQUIRE(kf1 != kf_diff_handle);
    }
}

TEST_CASE("Keyframe copy and move semantics", "[keyframe]") {
    Point pos(1.0, 2.0);
    Point in_handle(0.5, 1.5);
    Point out_handle(1.5, 2.5);
    Keyframe original(pos, Function::linear, HandleMode::free, in_handle, out_handle);
    
    SECTION("Copy constructor") {
        Keyframe copy(original);
        REQUIRE(copy == original);
        REQUIRE(copy.position == original.position);
        REQUIRE(copy.function == original.function);
        REQUIRE(copy.handle_mode == original.handle_mode);
    }
    
    SECTION("Copy assignment") {
        Keyframe copy;
        copy = original;
        REQUIRE(copy == original);
    }
    
    SECTION("Move constructor") {
        Keyframe original_copy = original;
        Keyframe moved(std::move(original_copy));
        REQUIRE(moved == original);
    }
    
    SECTION("Move assignment") {
        Keyframe original_copy = original;
        Keyframe moved;
        moved = std::move(original_copy);
        REQUIRE(moved == original);
    }
}

TEST_CASE("Keyframe accessor methods", "[keyframe]") {
    Keyframe kf(5.5, 10.25);
    
    SECTION("time() method") {
        REQUIRE(kf.time() == 5.5);
        REQUIRE(kf.time() == kf.position.time);
    }
    
    SECTION("value() method") {
        REQUIRE(kf.value() == 10.25);
        REQUIRE(kf.value() == kf.position.value);
    }
}

TEST_CASE("Keyframe with different enum values", "[keyframe]") {
    SECTION("All Function values") {
        Point pos(1.0, 1.0);
        Point handle;
        
        Keyframe constant_kf(pos, Function::constant);
        Keyframe linear_kf(pos, Function::linear);
        Keyframe bezier_kf(pos, Function::bezier);
        
        REQUIRE(constant_kf.function == Function::constant);
        REQUIRE(linear_kf.function == Function::linear);
        REQUIRE(bezier_kf.function == Function::bezier);
        
        REQUIRE(constant_kf != linear_kf);
        REQUIRE(linear_kf != bezier_kf);
        REQUIRE(bezier_kf != constant_kf);
    }
    
    SECTION("All HandleMode values") {
        Point pos(1.0, 1.0);
        Point handle;
        
        Keyframe flat_kf(pos, Function::bezier, HandleMode::flat);
        Keyframe smooth_kf(pos, Function::bezier, HandleMode::smooth);
        Keyframe aligned_kf(pos, Function::bezier, HandleMode::aligned);
        Keyframe free_kf(pos, Function::bezier, HandleMode::free);
        
        REQUIRE(flat_kf.handle_mode == HandleMode::flat);
        REQUIRE(smooth_kf.handle_mode == HandleMode::smooth);
        REQUIRE(aligned_kf.handle_mode == HandleMode::aligned);
        REQUIRE(free_kf.handle_mode == HandleMode::free);
        
        REQUIRE(flat_kf != smooth_kf);
        REQUIRE(smooth_kf != aligned_kf);
        REQUIRE(aligned_kf != free_kf);
        REQUIRE(free_kf != flat_kf);
    }
}
