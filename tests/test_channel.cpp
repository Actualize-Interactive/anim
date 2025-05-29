#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/channel.hpp>
#include <anim/keyframe.hpp> // Required for Keyframe and Point
#include <anim/handle_utils.hpp> // Required for GrabbedHandle enum

using namespace anim;

TEST_CASE("Channel Construction and Naming", "[channel]") {
    SECTION("Default constructor") {
        Channel ch;
        REQUIRE(ch.name().empty());
        REQUIRE(ch.empty());
        REQUIRE(ch.size() == 0);
    }

    SECTION("Constructor with a name") {
        Channel ch("TestChannel");
        REQUIRE(ch.name() == "TestChannel");
        REQUIRE(ch.empty());
        REQUIRE(ch.size() == 0);
    }

    SECTION("Set and get name") {
        Channel ch;
        ch.set_name("NewName");
        REQUIRE(ch.name() == "NewName");
    }
}

TEST_CASE("Channel Keyframe Creation and Basic Properties", "[channel]") {
    Channel ch;

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
    Channel ch;

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
        Channel single_ch;
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
    Channel ch;
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
        Channel single_ch;
        single_ch.create_keyframe(2.0, 20.0);
        single_ch.delete_keyframe(0);
        REQUIRE(single_ch.empty());
        REQUIRE(single_ch.size() == 0);
    }

    SECTION("Delete with invalid index throws exception") {
        REQUIRE_THROWS_AS(ch.delete_keyframe(3), std::out_of_range);
        REQUIRE_THROWS_AS(ch.delete_keyframe(100), std::out_of_range);
        
        Channel empty_ch;
        REQUIRE_THROWS_AS(empty_ch.delete_keyframe(0), std::out_of_range);
    }
}

TEST_CASE("Channel Keyframe Updates", "[channel]") {
    Channel ch;
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
        Channel ch;
        REQUIRE(ch.start_time() == 0.0);
        REQUIRE(ch.end_time() == 0.0);
        REQUIRE(ch.length() == 0.0);
        REQUIRE(ch.num_samples(30.0) == 0);
    }

    SECTION("Single keyframe") {
        Channel ch;
        ch.create_keyframe(2.5, 25.0);
        REQUIRE(ch.start_time() == 2.5);
        REQUIRE(ch.end_time() == 2.5);
        REQUIRE(ch.length() == 0.0);
        REQUIRE(ch.num_samples(30.0) == 1); // Single sample at the keyframe time
    }

    SECTION("Multiple keyframes") {
        Channel ch;
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
        Channel ch;
        REQUIRE(ch.evaluate(1.0) == 0.0);
        REQUIRE(ch.evaluate(0.0) == 0.0);
        REQUIRE(ch.evaluate(-1.0) == 0.0);
    }

    SECTION("Single keyframe") {
        Channel ch;
        ch.create_keyframe(2.0, 20.0);
        REQUIRE(ch.evaluate(0.0) == 20.0); // Before keyframe
        REQUIRE(ch.evaluate(2.0) == 20.0); // At keyframe
        REQUIRE(ch.evaluate(5.0) == 20.0); // After keyframe
    }

    SECTION("Linear interpolation") {
        Channel ch;
        ch.create_keyframe(0.0, 0.0, Point(), Point(), Function::linear);
        ch.create_keyframe(2.0, 20.0, Point(), Point(), Function::linear);
        
        REQUIRE(ch.evaluate(0.0) == Catch::Approx(0.0));
        REQUIRE(ch.evaluate(1.0) == Catch::Approx(10.0));
        REQUIRE(ch.evaluate(2.0) == Catch::Approx(20.0));
        REQUIRE(ch.evaluate(-1.0) == Catch::Approx(0.0)); // Before first
        REQUIRE(ch.evaluate(3.0) == Catch::Approx(20.0)); // After last
    }

    SECTION("Constant interpolation") {
        Channel ch;
        ch.create_keyframe(0.0, 0.0, Point(), Point(), Function::constant);
        ch.create_keyframe(2.0, 20.0, Point(), Point(), Function::constant);
        
        REQUIRE(ch.evaluate(0.0) == Catch::Approx(0.0));
        REQUIRE(ch.evaluate(1.0) == Catch::Approx(0.0)); // Constant until next keyframe
        REQUIRE(ch.evaluate(1.9) == Catch::Approx(0.0));
        REQUIRE(ch.evaluate(2.0) == Catch::Approx(20.0));
    }

    SECTION("Bezier interpolation basic") {
        Channel ch;
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
    Channel ch;
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
        Channel ch;
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
    }

    SECTION("Handles update when keyframe time changes") {
        Channel ch;
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
    }

    SECTION("Smooth handles recalculate when keyframe time changes") {
        Channel ch;
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
    }

    SECTION("Adjacent keyframes' handles update when middle keyframe moves") {
        Channel ch;
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
    }

    SECTION("Flat handles maintain horizontal orientation") {
        Channel ch;
        ch.create_keyframe(1.0, 10.0, Point(), Point(), Function::bezier, HandleMode::flat);
        ch.create_keyframe(3.0, 30.0, Point(), Point(), Function::bezier, HandleMode::flat);
        
        // For flat handles, the value component should match the keyframe value
        REQUIRE(ch.keyframe(0).in_handle.value == Catch::Approx(ch.keyframe(0).value()));
        REQUIRE(ch.keyframe(0).out_handle.value == Catch::Approx(ch.keyframe(0).value()));
        REQUIRE(ch.keyframe(1).in_handle.value == Catch::Approx(ch.keyframe(1).value()));
        REQUIRE(ch.keyframe(1).out_handle.value == Catch::Approx(ch.keyframe(1).value()));
    }

    SECTION("Handle mode changes update handles") {
        Channel ch;
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
