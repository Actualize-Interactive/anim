#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/animation_channel.h>

using namespace anim;

TEST_CASE("AnimationChannel empty state", "[animation_channel]") {
    AnimationChannel channel;
    
    SECTION("Channel is empty") {
        REQUIRE(channel.is_empty());
    }
    
    SECTION("No start or end time") {
        REQUIRE_FALSE(channel.get_start_time().has_value());
        REQUIRE_FALSE(channel.get_end_time().has_value());
    }
    
    SECTION("Evaluating empty channel returns 0") {
        REQUIRE(channel.evaluate(0.0) == Catch::Approx(0.0));
        REQUIRE(channel.evaluate(1.0) == Catch::Approx(0.0));
    }
    
    SECTION("Range evaluation with no keyframes") {
        std::vector<double> values = channel.evaluate_range(0.0, 1.0, 5);
        REQUIRE(values.size() == 5);
        for (double v : values) {
            REQUIRE(v == Catch::Approx(0.0));
        }
    }
}

TEST_CASE("AnimationChannel with a single keyframe", "[animation_channel]") {
    AnimationChannel channel;
      // Create a single keyframe
    BezierHandle in_tangent(1.5, 3.0);
    BezierHandle out_tangent(2.5, 3.0);
    TangentMode mode = TangentMode::flat;
    
    channel.set_keyframe(2.0, 3.0, in_tangent, out_tangent, mode);
    
    SECTION("Channel is not empty") {
        REQUIRE_FALSE(channel.is_empty());
    }
    
    SECTION("Start and end time match the keyframe") {
        REQUIRE(channel.get_start_time().value() == Catch::Approx(2.0));
        REQUIRE(channel.get_end_time().value() == Catch::Approx(2.0));
    }
    
    SECTION("Can retrieve the keyframe") {
        auto kf_opt = channel.get_keyframe(2.0);
        REQUIRE(kf_opt.has_value());
        REQUIRE(kf_opt->time() == Catch::Approx(2.0));
        REQUIRE(kf_opt->value() == Catch::Approx(3.0));
    }
    
    SECTION("Evaluation returns keyframe value") {
        REQUIRE(channel.evaluate(0.0) == Catch::Approx(3.0)); // Before keyframe
        REQUIRE(channel.evaluate(2.0) == Catch::Approx(3.0)); // At keyframe
        REQUIRE(channel.evaluate(5.0) == Catch::Approx(3.0)); // After keyframe
    }
}

TEST_CASE("AnimationChannel with multiple keyframes", "[animation_channel]") {
    AnimationChannel channel;
      // Add keyframes in non-sequential order to test sorting
    BezierHandle in_tangent(2.5, 5.0);
    BezierHandle out_tangent(3.5, 5.0);
    channel.set_keyframe(3.0, 5.0, in_tangent, out_tangent, TangentMode::flat);
      BezierHandle in_tangent2(0.5, 2.0);
    BezierHandle out_tangent2(1.5, 2.0);
    channel.set_keyframe(1.0, 2.0, in_tangent2, out_tangent2, TangentMode::flat);
    
    SECTION("Keyframes are sorted by time") {
        const auto& keyframes = channel.get_all_keyframes();
        REQUIRE(keyframes.size() == 2);
        REQUIRE(keyframes[0].time() == Catch::Approx(1.0));
        REQUIRE(keyframes[1].time() == Catch::Approx(3.0));
    }
    
    SECTION("Start and end times match the first and last keyframes") {
        REQUIRE(channel.get_start_time().value() == Catch::Approx(1.0));
        REQUIRE(channel.get_end_time().value() == Catch::Approx(3.0));
    }
    
    SECTION("Evaluation outside keyframe range") {
        REQUIRE(channel.evaluate(0.0) == Catch::Approx(2.0)); // Before first keyframe
        REQUIRE(channel.evaluate(4.0) == Catch::Approx(5.0)); // After last keyframe
    }
}

TEST_CASE("AnimationChannel tangent modes", "[animation_channel]") {
    AnimationChannel channel;
      SECTION("LINEAR mode evaluation") {
        // Set up two keyframes with LINEAR mode
        channel.set_keyframe(1.0, 2.0, Point2D(0.9, 2.0), Point2D(1.1, 2.0), TangentMode::linear);
        channel.set_keyframe(3.0, 6.0, Point2D(2.9, 6.0), Point2D(3.1, 6.0), TangentMode::linear);
        
        // Evaluate at points between keyframes
        REQUIRE(channel.evaluate(1.0) == Catch::Approx(2.0));
        REQUIRE(channel.evaluate(2.0) == Catch::Approx(4.0)); // Halfway between 2.0 and 6.0
        REQUIRE(channel.evaluate(3.0) == Catch::Approx(6.0));
    }
      SECTION("STEPPED mode evaluation") {
        // Set up two keyframes with STEPPED mode
        channel.set_keyframe(1.0, 2.0, Point2D(0.9, 2.0), Point2D(1.1, 2.0), TangentMode::stepped);
        channel.set_keyframe(3.0, 6.0, Point2D(2.9, 6.0), Point2D(3.1, 6.0), TangentMode::stepped);
        
        // Evaluate at points between keyframes
        REQUIRE(channel.evaluate(1.0) == Catch::Approx(2.0));
        REQUIRE(channel.evaluate(2.0) == Catch::Approx(2.0)); // Should stay at first keyframe value
        REQUIRE(channel.evaluate(2.99) == Catch::Approx(2.0)); // Just before second keyframe
        REQUIRE(channel.evaluate(3.0) == Catch::Approx(6.0)); // At second keyframe
    }
      SECTION("SMOOTH_AUTO mode evaluation") {
        // Set up keyframes with SMOOTH_AUTO mode
        channel.set_keyframe(1.0, 2.0, Point2D(0.9, 2.0), Point2D(1.1, 2.0), TangentMode::smoothAuto);
        channel.set_keyframe(3.0, 6.0, Point2D(2.9, 6.0), Point2D(3.1, 6.0), TangentMode::smoothAuto);
        
        // Evaluate at each keyframe and between
        REQUIRE(channel.evaluate(1.0) == Catch::Approx(2.0));
        REQUIRE(channel.evaluate(3.0) == Catch::Approx(6.0));
        
        // Intermediate value should be smooth but the exact value depends on the auto-tangent calculation
        double mid_value = channel.evaluate(2.0);
        REQUIRE(mid_value >= 2.0); // Value should be monotonically increasing
        REQUIRE(mid_value <= 6.0);
    }
}

TEST_CASE("AnimationChannel keyframe manipulation", "[animation_channel]") {
    AnimationChannel channel;
      // Add initial keyframes
    channel.set_keyframe(1.0, 2.0, Point2D(0.9, 2.0), Point2D(1.1, 2.0), TangentMode::flat);
    channel.set_keyframe(3.0, 6.0, Point2D(2.9, 6.0), Point2D(3.1, 6.0), TangentMode::flat);
    
    SECTION("Changing keyframe time") {
        channel.set_keyframe_time(1.0, 1.5);
        
        REQUIRE_FALSE(channel.get_keyframe(1.0).has_value());
        REQUIRE(channel.get_keyframe(1.5).has_value());
        REQUIRE(channel.get_keyframe(1.5)->value() == Catch::Approx(2.0));
    }
    
    SECTION("Changing keyframe value") {
        channel.set_keyframe_value(1.0, 2.5);
        
        REQUIRE(channel.get_keyframe(1.0)->value() == Catch::Approx(2.5));
        
        // Tangents should be adjusted
        REQUIRE(channel.get_keyframe(1.0)->in_tangent().value == Catch::Approx(2.5));
        REQUIRE(channel.get_keyframe(1.0)->out_tangent().value == Catch::Approx(2.5));
    }
      SECTION("Changing keyframe tangent mode") {
        channel.set_keyframe_tangent_mode(1.0, TangentMode::linear);
        
        REQUIRE(channel.get_keyframe(1.0)->mode() == TangentMode::linear);
        
        // Tangents should be recalculated
        const Keyframe& kf = *channel.get_keyframe(1.0);
        REQUIRE(kf.out_tangent().time > kf.time()); // Out tangent should point toward next keyframe
    }
    
    SECTION("Removing a keyframe") {
        REQUIRE(channel.remove_keyframe(1.0));
        REQUIRE_FALSE(channel.get_keyframe(1.0).has_value());
        REQUIRE(channel.get_all_keyframes().size() == 1);
    }
    
    SECTION("Removing a nonexistent keyframe") {
        REQUIRE_FALSE(channel.remove_keyframe(2.0));
        REQUIRE(channel.get_all_keyframes().size() == 2);
    }
}

TEST_CASE("AnimationChannel evaluation range", "[animation_channel]") {
    AnimationChannel channel;
      // Add keyframes for a linear interpolation
    channel.set_keyframe(0.0, 0.0, Point2D(-0.1, 0.0), Point2D(0.1, 0.0), TangentMode::linear);
    channel.set_keyframe(10.0, 10.0, Point2D(9.9, 10.0), Point2D(10.1, 10.0), TangentMode::linear);
    
    SECTION("evaluate_range with set number of samples") {
        std::vector<double> values = channel.evaluate_range(0.0, 10.0, 11);
        
        REQUIRE(values.size() == 11);
        for (int i = 0; i < 11; ++i) {
            REQUIRE(values[i] == Catch::Approx(i));
        }
    }
    
    SECTION("evaluate_range_by_rate") {
        // Sample every 2.5 units, should give 5 samples
        std::vector<double> values = channel.evaluate_range_by_rate(0.0, 10.0, 0.4);
        
        REQUIRE(values.size() == 5);
        REQUIRE(values[0] == Catch::Approx(0.0));
        REQUIRE(values[1] == Catch::Approx(2.5));
        REQUIRE(values[2] == Catch::Approx(5.0));
        REQUIRE(values[3] == Catch::Approx(7.5));
        REQUIRE(values[4] == Catch::Approx(10.0));
    }
    
    SECTION("Edge cases for evaluate_range") {
        // Zero samples
        REQUIRE(channel.evaluate_range(0.0, 10.0, 0).empty());
        
        // One sample
        std::vector<double> values = channel.evaluate_range(0.0, 10.0, 1);
        REQUIRE(values.size() == 1);
        REQUIRE(values[0] == Catch::Approx(0.0));
    }
}
