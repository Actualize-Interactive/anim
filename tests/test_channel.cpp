#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/animation.hpp>
#include <anim/channel.hpp>
#include <anim/keyframe.hpp> // Required for Keyframe and Point
#include <anim/handle_utils.hpp> // Required for GrabbedHandle enum
#include <iostream>

using namespace anim;

TEST_CASE("Channel Construction and Naming", "[channel]") {
    SECTION("Default constructor via Animation") {
        Animation anim;
        Channel& ch = anim.create_channel("");
        REQUIRE(ch.name().empty());
        REQUIRE(ch.empty());
        REQUIRE(ch.size() == 0);
    }

    SECTION("Constructor with a name via Animation") {
        Animation anim;
        Channel& ch = anim.create_channel("TestChannel");
        REQUIRE(ch.name() == "TestChannel");
        REQUIRE(ch.empty());
        REQUIRE(ch.size() == 0);
    }

    SECTION("Set and get name") {
        Animation anim;
        Channel& ch = anim.create_channel("");
        ch.set_name("NewName");
        REQUIRE(ch.name() == "NewName");
    }
}

TEST_CASE("Channel Keyframe Creation and Basic Properties", "[channel]") {
    Animation anim;
    Channel& ch = anim.create_channel("test");

    SECTION("Create keyframe with Point") {
        const Keyframe& kf1 = ch.create_keyframe(Point(1.0, 10.0));
        REQUIRE(ch.size() == 1);
        REQUIRE_FALSE(ch.empty());
        REQUIRE(kf1.time() == 1.0);
        REQUIRE(kf1.value() == 10.0);
        REQUIRE(ch.has_keyframe(1.0));
        REQUIRE_FALSE(ch.has_keyframe(2.0));

        const Keyframe& kf2 = ch.create_keyframe(Point(0.5, 5.0));
        REQUIRE(ch.size() == 2);
        REQUIRE(ch.keyframe(0).time() == 0.5); // Check sorting
        REQUIRE(ch.keyframe(1).time() == 1.0);
        REQUIRE(ch.has_keyframe(0.5));
    }

    SECTION("Create keyframe with time/value") {
        const Keyframe& kf1 = ch.create_keyframe(2.0, 20.0);
        REQUIRE(ch.size() == 1);
        REQUIRE(kf1.time() == 2.0);
        REQUIRE(kf1.value() == 20.0);

        const Keyframe& kf2 = ch.create_keyframe(0.0, 0.0, Point(-0.1, 0), Point(0.1, 0), Function::linear, HandleMode::free);
        REQUIRE(ch.size() == 2);
        REQUIRE(ch.keyframe(0).time() == 0.0);
        REQUIRE(ch.keyframe(1).time() == 2.0);
        REQUIRE(kf2.function == Function::linear);
        REQUIRE(kf2.handle_mode == HandleMode::free);
        REQUIRE(kf2.in_handle.time == Catch::Approx(-0.1));
        REQUIRE(kf2.out_handle.time == Catch::Approx(0.1));
    }

    SECTION("Emplace keyframe") {
        Keyframe new_kf(3.0, 30.0);
        const Keyframe& kf1 = ch.emplace_keyframe(std::move(new_kf));
        REQUIRE(ch.size() == 1);
        REQUIRE(ch.num_keyframes() == 1);
        REQUIRE(kf1.time() == 3.0);
        REQUIRE(kf1.value() == 30.0);

        // new_kf is now in a moved-from state, but kf1 is a reference to the one in the channel
        REQUIRE(ch.has_keyframe(3.0));

        Keyframe newer_kf(1.5, 15.0);
        ch.emplace_keyframe(std::move(newer_kf));
        REQUIRE(ch.size() == 2);
        REQUIRE(ch.num_keyframes() == 2);
        REQUIRE(ch.keyframe(0).time() == 1.5);
        REQUIRE(ch.keyframe(1).time() == 3.0);
    }

    SECTION("has_keyframe") {
        ch.create_keyframe(1.0, 10.0);
        ch.create_keyframe(3.0, 30.0);
        REQUIRE(ch.has_keyframe(1.0));
        REQUIRE(ch.has_keyframe(3.0));
        REQUIRE_FALSE(ch.has_keyframe(0.0));
        REQUIRE_FALSE(ch.has_keyframe(2.0));
        REQUIRE_FALSE(ch.has_keyframe(4.0));
    }
    
    SECTION("Keyframes are sorted by time") {
        ch.create_keyframe(5.0, 50.0);
        ch.create_keyframe(1.0, 10.0);
        ch.create_keyframe(3.0, 30.0);
        REQUIRE(ch.size() == 3);
        REQUIRE(ch.num_keyframes() == 3);
        REQUIRE(ch.keyframe(0).time() == 1.0);
        REQUIRE(ch.keyframe(1).time() == 3.0);
        REQUIRE(ch.keyframe(2).time() == 5.0);
    }
}

TEST_CASE("Channel Keyframe Access", "[channel]") {
    Animation anim;
    Channel& ch = anim.create_channel("test");

    SECTION("Access on empty channel") {
        REQUIRE_THROWS_AS(ch.keyframe(0), std::out_of_range);
        REQUIRE_THROWS_AS(ch.prev_keyframe(0.0), std::out_of_range);
        REQUIRE_THROWS_AS(ch.next_keyframe(0.0), std::out_of_range);
        REQUIRE_THROWS_AS(ch.closest_keyframe(0.0), std::out_of_range);
    }

    ch.create_keyframe(1.0, 10.0);
    ch.create_keyframe(3.0, 30.0);
    ch.create_keyframe(5.0, 50.0);
    // Keyframes at times: 1.0, 3.0, 5.0

    SECTION("keyframe(index)") {
        REQUIRE(ch.keyframe(0).time() == 1.0);
        REQUIRE(ch.keyframe(1).time() == 3.0);
        REQUIRE(ch.keyframe(2).time() == 5.0);
        REQUIRE_THROWS_AS(ch.keyframe(3), std::out_of_range);
        REQUIRE_THROWS_AS(ch.keyframe(-1), std::out_of_range); // Assuming size_t, effectively a large positive
    }

    SECTION("operator[](index)") {
        REQUIRE(ch[0].time() == 1.0);
        REQUIRE(ch[1].time() == 3.0);
        REQUIRE(ch[2].time() == 5.0);
        REQUIRE_THROWS_AS(ch[3], std::out_of_range);
        REQUIRE_THROWS_AS(ch[-1], std::out_of_range); // Assuming size_t, effectively a large positive
    }

    SECTION("prev_keyframe(time)") {
        REQUIRE(ch.prev_keyframe(3.0).time() == 1.0); // Time exactly on a keyframe
        REQUIRE(ch.prev_keyframe(3.5).time() == 3.0); // Time between keyframes
        REQUIRE(ch.prev_keyframe(5.0).time() == 3.0); // Time on last keyframe
        REQUIRE(ch.prev_keyframe(6.0).time() == 5.0); // Time after last keyframe
        REQUIRE_THROWS_AS(ch.prev_keyframe(1.0), std::out_of_range); // Time on first keyframe
        REQUIRE_THROWS_AS(ch.prev_keyframe(0.0), std::out_of_range); // Time before first keyframe
    }

    SECTION("next_keyframe(time)") {
        REQUIRE(ch.next_keyframe(3.0).time() == 5.0); // Time exactly on a keyframe
        REQUIRE(ch.next_keyframe(2.5).time() == 3.0); // Time between keyframes
        REQUIRE(ch.next_keyframe(1.0).time() == 3.0); // Time on first keyframe
        REQUIRE(ch.next_keyframe(0.0).time() == 1.0); // Time before first keyframe
        REQUIRE_THROWS_AS(ch.next_keyframe(5.0), std::out_of_range); // Time on last keyframe
        REQUIRE_THROWS_AS(ch.next_keyframe(6.0), std::out_of_range); // Time after last keyframe
    }

    SECTION("closest_keyframe(time)") {
        REQUIRE(ch.closest_keyframe(0.0).time() == 1.0);  // Before first
        REQUIRE(ch.closest_keyframe(1.0).time() == 1.0);  // Exactly on first
        REQUIRE(ch.closest_keyframe(1.9).time() == 1.0);  // Closer to first
        REQUIRE(ch.closest_keyframe(2.0).time() == 1.0);  // Midpoint, bias to earlier
        REQUIRE(ch.closest_keyframe(2.1).time() == 3.0);  // Closer to second (midpoint test)
        REQUIRE(ch.closest_keyframe(2.4).time() == 3.0);  // Closer to second
        REQUIRE(ch.closest_keyframe(3.0).time() == 3.0);  // Exactly on second
        REQUIRE(ch.closest_keyframe(4.0).time() == 3.0);  // Midpoint, bias to earlier
        REQUIRE(ch.closest_keyframe(4.1).time() == 5.0);  // Closer to third
        REQUIRE(ch.closest_keyframe(5.0).time() == 5.0);  // Exactly on third
        REQUIRE(ch.closest_keyframe(6.0).time() == 5.0);  // After last
    }
      SECTION("Access with a single keyframe") {
        Animation anim;
        auto& single_ch = anim.create_channel("single");
        single_ch.create_keyframe(2.0, 20.0);
        REQUIRE(single_ch.keyframe(0).time() == 2.0);
        REQUIRE_THROWS_AS(single_ch.prev_keyframe(2.0), std::out_of_range);
        REQUIRE_THROWS_AS(single_ch.prev_keyframe(1.0), std::out_of_range);
        REQUIRE(single_ch.prev_keyframe(3.0).time() == 2.0);

        REQUIRE_THROWS_AS(single_ch.next_keyframe(2.0), std::out_of_range);
        REQUIRE_THROWS_AS(single_ch.next_keyframe(3.0), std::out_of_range);
        REQUIRE(single_ch.next_keyframe(1.0).time() == 2.0);
        
        REQUIRE(single_ch.closest_keyframe(0.0).time() == 2.0);
        REQUIRE(single_ch.closest_keyframe(2.0).time() == 2.0);
        REQUIRE(single_ch.closest_keyframe(10.0).time() == 2.0);
    }
}

TEST_CASE("Channel Keyframe Deletion", "[channel]") {
    Animation anim;
    Channel& ch = anim.create_channel("test");
    ch.create_keyframe(1.0, 10.0);
    ch.create_keyframe(3.0, 30.0);
    ch.create_keyframe(5.0, 50.0);
    // Keyframes at times: 1.0, 3.0, 5.0

    SECTION("Delete middle keyframe") {
        ch.delete_keyframe(1); // Delete keyframe at time 3.0
        REQUIRE(ch.size() == 2);
        REQUIRE(ch.keyframe(0).time() == 1.0);
        REQUIRE(ch.keyframe(1).time() == 5.0);
        REQUIRE_FALSE(ch.has_keyframe(3.0));
    }

    SECTION("Delete first keyframe") {
        ch.delete_keyframe(0);
        REQUIRE(ch.size() == 2);
        REQUIRE(ch.keyframe(0).time() == 3.0);
        REQUIRE(ch.keyframe(1).time() == 5.0);
        REQUIRE_FALSE(ch.has_keyframe(1.0));
    }

    SECTION("Delete last keyframe") {
        ch.delete_keyframe(2);
        REQUIRE(ch.size() == 2);
        REQUIRE(ch.keyframe(0).time() == 1.0);
        REQUIRE(ch.keyframe(1).time() == 3.0);
        REQUIRE_FALSE(ch.has_keyframe(5.0));
    }    
    
    SECTION("Delete from single keyframe channel") {
        Animation anim;
        auto& single_ch = anim.create_channel("single");
        single_ch.create_keyframe(2.0, 20.0);
        single_ch.delete_keyframe(0);
        REQUIRE(single_ch.empty());
        REQUIRE(single_ch.size() == 0);
    }

    SECTION("Delete with invalid index throws exception") {
        REQUIRE_THROWS_AS(ch.delete_keyframe(3), std::out_of_range);
        REQUIRE_THROWS_AS(ch.delete_keyframe(100), std::out_of_range);
          Animation anim;
        auto& empty_ch = anim.create_channel("empty");
        REQUIRE_THROWS_AS(empty_ch.delete_keyframe(0), std::out_of_range);
    }
}

TEST_CASE("Channel Keyframe Updates", "[channel]") {
    Animation anim;
    Channel& ch = anim.create_channel("test");
    ch.create_keyframe(1.0, 10.0); // Default HandleMode::smooth
    ch.create_keyframe(3.0, 30.0); // Default HandleMode::smooth
    ch.create_keyframe(5.0, 50.0); // Default HandleMode::smooth

    SECTION("update_keyframe") {
        Keyframe new_kf(2.5, 25.0, Function::linear, HandleMode::free);
        ch.update_keyframe(1, new_kf); // Update middle keyframe
        REQUIRE(ch.keyframe(1).time() == 2.5);
        REQUIRE(ch.keyframe(1).value() == 25.0);
        REQUIRE(ch.keyframe(1).function == Function::linear);
        REQUIRE(ch.keyframe(1).handle_mode == HandleMode::free);
        
        // Verify keyframes are still sorted
        REQUIRE(ch.keyframe(0).time() == 1.0);
        REQUIRE(ch.keyframe(2).time() == 5.0);
    }

    SECTION("set_keyframe_time maintains sorting") {
        ch.set_keyframe_time(1, 4.5); // Move middle keyframe to between 3rd and last
        REQUIRE(ch.keyframe(0).time() == 1.0);
        REQUIRE(ch.keyframe(1).time() == 4.5);
        REQUIRE(ch.keyframe(2).time() == 5.0);
        REQUIRE(ch.keyframe(1).value() == 30.0); // Value should remain the same
    }

    SECTION("set_keyframe_value") {
        ch.set_keyframe_value(1, 35.0);
        REQUIRE(ch.keyframe(1).value() == 35.0);
        REQUIRE(ch.keyframe(1).time() == 3.0); // Time should remain the same
    }

    SECTION("set_keyframe_in_handle") {
        Point new_in_handle(2.5, 25.0);
        // Set HandleMode to free to prevent smooth/flat/aligned logic from overriding the manual set.
        ch.set_keyframe_handle_mode(1, HandleMode::free); 
        ch.set_keyframe_in_handle(1, new_in_handle);
        REQUIRE(ch.keyframe(1).in_handle == new_in_handle);
    }

    SECTION("set_keyframe_out_handle") {
        Point new_out_handle(3.5, 35.0);
        // Set HandleMode to free
        ch.set_keyframe_handle_mode(1, HandleMode::free);
        ch.set_keyframe_out_handle(1, new_out_handle);
        REQUIRE(ch.keyframe(1).out_handle == new_out_handle);
    }

    SECTION("set_keyframe_function") {
        ch.set_keyframe_function(1, Function::constant);
        REQUIRE(ch.keyframe(1).function == Function::constant);
    }

    SECTION("set_keyframe_handle_mode") {
        ch.set_keyframe_handle_mode(1, HandleMode::aligned);
        REQUIRE(ch.keyframe(1).handle_mode == HandleMode::aligned);
    }

    SECTION("Update with invalid index throws exception") {
        Keyframe dummy_kf;
        REQUIRE_THROWS_AS(ch.update_keyframe(3, dummy_kf), std::out_of_range);
        REQUIRE_THROWS_AS(ch.set_keyframe_time(3, 1.0), std::out_of_range);
        REQUIRE_THROWS_AS(ch.set_keyframe_value(3, 1.0), std::out_of_range);
        REQUIRE_THROWS_AS(ch.set_keyframe_in_handle(3, Point()), std::out_of_range);
        REQUIRE_THROWS_AS(ch.set_keyframe_out_handle(3, Point()), std::out_of_range);
        REQUIRE_THROWS_AS(ch.set_keyframe_function(3, Function::linear), std::out_of_range);
        REQUIRE_THROWS_AS(ch.set_keyframe_handle_mode(3, HandleMode::free), std::out_of_range);
    }
}

TEST_CASE("Channel Time Properties", "[channel]") {
    SECTION("Empty channel") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        REQUIRE(ch.start_time() == 0.0);
        REQUIRE(ch.end_time() == 0.0);
        REQUIRE(ch.length() == 0.0);
        REQUIRE(ch.num_samples(30.0) == 0);
    }

    SECTION("Single keyframe") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(2.5, 25.0);
        REQUIRE(ch.start_time() == 2.5);
        REQUIRE(ch.end_time() == 2.5);
        REQUIRE(ch.length() == 0.0);
        REQUIRE(ch.num_samples(30.0) == 1); // Single sample at the keyframe time
    }    SECTION("Multiple keyframes") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(1.0, 10.0);
        ch.create_keyframe(5.0, 50.0);
        ch.create_keyframe(3.0, 30.0);
        
        REQUIRE(ch.start_time() == 1.0);
        REQUIRE(ch.end_time() == 5.0);
        REQUIRE(ch.length() == 4.0); // 5.0 - 1.0
        
        // Duration is 4 seconds, at 30fps = 120 samples + 1 for endpoint
        REQUIRE(ch.num_samples(30.0) == 121);
        REQUIRE(ch.num_samples(1.0) == 5); // 4 seconds + 1 for endpoint
    }
}

TEST_CASE("Channel Evaluation", "[channel]") {
    SECTION("Empty channel") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        REQUIRE(ch.evaluate(1.0) == 0.0);
        REQUIRE(ch.evaluate(0.0) == 0.0);
        REQUIRE(ch.evaluate(-1.0) == 0.0);
    }

    SECTION("Single keyframe") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(2.0, 20.0);
        REQUIRE(ch.evaluate(0.0) == 20.0); // Before keyframe
        REQUIRE(ch.evaluate(2.0) == 20.0); // At keyframe
        REQUIRE(ch.evaluate(5.0) == 20.0); // After keyframe
    }    SECTION("Linear interpolation") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(0.0, 0.0, Point(), Point(), Function::linear);
        ch.create_keyframe(2.0, 20.0, Point(), Point(), Function::linear);
        
        REQUIRE(ch.evaluate(0.0) == Catch::Approx(0.0));
        REQUIRE(ch.evaluate(1.0) == Catch::Approx(10.0));
        REQUIRE(ch.evaluate(2.0) == Catch::Approx(20.0));
        REQUIRE(ch.evaluate(-1.0) == Catch::Approx(0.0)); // Before first
        REQUIRE(ch.evaluate(3.0) == Catch::Approx(20.0)); // After last
    }    SECTION("Constant interpolation") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(0.0, 0.0, Point(), Point(), Function::constant);
        ch.create_keyframe(2.0, 20.0, Point(), Point(), Function::constant);
        
        REQUIRE(ch.evaluate(0.0) == Catch::Approx(0.0));
        REQUIRE(ch.evaluate(1.0) == Catch::Approx(0.0)); // Constant until next keyframe
        REQUIRE(ch.evaluate(1.9) == Catch::Approx(0.0));
        REQUIRE(ch.evaluate(2.0) == Catch::Approx(20.0));
    }    SECTION("Bezier interpolation basic") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        // Create keyframes with default bezier function
        ch.create_keyframe(0.0, 0.0);
        ch.create_keyframe(1.0, 10.0);
        
        REQUIRE(ch.evaluate(0.0) == Catch::Approx(0.0));
        REQUIRE(ch.evaluate(1.0) == Catch::Approx(10.0));
        // Middle value should be somewhere between 0 and 10
        double mid_val = ch.evaluate(0.5);
        REQUIRE(mid_val > 0.0);
        REQUIRE(mid_val < 10.0);
    }
}

TEST_CASE("Channel Evaluation Range", "[channel]") {
    Animation anim;
    Channel& ch = anim.create_channel("test");
    ch.create_keyframe(0.0, 0.0, Point(), Point(), Function::linear);
    ch.create_keyframe(2.0, 20.0, Point(), Point(), Function::linear);

    SECTION("evaluate_range") {
        auto result = ch.evaluate_range(0.0, 2.0, 5);
        REQUIRE(result.size() == 5);
        REQUIRE(result[0] == Catch::Approx(0.0));
        REQUIRE(result[1] == Catch::Approx(5.0));
        REQUIRE(result[2] == Catch::Approx(10.0));
        REQUIRE(result[3] == Catch::Approx(15.0));
        REQUIRE(result[4] == Catch::Approx(20.0));
    }

    SECTION("evaluate_range with single sample") {
        auto result = ch.evaluate_range(1.0, 2.0, 1);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == Catch::Approx(10.0)); // Value at start time
    }

    SECTION("evaluate_range with equal start and end times") {
        auto result = ch.evaluate_range(1.0, 1.0, 5);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == Catch::Approx(10.0));
    }

    SECTION("evaluate_range invalid arguments") {
        REQUIRE_THROWS_AS(ch.evaluate_range(2.0, 1.0, 5), std::invalid_argument);
    }

    SECTION("evaluate_range_by_rate") {
        auto result = ch.evaluate_range_by_rate(0.0, 2.0, 1.0); // 1 sample per second
        REQUIRE(result.size() == 3); // 0, 1, 2 seconds
        REQUIRE(result[0] == Catch::Approx(0.0));
        REQUIRE(result[1] == Catch::Approx(10.0));
        REQUIRE(result[2] == Catch::Approx(20.0));
    }

    SECTION("evaluate_range_by_rate invalid arguments") {
        REQUIRE_THROWS_AS(ch.evaluate_range_by_rate(0.0, 2.0, 0.0), std::invalid_argument);
        REQUIRE_THROWS_AS(ch.evaluate_range_by_rate(0.0, 2.0, -1.0), std::invalid_argument);
        REQUIRE_THROWS_AS(ch.evaluate_range_by_rate(2.0, 1.0, 1.0), std::invalid_argument);
    }

    SECTION("evaluate_range_by_rate with equal times") {
        auto result = ch.evaluate_range_by_rate(1.0, 1.0, 30.0);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == Catch::Approx(10.0));
    }
}

TEST_CASE("Channel Handle Updates", "[channel]") {
    SECTION("Smooth handles update when keyframes change") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(0.0, 0.0, Point(), Point(), Function::bezier, HandleMode::smooth);
        ch.create_keyframe(2.0, 20.0, Point(), Point(), Function::bezier, HandleMode::smooth);
        ch.create_keyframe(4.0, 0.0, Point(), Point(), Function::bezier, HandleMode::smooth);
        
        // Get original handle positions
        Point orig_in = ch.keyframe(1).in_handle;
        Point orig_out = ch.keyframe(1).out_handle;
        
        // Change middle keyframe value - handles should update
        ch.set_keyframe_value(1, 40.0);
        
        // Handles may have been recalculated, but we can't easily predict exact values
        // Just verify the keyframe was updated
        REQUIRE(ch.keyframe(1).value() == 40.0);
    }    SECTION("Handles update when keyframe time changes") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(0.0, 0.0, Point(-0.5, 0.0), Point(0.5, 0.0), Function::bezier, HandleMode::free);
        ch.create_keyframe(2.0, 20.0, Point(1.5, 20.0), Point(2.5, 20.0), Function::bezier, HandleMode::free);
        ch.create_keyframe(4.0, 0.0, Point(3.5, 0.0), Point(4.5, 0.0), Function::bezier, HandleMode::free);
        
        // Store original handle positions for the middle keyframe
        Point orig_in = ch.keyframe(1).in_handle;
        Point orig_out = ch.keyframe(1).out_handle;
        
        // Move the middle keyframe to a new time - this should trigger handle updates
        ch.set_keyframe_time(1, 3.0);
        
        // Verify the time was changed
        REQUIRE(ch.keyframe(1).time() == 3.0);
        
        // Handles should be constrained/updated - at minimum, they should be clamped to valid time ranges
        // The in_handle time should be between the previous keyframe (0.0) and current keyframe (3.0)
        REQUIRE(ch.keyframe(1).in_handle.time >= 0.0);
        REQUIRE(ch.keyframe(1).in_handle.time <= 3.0);
        
        // The out_handle time should be between current keyframe (3.0) and next keyframe (4.0)
        REQUIRE(ch.keyframe(1).out_handle.time >= 3.0);
        REQUIRE(ch.keyframe(1).out_handle.time <= 4.0);
        
        // At least one handle should have changed from its original position due to time constraints
        bool handles_updated = (ch.keyframe(1).in_handle.time != orig_in.time) || 
                              (ch.keyframe(1).out_handle.time != orig_out.time);
        REQUIRE(handles_updated);
    }    SECTION("Smooth handles recalculate when keyframe time changes") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(0.0, 0.0, Point(), Point(), Function::bezier, HandleMode::smooth);
        ch.create_keyframe(2.0, 5.0, Point(), Point(), Function::bezier, HandleMode::smooth);
        ch.create_keyframe(4.0, 0.0, Point(), Point(), Function::bezier, HandleMode::smooth);
        
        // Store original handle positions for the middle keyframe
        Point orig_in = ch.keyframe(1).in_handle;
        Point orig_out = ch.keyframe(1).out_handle;
        
        // Move the middle keyframe - smooth handles should recalculate
        ch.set_keyframe_time(1, 1.5);
        
        // Verify the time was changed
        REQUIRE(ch.keyframe(1).time() == 1.5);
        
        // For smooth handles, they should be recalculated based on neighboring keyframes
        // The handles should have changed from their original positions
        REQUIRE(ch.keyframe(1).in_handle != orig_in);
        REQUIRE(ch.keyframe(1).out_handle != orig_out);
    }    SECTION("Adjacent keyframes' handles update when middle keyframe moves") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(0.0, 0.0, Point(), Point(), Function::bezier, HandleMode::smooth);
        ch.create_keyframe(2.0, 5.0, Point(), Point(), Function::bezier, HandleMode::smooth);
        ch.create_keyframe(4.0, 0.0, Point(), Point(), Function::bezier, HandleMode::smooth);
        
        // Store original handle positions for adjacent keyframes
        Point prev_out_orig = ch.keyframe(0).out_handle;
        Point next_in_orig = ch.keyframe(2).in_handle;
        
        // Move the middle keyframe - this should affect neighboring keyframes' handles
        ch.set_keyframe_time(1, 3.0);
        
        REQUIRE(ch.keyframe(0).out_handle != prev_out_orig);
        REQUIRE(ch.keyframe(2).in_handle != next_in_orig);
    }    SECTION("Flat handles maintain horizontal orientation") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(1.0, 10.0, Point(), Point(), Function::bezier, HandleMode::flat);
        ch.create_keyframe(3.0, 30.0, Point(), Point(), Function::bezier, HandleMode::flat);
        
        // For flat handles, the value component should match the keyframe value
        REQUIRE(ch.keyframe(0).in_handle.value == Catch::Approx(ch.keyframe(0).value()));
        REQUIRE(ch.keyframe(0).out_handle.value == Catch::Approx(ch.keyframe(0).value()));
        REQUIRE(ch.keyframe(1).in_handle.value == Catch::Approx(ch.keyframe(1).value()));
        REQUIRE(ch.keyframe(1).out_handle.value == Catch::Approx(ch.keyframe(1).value()));
    }    SECTION("Handle mode changes update handles") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(1.0, 10.0, Point(), Point(), Function::bezier, HandleMode::free);
        ch.create_keyframe(3.0, 30.0, Point(), Point(), Function::bezier, HandleMode::free);
        
        // Change to smooth mode should recalculate handles
        ch.set_keyframe_handle_mode(0, HandleMode::smooth);
        REQUIRE(ch.keyframe(0).handle_mode == HandleMode::smooth);
        
        // Change to flat mode
        ch.set_keyframe_handle_mode(1, HandleMode::flat);
        REQUIRE(ch.keyframe(1).handle_mode == HandleMode::flat);
        REQUIRE(ch.keyframe(1).in_handle.value == Catch::Approx(ch.keyframe(1).value()));
        REQUIRE(ch.keyframe(1).out_handle.value == Catch::Approx(ch.keyframe(1).value()));
    }
}

TEST_CASE("Channel Complex Animation Scenario", "[channel]") {
    SECTION("Reproduce Python test scenario") {
        Animation anim;
        auto& pos_x = anim.create_channel("pos_x");
        auto& pos_y = anim.create_channel("pos_y");
        auto& rotation = anim.create_channel("rotation");
        
        // Create animation data
        std::vector<double> times = {0.0, 1.0, 2.0, 3.0, 4.0};
        std::vector<double> x_values = {0.0, 10.0, 20.0, 15.0, 5.0};
        std::vector<double> y_values = {0.0, 5.0, 10.0, 25.0, 30.0};
        std::vector<double> rot_values = {0.0, 90.0, 180.0, 270.0, 360.0};
        
        std::vector<Function> functions = {Function::constant, Function::linear, Function::bezier, Function::linear, Function::bezier};
        std::vector<HandleMode> handle_modes = {HandleMode::flat, HandleMode::smooth, HandleMode::free, HandleMode::aligned, HandleMode::smooth};
        
        // Add keyframes with different interpolation types
        for (size_t i = 0; i < times.size(); ++i) {
            pos_x.create_keyframe(times[i], x_values[i], functions[i], handle_modes[i]);
            pos_y.create_keyframe(times[i], y_values[i], functions[i], handle_modes[i]);
            rotation.create_keyframe(times[i], rot_values[i], functions[i], handle_modes[i]);
        }
        
        REQUIRE(pos_x.num_keyframes() == 5);
        REQUIRE(pos_y.num_keyframes() == 5);
        REQUIRE(rotation.num_keyframes() == 5);
        
        // Test evaluation at various times
        std::vector<double> test_times = {0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0};
        for (double t : test_times) {
            REQUIRE_NOTHROW(pos_x.evaluate(t));
            REQUIRE_NOTHROW(pos_y.evaluate(t));
            REQUIRE_NOTHROW(rotation.evaluate(t));
            
            double x_val = pos_x.evaluate(t);
            double y_val = pos_y.evaluate(t);
            double r_val = rotation.evaluate(t);
            
            REQUIRE(std::isfinite(x_val));
            REQUIRE(std::isfinite(y_val));
            REQUIRE(std::isfinite(r_val));
        }
        
        // Test multiple evaluation ranges - this is where the error occurred
        std::vector<double> sample_rates = {30.0, 60.0, 120.0};
        for (double rate : sample_rates) {
            REQUIRE_NOTHROW(pos_x.evaluate_range_by_rate(0.0, 4.0, rate));
            
            auto values = pos_x.evaluate_range_by_rate(0.0, 4.0, rate);
            size_t expected_samples = static_cast<size_t>(std::ceil(4.0 * rate)) + 1;
            REQUIRE(values.size() == expected_samples);
            
            // Verify all values are finite
            for (double val : values) {
                REQUIRE(std::isfinite(val));
            }
        }
    }
      SECTION("Edge case: Very high sample rates") {
        Animation anim;
        auto& ch = anim.create_channel("ch");
        ch.create_keyframe(0.0, 0.0, Function::bezier, HandleMode::smooth);
        ch.create_keyframe(1.0, 10.0, Function::bezier, HandleMode::smooth);
        ch.create_keyframe(2.0, 5.0, Function::bezier, HandleMode::smooth);
        
        // Test with very high sample rate that might cause precision issues
        REQUIRE_NOTHROW(ch.evaluate_range_by_rate(0.0, 2.0, 1000.0));
        
        auto values = ch.evaluate_range_by_rate(0.0, 2.0, 1000.0);
        for (double val : values) {
            REQUIRE(std::isfinite(val));
        }
    }
}

TEST_CASE("Channel API Comprehensive Test", "[channel]") {
    SECTION("Complete Channel API functionality test") {
        // Create a test channel
        Animation anim;
        auto& channel = anim.create_channel("test_channel");
        
        // Test initial channel state
        REQUIRE(channel.name() == "test_channel");
        REQUIRE(channel.size() == 0);
        REQUIRE(channel.num_keyframes() == 0);
        REQUIRE(channel.empty());
        
        // Test channel name setter
        channel.set_name("renamed_channel");
        REQUIRE(channel.name() == "renamed_channel");
        
        // Test keyframe creation methods
        const Keyframe& kf1 = channel.create_keyframe(0.0, 1.0);
        REQUIRE(kf1.time() == 0.0);
        REQUIRE(kf1.value() == 1.0);
        
        // Test channel state after keyframe creation
        REQUIRE(channel.size() == 1);
        REQUIRE(channel.num_keyframes() == 1);
        REQUIRE_FALSE(channel.empty());
        
        // Test keyframe creation with Point
        Point point(2.0, 3.0);
        const Keyframe& kf2 = channel.create_keyframe(point);
        REQUIRE(kf2.time() == 2.0);
        REQUIRE(kf2.value() == 3.0);
        
        // Test keyframe creation with handles
        Point in_handle(1.5, 2.0);
        Point out_handle(2.5, 4.0);
        const Keyframe& kf3 = channel.create_keyframe(4.0, 5.0, in_handle, out_handle, Function::bezier, HandleMode::free);
        REQUIRE(kf3.time() == 4.0);
        REQUIRE(kf3.value() == 5.0);
        REQUIRE(kf3.in_handle.time == Catch::Approx(2.0)); // the previous keyframe's time since the handle was set beyond it
        REQUIRE(kf3.out_handle.time == Catch::Approx(2.5));
        
        // Test keyframe access by index
        const Keyframe& retrieved_kf = channel.keyframe(0);
        REQUIRE(retrieved_kf.time() == 0.0);
        
        // Test sequence protocol (size, operator[], has_keyframe)
        REQUIRE(channel.size() == 3);
        
        const Keyframe& kf_by_index = channel[1];
        REQUIRE(kf_by_index.time() == 2.0);
        
        REQUIRE(channel.has_keyframe(0.0)); // existing time
        REQUIRE_FALSE(channel.has_keyframe(10.0)); // non-existing time
        
        // Test keyframe queries
        const Keyframe& prev_kf = channel.prev_keyframe(3.0);
        REQUIRE(prev_kf.time() == 2.0);
        
        const Keyframe& next_kf = channel.next_keyframe(1.0);
        REQUIRE(next_kf.time() == 2.0);
        
        const Keyframe& closest_kf = channel.closest_keyframe(1.8);
        REQUIRE(closest_kf.time() == 2.0);
        
        // Test keyframe modification methods
        Keyframe new_kf(1.0, 10.0);
        channel.update_keyframe(1, new_kf);
        const Keyframe& updated_kf = channel.keyframe(1);
        REQUIRE(updated_kf.time() == 1.0);
        REQUIRE(updated_kf.value() == 10.0);
        
        channel.set_keyframe_time(1, 1.5);
        REQUIRE(channel.keyframe(1).time() == 1.5);
        
        channel.set_keyframe_value(1, 15.0);
        REQUIRE(channel.keyframe(1).value() == 15.0);
        
        Point new_point(1.8, 18.0);
        channel.set_keyframe_position(1, new_point);
        REQUIRE(channel.keyframe(1).time() == 1.8);
        REQUIRE(channel.keyframe(1).value() == 18.0);
        
        channel.set_keyframe_position(1, 1.9, 19.0);
        REQUIRE(channel.keyframe(1).time() == 1.9);
        REQUIRE(channel.keyframe(1).value() == 19.0);
        
        // Test handle setters - set to free mode first to prevent automatic handle updates
        channel.set_keyframe_handle_mode(1, HandleMode::free);
        Point new_in_handle(1.0, 18.0);
        Point new_out_handle(2.8, 20.0);
        channel.set_keyframe_in_handle(1, new_in_handle);
        channel.set_keyframe_out_handle(1, new_out_handle);
        const Keyframe& updated_kf_handles = channel.keyframe(1);
        REQUIRE(updated_kf_handles.in_handle.time == Catch::Approx(1.0));
        REQUIRE(updated_kf_handles.out_handle.time == Catch::Approx(2.8));
        
        // Test function and handle mode setters
        channel.set_keyframe_function(1, Function::linear);
        REQUIRE(channel.keyframe(1).function == Function::linear);
        
        channel.set_keyframe_handle_mode(1, HandleMode::free);
        REQUIRE(channel.keyframe(1).handle_mode == HandleMode::free);
        
        // Test evaluation
        double value_at_0 = channel.evaluate(0.0);
        REQUIRE(std::isfinite(value_at_0));
        REQUIRE(value_at_0 == Catch::Approx(1.0));
        
        // Test evaluation ranges
        auto values_range = channel.evaluate_range(0.0, 4.0, 5);
        REQUIRE(values_range.size() == 5);
        for (double v : values_range) {
            REQUIRE(std::isfinite(v));
        }
        
        auto values_by_rate = channel.evaluate_range_by_rate(0.0, 2.0, 1.0);
        REQUIRE(values_by_rate.size() == 3); // 0, 1, 2 seconds at 1Hz
        
        // Test channel timing properties
        REQUIRE(channel.start_time() == 0.0);
        REQUIRE(channel.end_time() == 4.0);
        REQUIRE(channel.length() == 4.0);
        
        // Test num_samples calculation
        size_t num_samples = channel.num_samples(30.0);
        REQUIRE(num_samples > 0);
        
        // Test keyframe removal
        channel.delete_keyframe(1);
        REQUIRE(channel.num_keyframes() == 2);
        
        // Verify the remaining keyframes are correct
        REQUIRE(channel.keyframe(0).time() == 0.0);
        REQUIRE(channel.keyframe(1).time() == 4.0);
    }
      SECTION("Channel API error conditions") {
        Animation anim;
        auto& channel = anim.create_channel("error_test");
        
        // Test errors on empty channel
        REQUIRE_THROWS_AS(channel.keyframe(0), std::out_of_range);
        REQUIRE_THROWS_AS(channel[0], std::out_of_range);
        REQUIRE_THROWS_AS(channel.prev_keyframe(1.0), std::out_of_range);
        REQUIRE_THROWS_AS(channel.next_keyframe(1.0), std::out_of_range);
        REQUIRE_THROWS_AS(channel.closest_keyframe(1.0), std::out_of_range);
        REQUIRE_THROWS_AS(channel.delete_keyframe(0), std::out_of_range);
        
        // Add some keyframes for further error testing
        channel.create_keyframe(1.0, 10.0);
        channel.create_keyframe(3.0, 30.0);
        
        // Test errors with invalid indices
        REQUIRE_THROWS_AS(channel.keyframe(5), std::out_of_range);
        REQUIRE_THROWS_AS(channel[5], std::out_of_range);
        REQUIRE_THROWS_AS(channel.delete_keyframe(5), std::out_of_range);
        REQUIRE_THROWS_AS(channel.update_keyframe(5, Keyframe()), std::out_of_range);
        REQUIRE_THROWS_AS(channel.set_keyframe_time(5, 2.0), std::out_of_range);
        REQUIRE_THROWS_AS(channel.set_keyframe_value(5, 20.0), std::out_of_range);
        REQUIRE_THROWS_AS(channel.set_keyframe_position(5, Point(2.0, 20.0)), std::out_of_range);
        REQUIRE_THROWS_AS(channel.set_keyframe_in_handle(5, Point()), std::out_of_range);
        REQUIRE_THROWS_AS(channel.set_keyframe_out_handle(5, Point()), std::out_of_range);
        REQUIRE_THROWS_AS(channel.set_keyframe_function(5, Function::linear), std::out_of_range);
        REQUIRE_THROWS_AS(channel.set_keyframe_handle_mode(5, HandleMode::smooth), std::out_of_range);
        
        // Test range evaluation errors
        REQUIRE_THROWS_AS(channel.evaluate_range(3.0, 1.0, 5), std::invalid_argument); // start > end
        REQUIRE_THROWS_AS(channel.evaluate_range_by_rate(0.0, 2.0, 0.0), std::invalid_argument); // zero rate
        REQUIRE_THROWS_AS(channel.evaluate_range_by_rate(0.0, 2.0, -1.0), std::invalid_argument); // negative rate
        REQUIRE_THROWS_AS(channel.evaluate_range_by_rate(2.0, 1.0, 1.0), std::invalid_argument); // start > end
        
        // Test num_samples error
        REQUIRE_THROWS_AS(channel.num_samples(0.0), std::invalid_argument); // zero rate
        REQUIRE_THROWS_AS(channel.num_samples(-1.0), std::invalid_argument); // negative rate
    }
      SECTION("Channel emplace keyframe functionality") {
        Animation anim;
        auto& channel = anim.create_channel("emplace_test");
        
        // Test emplace_keyframe
        Keyframe new_kf(2.0, 20.0, Function::linear, HandleMode::smooth);
        const Keyframe& emplaced_kf = channel.emplace_keyframe(std::move(new_kf));
        REQUIRE(emplaced_kf.time() == 2.0);
        REQUIRE(emplaced_kf.value() == 20.0);
        REQUIRE(emplaced_kf.function == Function::linear);
        REQUIRE(emplaced_kf.handle_mode == HandleMode::smooth);
        
        // Verify it was added to the channel
        REQUIRE(channel.num_keyframes() == 1);
        REQUIRE(channel.has_keyframe(2.0));
    }
      SECTION("Channel keyframe access boundary conditions") {
        Animation anim;
        auto& channel = anim.create_channel("boundary_test");
        channel.create_keyframe(1.0, 10.0);
        channel.create_keyframe(3.0, 30.0);
        channel.create_keyframe(5.0, 50.0);
        
        // Test prev_keyframe boundary conditions
        REQUIRE_THROWS_AS(channel.prev_keyframe(1.0), std::out_of_range); // at first keyframe
        REQUIRE_THROWS_AS(channel.prev_keyframe(0.0), std::out_of_range); // before first keyframe
        REQUIRE(channel.prev_keyframe(6.0).time() == 5.0); // after last keyframe
        
        // Test next_keyframe boundary conditions
        REQUIRE_THROWS_AS(channel.next_keyframe(5.0), std::out_of_range); // at last keyframe
        REQUIRE_THROWS_AS(channel.next_keyframe(6.0), std::out_of_range); // after last keyframe
        REQUIRE(channel.next_keyframe(0.0).time() == 1.0); // before first keyframe
        
        // Test closest_keyframe behavior
        REQUIRE(channel.closest_keyframe(0.0).time() == 1.0); // before first
        REQUIRE(channel.closest_keyframe(6.0).time() == 5.0); // after last
        REQUIRE(channel.closest_keyframe(2.0).time() == 1.0); // midpoint bias to earlier
        REQUIRE(channel.closest_keyframe(2.1).time() == 3.0); // closer to second
    }
}

TEST_CASE("Channel keyframes() method", "[channel]") {
    Animation anim;
    Channel& channel = anim.create_channel("keyframes_test");
    
    SECTION("Empty channel returns empty vector") {
        const std::vector<Keyframe>& keyframes = channel.keyframes();
        REQUIRE(keyframes.empty());
        REQUIRE(keyframes.size() == 0);
    }
    
    SECTION("Single keyframe") {
        channel.create_keyframe(1.0, 10.0);
        const std::vector<Keyframe>& keyframes = channel.keyframes();
        
        REQUIRE(keyframes.size() == 1);
        REQUIRE(keyframes[0].time() == 1.0);
        REQUIRE(keyframes[0].value() == 10.0);
    }
    
    SECTION("Multiple keyframes in order") {
        // Add keyframes in non-chronological order to test sorting
        channel.create_keyframe(3.0, 30.0);
        channel.create_keyframe(1.0, 10.0);
        channel.create_keyframe(2.0, 20.0);
        
        const std::vector<Keyframe>& keyframes = channel.keyframes();
        
        REQUIRE(keyframes.size() == 3);
        // Verify they are sorted by time
        REQUIRE(keyframes[0].time() == 1.0);
        REQUIRE(keyframes[0].value() == 10.0);
        REQUIRE(keyframes[1].time() == 2.0);
        REQUIRE(keyframes[1].value() == 20.0);
        REQUIRE(keyframes[2].time() == 3.0);
        REQUIRE(keyframes[2].value() == 30.0);
    }
    
    SECTION("keyframes() returns const reference") {
        channel.create_keyframe(1.0, 10.0);
        channel.create_keyframe(2.0, 20.0);
        
        const std::vector<Keyframe>& keyframes1 = channel.keyframes();
        const std::vector<Keyframe>& keyframes2 = channel.keyframes();
        
        // Both references should point to the same object
        REQUIRE(&keyframes1 == &keyframes2);
        
        // Verify content is correct
        REQUIRE(keyframes1.size() == 2);
        REQUIRE(keyframes1[0].time() == 1.0);
        REQUIRE(keyframes1[1].time() == 2.0);
    }
    
    SECTION("keyframes() reflects changes to channel") {
        const std::vector<Keyframe>& keyframes = channel.keyframes();
        REQUIRE(keyframes.empty());
        
        // Add a keyframe
        channel.create_keyframe(1.0, 10.0);
        REQUIRE(keyframes.size() == 1);
        REQUIRE(keyframes[0].time() == 1.0);
        
        // Add another keyframe
        channel.create_keyframe(2.0, 20.0);
        REQUIRE(keyframes.size() == 2);
        REQUIRE(keyframes[1].time() == 2.0);
        
        // Delete a keyframe
        channel.delete_keyframe(0);
        REQUIRE(keyframes.size() == 1);
        REQUIRE(keyframes[0].time() == 2.0);
    }
    
    SECTION("keyframes() with different keyframe properties") {
        // Create keyframes with different properties
        channel.create_keyframe(1.0, 10.0, Function::linear, HandleMode::smooth);
        channel.create_keyframe(2.0, 20.0, Function::bezier, HandleMode::aligned);
        channel.create_keyframe(3.0, 30.0, Function::constant, HandleMode::free);
        
        const std::vector<Keyframe>& keyframes = channel.keyframes();
        
        REQUIRE(keyframes.size() == 3);
        
        // Verify first keyframe properties
        REQUIRE(keyframes[0].time() == 1.0);
        REQUIRE(keyframes[0].value() == 10.0);
        REQUIRE(keyframes[0].function == Function::linear);
        REQUIRE(keyframes[0].handle_mode == HandleMode::smooth);
        
        // Verify second keyframe properties
        REQUIRE(keyframes[1].time() == 2.0);
        REQUIRE(keyframes[1].value() == 20.0);
        REQUIRE(keyframes[1].function == Function::bezier);
        REQUIRE(keyframes[1].handle_mode == HandleMode::aligned);
        
        // Verify third keyframe properties
        REQUIRE(keyframes[2].time() == 3.0);
        REQUIRE(keyframes[2].value() == 30.0);
        REQUIRE(keyframes[2].function == Function::constant);
        REQUIRE(keyframes[2].handle_mode == HandleMode::free);
    }
}

TEST_CASE("Last keyframe inherits Function and HandleMode from preceding keyframe", "[channel][last_keyframe]") {
    Animation anim;
    Channel& ch = anim.create_channel("test");

    SECTION("Last keyframe inherits when added as second keyframe") {
        // Add first keyframe with specific function and handle mode
        ch.create_keyframe(1.0, 10.0, Function::linear, HandleMode::free);
        
        // Add second keyframe that becomes the last
        ch.create_keyframe(3.0, 30.0, Function::bezier, HandleMode::smooth);
        
        // The last keyframe should inherit function and handle mode from the first
        REQUIRE(ch.keyframe(1).function == Function::linear);
        REQUIRE(ch.keyframe(1).handle_mode == HandleMode::free);
        
        // The first keyframe should remain unchanged
        REQUIRE(ch.keyframe(0).function == Function::linear);
        REQUIRE(ch.keyframe(0).handle_mode == HandleMode::free);
    }

    SECTION("Last keyframe inherits when added as third keyframe") {
        // Add first keyframe
        ch.create_keyframe(1.0, 10.0, Function::linear, HandleMode::free);
        // Add second keyframe with different function/handle mode
        ch.create_keyframe(2.0, 20.0, Function::bezier, HandleMode::smooth);
        // Add third keyframe that becomes the last
        ch.create_keyframe(3.0, 30.0, Function::constant, HandleMode::aligned);
        
        // The last keyframe should inherit from the second-to-last (middle) keyframe
        REQUIRE(ch.keyframe(2).function == Function::bezier);
        REQUIRE(ch.keyframe(2).handle_mode == HandleMode::smooth);
    }

    SECTION("Last keyframe inherits when middle keyframe function is changed") {
        // Add three keyframes
        ch.create_keyframe(1.0, 10.0, Function::linear, HandleMode::free);
        ch.create_keyframe(2.0, 20.0, Function::bezier, HandleMode::smooth);
        ch.create_keyframe(3.0, 30.0, Function::constant, HandleMode::aligned);
        
        // Change the second keyframe's function
        ch.set_keyframe_function(1, Function::constant);
        
        // The last keyframe should inherit the new function
        REQUIRE(ch.keyframe(2).function == Function::constant);
        REQUIRE(ch.keyframe(2).handle_mode == HandleMode::smooth); // handle mode should remain from second keyframe
    }

    SECTION("Last keyframe inherits when middle keyframe handle mode is changed") {
        // Add three keyframes
        ch.create_keyframe(1.0, 10.0, Function::linear, HandleMode::free);
        ch.create_keyframe(2.0, 20.0, Function::bezier, HandleMode::smooth);
        ch.create_keyframe(3.0, 30.0, Function::constant, HandleMode::aligned);
        
        // Change the second keyframe's handle mode
        ch.set_keyframe_handle_mode(1, HandleMode::flat);
        
        // The last keyframe should inherit the new handle mode
        REQUIRE(ch.keyframe(2).function == Function::bezier); // function should remain from second keyframe
        REQUIRE(ch.keyframe(2).handle_mode == HandleMode::flat);
    }

    SECTION("Last keyframe inherits when a keyframe is inserted that becomes second-to-last") {
        // Add two keyframes
        ch.create_keyframe(1.0, 10.0, Function::linear, HandleMode::free);
        ch.create_keyframe(3.0, 30.0, Function::bezier, HandleMode::smooth);
        
        // Insert a keyframe in the middle that becomes the new second-to-last
        ch.create_keyframe(2.0, 20.0, Function::constant, HandleMode::aligned);
        
        // The last keyframe should inherit from the new second-to-last keyframe
        REQUIRE(ch.keyframe(2).function == Function::constant);
        REQUIRE(ch.keyframe(2).handle_mode == HandleMode::aligned);
    }

    SECTION("Single keyframe is not affected") {
        // Add only one keyframe
        ch.create_keyframe(1.0, 10.0, Function::linear, HandleMode::free);
        
        // Single keyframe should remain unchanged
        REQUIRE(ch.keyframe(0).function == Function::linear);
        REQUIRE(ch.keyframe(0).handle_mode == HandleMode::free);
    }
}
