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
    channel1.set_keyframe(1.0, 10.0, Point2D(0.9, 10.0), Point2D(1.1, 10.0), TangentMode::linear);
    channel1.set_keyframe(3.0, 30.0, Point2D(2.9, 30.0), Point2D(3.1, 30.0), TangentMode::linear);
    
    Channel channel2("position.y");
    channel2.set_keyframe(2.0, 20.0, Point2D(1.9, 20.0), Point2D(2.1, 20.0), TangentMode::linear);
    channel2.set_keyframe(4.0, 40.0, Point2D(3.9, 40.0), Point2D(4.1, 40.0), TangentMode::linear);
    
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
        REQUIRE(anim.get_channel_count() == 2);
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
        REQUIRE(anim.get_channel_count() == 1);
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
    channel.set_keyframe(1.0, 10.0, Point2D(0.9, 10.0), Point2D(1.1, 10.0), TangentMode::linear);
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

// Test insertion and ordering functions
TEST_CASE("Animation channel ordering", "[animation]") {
    Animation anim;
    
    Channel channel1("channel1");
    Channel channel2("channel2");
    Channel channel3("channel3");
    
    SECTION("Append channels") {
        anim.append_channel(channel1);
        anim.append_channel(channel2);
        anim.append_channel(channel3);
        
        REQUIRE(anim.get_channel_count() == 3);
        REQUIRE(anim.get_channel(0)->name() == "channel1");
        REQUIRE(anim.get_channel(1)->name() == "channel2");
        REQUIRE(anim.get_channel(2)->name() == "channel3");
    }
    
    SECTION("Insert channels") {
        anim.append_channel(channel1);  // First channel
        anim.append_channel(channel3);  // Second channel
        
        // Insert channel2 in the middle
        anim.insert_channel(1, channel2);
        
        REQUIRE(anim.get_channel_count() == 3);
        REQUIRE(anim.get_channel(0)->name() == "channel1");
        REQUIRE(anim.get_channel(1)->name() == "channel2");
        REQUIRE(anim.get_channel(2)->name() == "channel3");
    }
    
    SECTION("Insert at beginning") {
        anim.append_channel(channel2);
        anim.append_channel(channel3);
        
        // Insert at the beginning
        anim.insert_channel(0, channel1);
        
        REQUIRE(anim.get_channel_count() == 3);
        REQUIRE(anim.get_channel(0)->name() == "channel1");
        REQUIRE(anim.get_channel(1)->name() == "channel2");
        REQUIRE(anim.get_channel(2)->name() == "channel3");
    }
    
    SECTION("Find channel index") {
        anim.append_channel(channel1);
        anim.append_channel(channel2);
        anim.append_channel(channel3);
        
        REQUIRE(anim.find_channel_index("channel1") == 0);
        REQUIRE(anim.find_channel_index("channel2") == 1);
        REQUIRE(anim.find_channel_index("channel3") == 2);
        REQUIRE(anim.find_channel_index("nonexistent") == static_cast<size_t>(-1));
    }
}
