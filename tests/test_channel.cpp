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
    BezierHandle in_handle(1.5, 3.0);
    BezierHandle out_handle(2.5, 3.0);
    TangentMode mode = TangentMode::flat;
    
    channel.set_keyframe_at_time(2.0, 3.0, in_handle, out_handle, mode);
    
    SECTION("Channel is not empty") {
        REQUIRE_FALSE(channel.is_empty());
    }
    
    SECTION("Start and end time match the keyframe") {
        REQUIRE(channel.get_start_time().value() == Catch::Approx(2.0));
        REQUIRE(channel.get_end_time().value() == Catch::Approx(2.0));
    }
    
    SECTION("Can retrieve the keyframe") {
        auto kf_opt = channel.get_keyframe_at_time(2.0);
        REQUIRE(kf_opt.has_value());
        REQUIRE(kf_opt->time() == Catch::Approx(2.0));
        REQUIRE(kf_opt->value() == Catch::Approx(3.0));
    }
    
    SECTION("Has correct keyframe count") {
        REQUIRE(channel.keyframe_count() == 1);
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
    BezierHandle in_handle(2.5, 5.0);
    BezierHandle out_handle(3.5, 5.0);
    channel.set_keyframe_at_time(3.0, 5.0, in_handle, out_handle, TangentMode::flat);
    BezierHandle in_handle2(0.5, 2.0);
    BezierHandle out_handle2(1.5, 2.0);
    channel.set_keyframe_at_time(1.0, 2.0, in_handle2, out_handle2, TangentMode::flat);
    
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
        channel.set_keyframe_at_time(1.0, 2.0, BezierHandle(0.9, 2.0), BezierHandle(1.1, 2.0), TangentMode::linear);
        channel.set_keyframe_at_time(3.0, 6.0, BezierHandle(2.9, 6.0), BezierHandle(3.1, 6.0), TangentMode::linear);
        
        // Evaluate at points between keyframes
        REQUIRE(channel.evaluate(1.0) == Catch::Approx(2.0));
        REQUIRE(channel.evaluate(2.0) == Catch::Approx(4.0)); // Linear interpolation
        REQUIRE(channel.evaluate(3.0) == Catch::Approx(6.0));
    }    SECTION("FLAT mode evaluation") {
        // Set up two keyframes with FLAT mode
        channel.set_keyframe_at_time(1.0, 2.0, BezierHandle(0.9, 2.0), BezierHandle(1.1, 2.0), TangentMode::flat);
        channel.set_keyframe_at_time(3.0, 6.0, BezierHandle(2.9, 6.0), BezierHandle(3.1, 6.0), TangentMode::flat);
        
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
        channel.set_keyframe_at_time(1.0, 2.0, BezierHandle(0.9, 2.0), BezierHandle(1.1, 2.0), TangentMode::stepped);
        channel.set_keyframe_at_time(3.0, 6.0, BezierHandle(2.9, 6.0), BezierHandle(3.1, 6.0), TangentMode::stepped);
        
        // Evaluate at points between keyframes
        REQUIRE(channel.evaluate(1.0) == Catch::Approx(2.0));        REQUIRE(channel.evaluate(2.0) == Catch::Approx(2.0)); // Should stay at first keyframe value
        REQUIRE(channel.evaluate(2.99) == Catch::Approx(2.0)); // Just before second keyframe
        REQUIRE(channel.evaluate(3.0) == Catch::Approx(6.0)); // At second keyframe
    }
    
    SECTION("SMOOTH_AUTO mode evaluation") {
        // Set up keyframes with SMOOTH_AUTO mode
        channel.set_keyframe_at_time(1.0, 2.0, BezierHandle(0.9, 2.0), BezierHandle(1.1, 2.0), TangentMode::smoothAuto);
        channel.set_keyframe_at_time(3.0, 6.0, BezierHandle(2.9, 6.0), BezierHandle(3.1, 6.0), TangentMode::smoothAuto);
        
        // Evaluate at each keyframe and between
        REQUIRE(channel.evaluate(1.0) == Catch::Approx(2.0));
        REQUIRE(channel.evaluate(3.0) == Catch::Approx(6.0));
        
        // Intermediate value should be smooth but the exact value depends on the auto-tangent calculation
        double mid_value = channel.evaluate(2.0);
        REQUIRE(mid_value >= 2.0); // Value should be monotonically increasing
        REQUIRE(mid_value <= 6.0);
    }
}

TEST_CASE("Channel keyframe indexing and removal", "[channel]") {
    Channel channel;
    
    // Add three keyframes
    channel.set_keyframe_at_time(1.0, 10.0, BezierHandle(0.9, 10.0), BezierHandle(1.1, 10.0), TangentMode::flat);
    channel.set_keyframe_at_time(2.0, 20.0, BezierHandle(1.9, 20.0), BezierHandle(2.1, 20.0), TangentMode::flat);
    channel.set_keyframe_at_time(3.0, 30.0, BezierHandle(2.9, 30.0), BezierHandle(3.1, 30.0), TangentMode::flat);
    
    SECTION("Keyframe count is correct") {
        REQUIRE(channel.keyframe_count() == 3);
    }
    
    SECTION("Checking keyframe existence by index") {
        REQUIRE(channel.has_keyframe(0));
        REQUIRE(channel.has_keyframe(1));
        REQUIRE(channel.has_keyframe(2));
        REQUIRE_FALSE(channel.has_keyframe(3));
    }
    
    SECTION("Remove keyframe by index") {
        REQUIRE(channel.has_keyframe(1));
        REQUIRE(channel.remove_keyframe(1));
        REQUIRE(channel.keyframe_count() == 2);
        
        // The middle keyframe should be gone, leaving keyframes at times 1.0 and 3.0
        const auto& keyframes = channel.get_all_keyframes();
        REQUIRE(keyframes[0].time() == Catch::Approx(1.0));
        REQUIRE(keyframes[1].time() == Catch::Approx(3.0));
    }
    
    SECTION("Remove keyframe by time") {
        REQUIRE(channel.has_keyframe_at_time(2.0));
        REQUIRE(channel.remove_keyframe_at_time(2.0));
        REQUIRE_FALSE(channel.has_keyframe_at_time(2.0));
        REQUIRE(channel.keyframe_count() == 2);
        
        // The middle keyframe should be gone, leaving keyframes at times 1.0 and 3.0
        const auto& keyframes = channel.get_all_keyframes();
        REQUIRE(keyframes[0].time() == Catch::Approx(1.0));
        REQUIRE(keyframes[1].time() == Catch::Approx(3.0));
    }
    
    SECTION("Removing non-existent keyframe returns false") {
        REQUIRE_FALSE(channel.remove_keyframe_at_time(4.0));
        REQUIRE_FALSE(channel.remove_keyframe(10)); // Index out of range
        REQUIRE(channel.keyframe_count() == 3); // Count unchanged
    }
    
    SECTION("Get keyframe by index") {
        Keyframe& kf = channel.get_keyframe(1);
        REQUIRE(kf.time() == Catch::Approx(2.0));
        REQUIRE(kf.value() == Catch::Approx(20.0));
        
        REQUIRE_THROWS_AS(channel.get_keyframe(5), std::out_of_range);
    }
}

TEST_CASE("Channel range evaluation", "[channel]") {
    Channel channel;
    channel.set_keyframe_at_time(1.0, 2.0, BezierHandle(0.9, 2.0), BezierHandle(1.1, 2.0), TangentMode::linear);
    channel.set_keyframe_at_time(5.0, 6.0, BezierHandle(4.9, 6.0), BezierHandle(5.1, 6.0), TangentMode::linear);
    
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

TEST_CASE("Channel bezier handle adjustments", "[channel]") {
    
    SECTION("FLAT mode handle adjustment") {
        Channel channel;
        // Set up keyframe with FLAT mode
        channel.set_keyframe_at_time(2.0, 3.0, 
                                   BezierHandle(1.8, 3.0), 
                                   BezierHandle(2.2, 3.0), 
                                   TangentMode::flat);
        
        // Get the keyframe to test
        Keyframe& kf = channel.get_keyframe(0);
        
        // Store original handle values
        BezierHandle orig_in_handle = kf.in_handle();
        BezierHandle orig_out_handle = kf.out_handle();
        
        // Try to adjust the in_handle time
        BezierHandle new_in_handle(1.5, 3.0);
        kf.set_in_handle(new_in_handle);
        
        // For flat mode, the time should be adjustable but the value should remain at keyframe value
        REQUIRE(kf.in_handle().time == Catch::Approx(1.5));
        REQUIRE(kf.in_handle().value == Catch::Approx(3.0)); // Value should remain at keyframe value
        
        // Try to adjust the out_handle time
        BezierHandle new_out_handle(2.5, 3.0);
        kf.set_out_handle(new_out_handle);
        
        // For flat mode, the time should be adjustable but the value should remain at keyframe value
        REQUIRE(kf.out_handle().time == Catch::Approx(2.5));
        REQUIRE(kf.out_handle().value == Catch::Approx(3.0)); // Value should remain at keyframe value
        
        // Try to adjust the value (should be reset to keyframe value)
        kf.set_in_handle(BezierHandle(1.5, 4.0));
        REQUIRE(kf.in_handle().time == Catch::Approx(1.5));
        REQUIRE(kf.in_handle().value == Catch::Approx(3.0)); // Value should remain at keyframe value
    }
    
    SECTION("SMOOTH_MANUAL mode handle adjustment") {
        Channel channel;
        // Set up keyframe with SMOOTH_MANUAL mode
        channel.set_keyframe_at_time(2.0, 3.0, 
                                   BezierHandle(1.7, 2.7), // In-handle
                                   BezierHandle(2.3, 3.3), // Out-handle
                                   TangentMode::smoothManual);
        
        // Get the keyframe to test
        Keyframe& kf = channel.get_keyframe(0);
        
        // Store original handle values
        BezierHandle orig_in_handle = kf.in_handle();
        BezierHandle orig_out_handle = kf.out_handle();
        
        // Adjust the in_handle
        BezierHandle new_in_handle(1.5, 2.5);
        kf.set_in_handle(new_in_handle);
        
        // In smoothManual mode, when adjusting in_handle, the out_handle should adjust to maintain colinearity
        // The out_handle should be at the same distance from the keyframe but in the opposite direction
        
        // Calculate the vectors from keyframe to handles
        BezierHandle kf_point(kf.time(), kf.value());
        BezierHandle in_vec = kf.in_handle() - kf_point;
        BezierHandle out_vec = kf.out_handle() - kf_point;
        
        // Check that the vectors are pointing in opposite directions (should be colinear)
        double in_slope = in_vec.value / in_vec.time;
        double out_slope = out_vec.value / out_vec.time;
        
        // Slopes should be approximately equal
        REQUIRE(in_slope == Catch::Approx(out_slope));
        
        // Lengths should be equal
        double in_length = in_vec.length();
        double out_length = out_vec.length();
        REQUIRE(in_length == Catch::Approx(out_length));
    }
    
    SECTION("SMOOTH_AUTO mode handle adjustment") {
        Channel channel;
        // Set up a sequence of keyframes with SMOOTH_AUTO mode
        channel.set_keyframe_at_time(1.0, 2.0, 
                                   BezierHandle(0.8, 2.0), 
                                   BezierHandle(1.2, 2.0), 
                                   TangentMode::smoothAuto);
        channel.set_keyframe_at_time(2.0, 3.0, 
                                   BezierHandle(1.8, 3.0), 
                                   BezierHandle(2.2, 3.0), 
                                   TangentMode::smoothAuto);
        channel.set_keyframe_at_time(3.0, 1.0, 
                                   BezierHandle(2.8, 1.0), 
                                   BezierHandle(3.2, 1.0), 
                                   TangentMode::smoothAuto);
        
        // Get the middle keyframe to test
        Keyframe& kf = channel.get_keyframe(1);
        
        // Store original handle values
        BezierHandle orig_in_handle = kf.in_handle();
        BezierHandle orig_out_handle = kf.out_handle();
        
        // Try to adjust the in_handle (should get auto-calculated by the channel)
        BezierHandle new_in_handle(1.5, 2.5);
        kf.set_in_handle(new_in_handle);
        
        // For smoothAuto, the handles should be automatically calculated based on neighboring keyframes
        // We just verify that they're not what we tried to set manually
        
        // In a proper implementation, changing the keyframe time should update the handles
        kf.set_time(2.1);
        
        // Handles should be recalculated
        REQUIRE(kf.in_handle().time != Catch::Approx(new_in_handle.time));
        REQUIRE(kf.out_handle().time != Catch::Approx(orig_out_handle.time));
    }
    
    SECTION("BROKEN mode handle adjustment") {
        Channel channel;
        // Set up keyframe with BROKEN mode
        channel.set_keyframe_at_time(2.0, 3.0, 
                                   BezierHandle(1.7, 2.7), // In-handle
                                   BezierHandle(2.3, 3.3), // Out-handle
                                   TangentMode::broken);
        
        // Get the keyframe to test
        Keyframe& kf = channel.get_keyframe(0);
        
        // Store original handle values
        BezierHandle orig_in_handle = kf.in_handle();
        BezierHandle orig_out_handle = kf.out_handle();
        
        // Adjust the in_handle
        BezierHandle new_in_handle(1.5, 2.5);
        kf.set_in_handle(new_in_handle);
        
        // In broken mode, adjusting in_handle should not affect the out_handle
        REQUIRE(kf.in_handle().time == Catch::Approx(1.5));
        REQUIRE(kf.in_handle().value == Catch::Approx(2.5));
        REQUIRE(kf.out_handle().time == Catch::Approx(orig_out_handle.time));
        REQUIRE(kf.out_handle().value == Catch::Approx(orig_out_handle.value));
        
        // Adjust the out_handle
        BezierHandle new_out_handle(2.5, 3.5);
        kf.set_out_handle(new_out_handle);
        
        // Both handles should now be adjusted independently
        REQUIRE(kf.in_handle().time == Catch::Approx(1.5));
        REQUIRE(kf.in_handle().value == Catch::Approx(2.5));
        REQUIRE(kf.out_handle().time == Catch::Approx(2.5));
        REQUIRE(kf.out_handle().value == Catch::Approx(3.5));
    }
}
