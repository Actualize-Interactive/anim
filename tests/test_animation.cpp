#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/animation.hpp>

using namespace anim;

TEST_CASE("Animation empty state", "[animation]") {
    Animation anim;
    
    SECTION("Animation is empty") {
        REQUIRE(anim.is_empty());
        REQUIRE(anim.has_no_keyframes());
    }
    
    SECTION("No start or end time") {
        REQUIRE_FALSE(anim.get_start_time().has_value());
        REQUIRE_FALSE(anim.get_end_time().has_value());
    }
    
    SECTION("Empty channel list") {
        REQUIRE(anim.get_channel_names().empty());
    }
    
    SECTION("Evaluating empty animation returns empty map") {
        auto results = anim.evaluate_channels(0.0);
        REQUIRE(results.empty());
    }
}

TEST_CASE("Animation with multiple channels", "[animation]") {
    Animation anim;
    // Create and add channels
    Channel channel1("position.x");
    channel1.set_keyframe_at_time(1.0, 10.0, BezierHandle(0.9, 10.0), BezierHandle(1.1, 10.0), TangentMode::linear);
    channel1.set_keyframe_at_time(3.0, 30.0, BezierHandle(2.9, 30.0), BezierHandle(3.1, 30.0), TangentMode::linear);
    
    Channel channel2("position.y");
    channel2.set_keyframe_at_time(2.0, 20.0, BezierHandle(1.9, 20.0), BezierHandle(2.1, 20.0), TangentMode::linear);
    channel2.set_keyframe_at_time(4.0, 40.0, BezierHandle(3.9, 40.0), BezierHandle(4.1, 40.0), TangentMode::linear);
    
    anim.append_channel(channel1);
    anim.append_channel(channel2);
    
    SECTION("Animation is not empty") {
        REQUIRE_FALSE(anim.is_empty());
        REQUIRE_FALSE(anim.has_no_keyframes());
    }
    
    SECTION("Channel names are correct") {
        auto names = anim.get_channel_names();
        REQUIRE(names.size() == 2);
        REQUIRE(std::find(names.begin(), names.end(), "position.x") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "position.y") != names.end());
    }
    
    SECTION("Start and end times span all channels") {
        REQUIRE(anim.get_start_time().value() == Catch::Approx(1.0)); // Earliest keyframe
        REQUIRE(anim.get_end_time().value() == Catch::Approx(4.0));   // Latest keyframe
    }
    
    SECTION("Channel retrieval by name") {
        const Channel* ch1 = anim.get_channel("position.x");
        REQUIRE(ch1 != nullptr);
        REQUIRE(ch1->get_all_keyframes().size() == 2);
        
        Channel* ch2 = anim.get_channel("position.y");
        REQUIRE(ch2 != nullptr);
        REQUIRE(ch2->get_all_keyframes().size() == 2);
        
        const Channel* missing = anim.get_channel("nonexistent");
        REQUIRE(missing == nullptr);
    }
    
    SECTION("Channel retrieval by index") {
        const Channel* ch1 = anim.get_channel(0);
        REQUIRE(ch1 != nullptr);
        REQUIRE(ch1->name() == "position.x");
        
        Channel* ch2 = anim.get_channel(1);
        REQUIRE(ch2 != nullptr);
        REQUIRE(ch2->name() == "position.y");
        
        const Channel* invalid = anim.get_channel(999);
        REQUIRE(invalid == nullptr);
    }
    
    SECTION("Channel count") {
        REQUIRE(anim.num_channels() == 2);
    }
    
    SECTION("Evaluate all channels at specific time") {
        auto results = anim.evaluate_channels(2.0);
        REQUIRE(results.size() == 2);
        REQUIRE(results["position.x"] == Catch::Approx(20.0)); // Linear interpolation
        REQUIRE(results["position.y"] == Catch::Approx(20.0)); // At keyframe
    }
    
    SECTION("Evaluate range across all channels") {
        auto results = anim.evaluate_channels_range(1.0, 4.0, 4);
        REQUIRE(results.size() == 2);
        REQUIRE(results["position.x"].size() == 4);
        REQUIRE(results["position.y"].size() == 4);
        
        // Check first and last values
        REQUIRE(results["position.x"][0] == Catch::Approx(10.0));
        REQUIRE(results["position.x"][3] == Catch::Approx(30.0));
        REQUIRE(results["position.y"][0] == Catch::Approx(20.0/1.0)); // Linear interp from 20->40 over 2->4
        REQUIRE(results["position.y"][3] == Catch::Approx(40.0));
    }
    
    SECTION("Animation length") {
        REQUIRE(anim.length() == Catch::Approx(3.0)); // 4.0 - 1.0
    }
    
    SECTION("Number of samples") {
        REQUIRE(anim.num_samples(10.0) == 31); // (4-1)*10 + 1
    }
    
    SECTION("Remove channel by name") {
        REQUIRE(anim.remove_channel("position.x"));
        REQUIRE(anim.get_channel_names().size() == 1);
        REQUIRE_FALSE(anim.remove_channel("nonexistent"));
    }
    
    SECTION("Remove channel by index") {
        REQUIRE(anim.remove_channel(0));
        REQUIRE(anim.num_channels() == 1);
        REQUIRE_FALSE(anim.remove_channel(999));
    }
}

TEST_CASE("Animation with empty channel", "[animation]") {
    Animation anim;
    
    // Add an empty channel
    Channel empty_channel("empty");
    anim.append_channel(empty_channel);
    
    // Add a non-empty channel
    Channel channel("non_empty");
    channel.set_keyframe_at_time(1.0, 10.0, BezierHandle(0.9, 10.0), BezierHandle(1.1, 10.0), TangentMode::linear);
    anim.append_channel(channel);
    
    SECTION("Animation is not empty") {
        REQUIRE_FALSE(anim.is_empty());
    }
    
    SECTION("Animation has keyframes") {
        REQUIRE_FALSE(anim.has_no_keyframes());
    }
    
    SECTION("Start and end times only consider non-empty channels") {
        REQUIRE(anim.get_start_time().value() == Catch::Approx(1.0));
        REQUIRE(anim.get_end_time().value() == Catch::Approx(1.0));
    }
    
    SECTION("Evaluation includes empty channels") {
        auto results = anim.evaluate_channels(1.0);
        REQUIRE(results.size() == 2);
        REQUIRE(results["empty"] == Catch::Approx(0.0));
        REQUIRE(results["non_empty"] == Catch::Approx(10.0));
    }
}

TEST_CASE("Animation channel creation", "[animation]") {
    Animation anim;
    
    SECTION("Creating channels directly") {
        Channel* ch1 = anim.create_channel("position.x");
        REQUIRE(ch1 != nullptr);
        REQUIRE(ch1->name() == "position.x");
        REQUIRE(anim.num_channels() == 1);
        
        // Set a keyframe on the created channel
        ch1->set_keyframe_at_time(1.0, 5.0, BezierHandle(0.9, 5.0), BezierHandle(1.1, 5.0), TangentMode::linear);
        REQUIRE_FALSE(ch1->is_empty());
        
        // Channel should be accessible through both index and name
        REQUIRE(anim.get_channel(0) == ch1);
        REQUIRE(anim.get_channel("position.x") == ch1);
        REQUIRE(anim.has_channel("position.x"));
    }
    
    SECTION("Creating multiple channels") {
        anim.create_channel("position.x");
        anim.create_channel("position.y");
        anim.create_channel("position.z");
        
        REQUIRE(anim.num_channels() == 3);
        REQUIRE(anim.has_channel("position.x"));
        REQUIRE(anim.has_channel("position.y"));
        REQUIRE(anim.has_channel("position.z"));
        REQUIRE_FALSE(anim.has_channel("rotation"));
    }
}

TEST_CASE("Animation channel modification", "[animation]") {
    Animation anim;
    
    // Set up animation with multiple channels
    Channel* ch1 = anim.create_channel("channel1");
    ch1->set_keyframe_at_time(1.0, 10.0, BezierHandle(0.9, 10.0), BezierHandle(1.1, 10.0), TangentMode::linear);
    
    Channel* ch2 = anim.create_channel("channel2");
    ch2->set_keyframe_at_time(2.0, 20.0, BezierHandle(1.9, 20.0), BezierHandle(2.1, 20.0), TangentMode::linear);
    
    SECTION("Modifying channel through the animation pointer") {
        Channel* ch = anim.get_channel("channel1");
        REQUIRE(ch != nullptr);
        
        // Add a new keyframe
        ch->set_keyframe_at_time(3.0, 30.0, BezierHandle(2.9, 30.0), BezierHandle(3.1, 30.0), TangentMode::linear);
        
        // Verify the keyframe was added
        REQUIRE(ch->get_all_keyframes().size() == 2);
        
        // Verify evaluation works with the new keyframe
        REQUIRE(anim.evaluate_channels(3.0)["channel1"] == Catch::Approx(30.0));
    }
    
    SECTION("Has channel checks") {
        REQUIRE(anim.has_channel("channel1"));
        REQUIRE(anim.has_channel("channel2"));
        REQUIRE_FALSE(anim.has_channel("channel3"));
        
        // After removing a channel
        anim.remove_channel("channel1");
        REQUIRE_FALSE(anim.has_channel("channel1"));
        REQUIRE(anim.has_channel("channel2"));
    }
    
    SECTION("Evaluating channels after modification") {
        // Original evaluation
        auto results1 = anim.evaluate_channels(1.5);
        REQUIRE(results1["channel1"] == Catch::Approx(10.0));
        REQUIRE(results1["channel2"] == Catch::Approx(20.0));
        
        // Modify channel1
        Channel* ch = anim.get_channel("channel1");
        ch->set_keyframe_at_time(1.0, 100.0, BezierHandle(0.9, 100.0), BezierHandle(1.1, 100.0), TangentMode::linear);
        
        // Evaluation should reflect changes
        auto results2 = anim.evaluate_channels(1.5);
        REQUIRE(results2["channel1"] == Catch::Approx(100.0));
        REQUIRE(results2["channel2"] == Catch::Approx(20.0));
    }
}

TEST_CASE("Animation channel sampling", "[animation]") {
    Animation anim;
    
    // Create channels with keyframes at different times
    Channel* ch1 = anim.create_channel("ch1");
    ch1->set_keyframe_at_time(0.0, 0.0, BezierHandle(-0.1, 0.0), BezierHandle(0.1, 0.0), TangentMode::linear);
    ch1->set_keyframe_at_time(10.0, 100.0, BezierHandle(9.9, 100.0), BezierHandle(10.1, 100.0), TangentMode::linear);
    
    SECTION("Sampling with fixed count") {
        auto results = anim.evaluate_channels_range(0.0, 10.0, 11);
        
        REQUIRE(results["ch1"].size() == 11);
        REQUIRE(results["ch1"][0] == Catch::Approx(0.0));
        REQUIRE(results["ch1"][1] == Catch::Approx(10.0)); // Linear interpolation at 10%
        REQUIRE(results["ch1"][2] == Catch::Approx(20.0)); // Linear interpolation at 20%
        REQUIRE(results["ch1"][3] == Catch::Approx(30.0)); // Linear interpolation at 30%
        REQUIRE(results["ch1"][4] == Catch::Approx(40.0)); // Linear interpolation at 40%
        REQUIRE(results["ch1"][5] == Catch::Approx(50.0)); // Linear interpolation at 50%
        REQUIRE(results["ch1"][6] == Catch::Approx(60.0)); // Linear interpolation at 60%
        REQUIRE(results["ch1"][7] == Catch::Approx(70.0)); // Linear interpolation at 70%
        REQUIRE(results["ch1"][8] == Catch::Approx(80.0)); // Linear interpolation at 80%
        REQUIRE(results["ch1"][9] == Catch::Approx(90.0)); // Linear interpolation at 90%
        REQUIRE(results["ch1"][10] == Catch::Approx(100.0));
    }
    
    SECTION("Sampling with rate") {
        // Sample at 1.0 rate (one sample per unit of time)
        auto results = anim.evaluate_channels_range_by_rate(0.0, 10.0, 1.0);
        
        REQUIRE(results["ch1"].size() == 11); // 0,1,2,3,4,5,6,7,8,9,10
        REQUIRE(results["ch1"][0] == Catch::Approx(0.0));
        REQUIRE(results["ch1"][10] == Catch::Approx(100.0));
        
        // Verify intermediate values (linear interpolation)
        for (int i = 0; i <= 10; ++i) {
            // output the values to the Catch2 logs
            REQUIRE(results["ch1"][i] == Catch::Approx(i * 10.0));
        }
    }
    
    SECTION("Number of samples calculation") {
        REQUIRE(anim.num_samples(1.0) == 11);  // 0-10 range with rate 1.0
        REQUIRE(anim.num_samples(0.5) == 6);   // 0-10 range with rate 0.5
        REQUIRE(anim.num_samples(2.0) == 21);  // 0-10 range with rate 2.0
        
        REQUIRE_THROWS_AS(anim.num_samples(0.0), std::invalid_argument);
        REQUIRE_THROWS_AS(anim.num_samples(-1.0), std::invalid_argument);
    }
}
