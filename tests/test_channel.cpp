#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/channel.hpp>

using namespace anim;

TEST_CASE("Channel empty state", "[channel]") {
    Channel channel;
    
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

TEST_CASE("Channel name", "[channel]") {
    SECTION("Default constructor sets empty name") {
        Channel channel;
        REQUIRE(channel.name().empty());
    }
    
    SECTION("Named constructor sets name") {
        Channel channel("test_channel");
        REQUIRE(channel.name() == "test_channel");
    }
    
    SECTION("Name can be changed") {
        Channel channel("original_name");
        REQUIRE(channel.name() == "original_name");
        
        channel.set_name("new_name");
        REQUIRE(channel.name() == "new_name");
    }
}

TEST_CASE("Channel with a single keyframe", "[channel]") {
    Channel channel;
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

TEST_CASE("Channel with multiple keyframes", "[channel]") {
    Channel channel("test");
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

TEST_CASE("Channel tangent modes", "[channel]") {
    Channel channel;
    SECTION("LINEAR mode evaluation") {
        // Set up two keyframes with LINEAR mode
        channel.set_keyframe(1.0, 2.0, Point2D(0.9, 2.0), Point2D(1.1, 2.0), TangentMode::linear);
        channel.set_keyframe(3.0, 6.0, Point2D(2.9, 6.0), Point2D(3.1, 6.0), TangentMode::linear);
        
        // Evaluate at points between keyframes
        REQUIRE(channel.evaluate(1.0) == Catch::Approx(2.0));
        REQUIRE(channel.evaluate(2.0) == Catch::Approx(4.0)); // Linear interpolation
        REQUIRE(channel.evaluate(3.0) == Catch::Approx(6.0));
    }    SECTION("FLAT mode evaluation") {
        // Set up two keyframes with FLAT mode
        channel.set_keyframe(1.0, 2.0, Point2D(0.9, 2.0), Point2D(1.1, 2.0), TangentMode::flat);
        channel.set_keyframe(3.0, 6.0, Point2D(2.9, 6.0), Point2D(3.1, 6.0), TangentMode::flat);
        
        // Check key frame values directly
        REQUIRE(channel.evaluate(1.0) == Catch::Approx(2.0));
        REQUIRE(channel.evaluate(3.0) == Catch::Approx(6.0));
        
        // Check that curve preserves monotonicity
        double value_at_1_5 = channel.evaluate(1.5);
        double value_at_2_0 = channel.evaluate(2.0);
        double value_at_2_5 = channel.evaluate(2.5);
        
        REQUIRE(value_at_1_5 >= 2.0);
        REQUIRE(value_at_1_5 <= value_at_2_0);
        REQUIRE(value_at_2_0 <= value_at_2_5);
        REQUIRE(value_at_2_5 <= 6.0);
    }
    
    SECTION("STEPPED mode evaluation") {
        // Set up two keyframes with STEPPED mode
        channel.set_keyframe(1.0, 2.0, Point2D(0.9, 2.0), Point2D(1.1, 2.0), TangentMode::stepped);
        channel.set_keyframe(3.0, 6.0, Point2D(2.9, 6.0), Point2D(3.1, 6.0), TangentMode::stepped);
        
        // Evaluate at points between keyframes
        REQUIRE(channel.evaluate(1.0) == Catch::Approx(2.0));        REQUIRE(channel.evaluate(2.0) == Catch::Approx(2.0)); // Should stay at first keyframe value
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

TEST_CASE("Channel keyframe modification", "[channel]") {
    Channel channel;
    channel.set_keyframe(2.0, 5.0, Point2D(1.8, 5.0), Point2D(2.2, 5.0), TangentMode::linear);
    
    SECTION("Modify keyframe time") {
        channel.set_keyframe_time(2.0, 3.0);
        
        auto keyframe = channel.get_keyframe(3.0);
        REQUIRE(keyframe.has_value());
        REQUIRE(keyframe->time() == Catch::Approx(3.0));
        REQUIRE(keyframe->value() == Catch::Approx(5.0));
        
        // Original keyframe should no longer exist
        REQUIRE_FALSE(channel.get_keyframe(2.0).has_value());
    }
    
    SECTION("Modify keyframe value") {
        channel.set_keyframe_value(2.0, 8.0);
        
        auto keyframe = channel.get_keyframe(2.0);
        REQUIRE(keyframe.has_value());
        REQUIRE(keyframe->time() == Catch::Approx(2.0));
        REQUIRE(keyframe->value() == Catch::Approx(8.0));
    }
    
    SECTION("Remove keyframe") {
        REQUIRE(channel.remove_keyframe(2.0));
        REQUIRE(channel.is_empty());
    }
}

TEST_CASE("Channel range evaluation", "[channel]") {
    Channel channel;
    channel.set_keyframe(1.0, 2.0, Point2D(0.9, 2.0), Point2D(1.1, 2.0), TangentMode::linear);
    channel.set_keyframe(5.0, 6.0, Point2D(4.9, 6.0), Point2D(5.1, 6.0), TangentMode::linear);
    
    SECTION("Fixed sample count") {
        std::vector<double> samples = channel.evaluate_range(1.0, 5.0, 5);
        REQUIRE(samples.size() == 5);
        REQUIRE(samples[0] == Catch::Approx(2.0));
        REQUIRE(samples[4] == Catch::Approx(6.0));
    }
    
    SECTION("Sample by rate") {
        std::vector<double> samples = channel.evaluate_range_by_rate(1.0, 5.0, 1.0);
        REQUIRE(samples.size() == 5);
        REQUIRE(samples[0] == Catch::Approx(2.0));
        REQUIRE(samples[4] == Catch::Approx(6.0));
    }
}
