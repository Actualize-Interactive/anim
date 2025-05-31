#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/animation.hpp>
#include <anim/channel.hpp>


using namespace anim;

TEST_CASE("Animation Constructors and Name", "[Animation]") {
    SECTION("Default constructor") {
        Animation animation;
        REQUIRE(animation.name().empty());
        REQUIRE(animation.empty());
        REQUIRE(animation.num_channels() == 0);
        REQUIRE(animation.start_time() == Catch::Approx(0.0));
        REQUIRE(animation.end_time() == Catch::Approx(30.0));
    }

    SECTION("Named constructor") {
        Animation animation("TestAnim");
        REQUIRE(animation.name() == "TestAnim");
        REQUIRE(animation.empty());
        REQUIRE(animation.num_channels() == 0);
    }

    SECTION("Set and get name") {
        Animation animation;
        animation.set_name("NewName");
        REQUIRE(animation.name() == "NewName");
    }
}

TEST_CASE("Animation Channel Management", "[Animation]") {
    Animation animation("ChannelTestAnim");

    SECTION("Create channel") {
        Channel& ch1 = animation.create_channel("Chan1");
        REQUIRE(animation.num_channels() == 1);
        REQUIRE(animation.has_channel("Chan1"));
        REQUIRE(ch1.name() == "Chan1");
        REQUIRE(&animation.channel(0) == &ch1);
        REQUIRE(&animation.channel("Chan1") == &ch1);

        Channel& ch2 = animation.create_channel("Chan2", 0);
        REQUIRE(animation.num_channels() == 2);
        REQUIRE(animation.size() == 2);
        REQUIRE(animation.has_channel("Chan2"));
        REQUIRE(ch2.name() == "Chan2");
        REQUIRE(animation.channel(0).name() == "Chan2");
        REQUIRE(animation.channel(1).name() == "Chan1");

        REQUIRE_THROWS_AS(animation.create_channel("Chan3", 5), std::out_of_range);
    }    SECTION("Create channel variations") {
        Animation animation("VariationTest");
        Channel& ch = animation.create_channel("CreatedChan");
        REQUIRE(animation.num_channels() == 1);
        REQUIRE(animation.has_channel("CreatedChan"));
        REQUIRE(animation.channel(0).name() == "CreatedChan");
    }SECTION("Insert channel at position") {
        Channel& ch1 = animation.create_channel("FirstChan", 0);
        REQUIRE(animation.num_channels() == 1);
        REQUIRE(animation.size() == 1);
        REQUIRE(animation.has_channel("FirstChan"));
        REQUIRE(animation.channel(0).name() == "FirstChan");

        Channel& ch2 = animation.create_channel("SecondChan");
        REQUIRE(animation.num_channels() == 2);
        REQUIRE(animation.has_channel("SecondChan"));
        REQUIRE(animation.channel(1).name() == "SecondChan");

        REQUIRE_THROWS_AS(animation.create_channel("ThirdChan", 5), std::out_of_range);
    }

    SECTION("Access channels") {
        animation.create_channel("Ch1");
        animation.create_channel("Ch2");

        REQUIRE(animation.channel(0).name() == "Ch1");
        REQUIRE(animation[0].name() == "Ch1");
        REQUIRE(animation.channel("Ch2").name() == "Ch2");
        REQUIRE(animation["Ch2"].name() == "Ch2");

        const Animation& const_anim = animation;
        REQUIRE(const_anim.channel(0).name() == "Ch1");
        REQUIRE(const_anim[0].name() == "Ch1");
        REQUIRE(const_anim.channel("Ch2").name() == "Ch2");
        REQUIRE(const_anim["Ch2"].name() == "Ch2");

        REQUIRE_THROWS_AS(animation.channel(5), std::out_of_range);
        REQUIRE_THROWS_AS(animation.channel("NonExistent"), std::out_of_range);
        REQUIRE_THROWS_AS(const_anim.channel(5), std::out_of_range);
        REQUIRE_THROWS_AS(const_anim.channel("NonExistent"), std::out_of_range);
    }
    
    SECTION("Modify accessed channel") {
        animation.create_channel("ModCh");
        animation.channel("ModCh").create_keyframe(0.0, 1.0f);
        REQUIRE(animation.channel("ModCh").size() == 1);
        animation[0].create_keyframe(1.0, 2.0f);
        REQUIRE(animation[0].size() == 2);
    }


    SECTION("Channel information") {
        REQUIRE(animation.empty());
        REQUIRE(animation.size() == 0);
        REQUIRE(animation.num_channels() == 0);

        animation.create_channel("InfoCh1");
        animation.create_channel("InfoCh2");

        REQUIRE_FALSE(animation.empty());
        REQUIRE(animation.size() == 2);
        REQUIRE(animation.num_channels() == 2);
        REQUIRE(animation.has_channel("InfoCh1"));
        REQUIRE(animation.has_channel("InfoCh2"));
        REQUIRE_FALSE(animation.has_channel("NonExistent"));

        std::vector<std::string> names = animation.channel_names();
        REQUIRE(names.size() == 2);
        REQUIRE(std::find(names.begin(), names.end(), "InfoCh1") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "InfoCh2") != names.end());
    }

    SECTION("Remove channels") {
        animation.create_channel("RemCh1");
        animation.create_channel("RemCh2");
        animation.create_channel("RemCh3");
        REQUIRE(animation.num_channels() == 3);

        animation.remove_channel(1); // Removes RemCh2
        REQUIRE(animation.num_channels() == 2);
        REQUIRE_FALSE(animation.has_channel("RemCh2"));
        REQUIRE(animation.channel(0).name() == "RemCh1");
        REQUIRE(animation.channel(1).name() == "RemCh3");
        REQUIRE_THROWS_AS(animation.remove_channel(5), std::out_of_range);

        animation.remove_channel("RemCh1");
        REQUIRE(animation.num_channels() == 1);
        REQUIRE_FALSE(animation.has_channel("RemCh1"));
        REQUIRE(animation.channel(0).name() == "RemCh3");
        REQUIRE_THROWS_AS(animation.remove_channel("NonExistent"), std::out_of_range);

        animation.clear();
        REQUIRE(animation.empty());
        REQUIRE(animation.num_channels() == 0);
    }
    
    SECTION("Channels accessor") {        animation.create_channel("C1");
        animation.create_channel("C2");
        
        auto channels_ref = animation.channels();
        REQUIRE(channels_ref.size() == 2);
        channels_ref[0].get().set_name("NewC1");
        REQUIRE(animation.channel(0).name() == "NewC1");

        const Animation& const_anim = animation;
        auto const_channels_ref = const_anim.channels();
        REQUIRE(const_channels_ref.size() == 2);
        REQUIRE(const_channels_ref[0].get().name() == "NewC1");
    }

    SECTION("Reorder by index") {
        Channel& ch0 = animation.create_channel("channel0");
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        
        Id id0 = ch0.id();
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        
        // Initial order: ch0, ch1, ch2
        REQUIRE(animation.channel(0).name() == "channel0");
        REQUIRE(animation.channel(1).name() == "channel1");
        REQUIRE(animation.channel(2).name() == "channel2");
        
        // Move ch2 to position 0
        animation.reorder_channel(2, 0);
        
        // New order: ch2, ch0, ch1
        REQUIRE(animation.channel(0).name() == "channel2");
        REQUIRE(animation.channel(1).name() == "channel0");
        REQUIRE(animation.channel(2).name() == "channel1");
        
        // IDs should still work
        REQUIRE(animation.channel(id0) == &ch0);
        REQUIRE(animation.channel(id1) == &ch1);
        REQUIRE(animation.channel(id2) == &ch2);
    }
      SECTION("Reorder by name") {
        Channel& ch0 = animation.create_channel("channel0");
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        
        Id id0 = ch0.id();
        
        // Move channel0 to the end (position 2)
        animation.reorder_channel("channel0", 2);
        
        // New order: ch1, ch2, ch0
        REQUIRE(animation.channel(0).name() == "channel1");
        REQUIRE(animation.channel(1).name() == "channel2");
        REQUIRE(animation.channel(2).name() == "channel0");
        
        // ID access should still work
        REQUIRE(animation.channel(id0) == &ch0);
    }
    
    SECTION("Reorder by ID") {
        Channel& ch0 = animation.create_channel("channel0");
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        
        Id id1 = ch1.id();
        
        // Move channel1 to position 2
        animation.reorder_channel(id1, 2);
        
        // New order: ch0, ch2, ch1
        REQUIRE(animation.channel(0).name() == "channel0");
        REQUIRE(animation.channel(1).name() == "channel2");
        REQUIRE(animation.channel(2).name() == "channel1");
        
        // ID access should still work
        REQUIRE(animation.channel(id1) == &ch1);
    }

    SECTION("Reorder to current position") {
        Channel& ch0 = animation.create_channel("channel0");
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        
        Id id0 = ch0.id();
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        
        animation.reorder_channel("channel0", 0); // Reorder channel0 to its current position
        animation.reorder_channel(1, 1);
        animation.reorder_channel(id2, 2);
        
        // New order: ch0, ch1, ch2
        REQUIRE(animation.channel(0).name() == "channel0");
        REQUIRE(animation.channel(1).name() == "channel1");
        REQUIRE(animation.channel(2).name() == "channel2");
        
        // ID access should still work
        REQUIRE(animation.channel(id0) == &ch0);
        REQUIRE(animation.channel(id1) == &ch1);
        REQUIRE(animation.channel(id2) == &ch2);
    }

}

TEST_CASE("Animation Time Management", "[Animation]") {
    Animation animation;

    SECTION("Default times") {
        REQUIRE(animation.start_time() == Catch::Approx(0.0));
        REQUIRE(animation.end_time() == Catch::Approx(30.0));
        REQUIRE(animation.length() == Catch::Approx(30.0));
    }

    SECTION("Set start time") {
        animation.set_start_time(5.0);
        REQUIRE(animation.start_time() == Catch::Approx(5.0));
        REQUIRE(animation.end_time() == Catch::Approx(30.0)); // End time should remain
        REQUIRE(animation.length() == Catch::Approx(25.0));

        animation.set_start_time(35.0); // Should be capped by end_time
        REQUIRE(animation.start_time() == Catch::Approx(30.0));
        REQUIRE(animation.end_time() == Catch::Approx(30.0));
        REQUIRE(animation.length() == Catch::Approx(0.0));
    }

    SECTION("Set end time") {
        animation.set_start_time(0.0); // Reset for clarity
        animation.set_end_time(10.0);
        REQUIRE(animation.start_time() == Catch::Approx(0.0));
        REQUIRE(animation.end_time() == Catch::Approx(10.0));
        REQUIRE(animation.length() == Catch::Approx(10.0));

        animation.set_end_time(-5.0); // Should be capped by start_time
        REQUIRE(animation.start_time() == Catch::Approx(0.0));
        REQUIRE(animation.end_time() == Catch::Approx(0.0));
        REQUIRE(animation.length() == Catch::Approx(0.0));
    }

    SECTION("Set length") {
        animation.set_start_time(10.0);
        animation.set_end_time(20.0); // length is 10

        animation.set_length(15.0);
        REQUIRE(animation.start_time() == Catch::Approx(10.0));
        REQUIRE(animation.end_time() == Catch::Approx(25.0));
        REQUIRE(animation.length() == Catch::Approx(15.0));

        REQUIRE_THROWS_AS(animation.set_length(-5.0), std::invalid_argument);
    }
    
    SECTION("Length calculation with invalid times") {
        // This state should ideally not be reachable if setters are used correctly,
        // but testing the length() getter's robustness.
        // Manually setting private members is not possible here, so we rely on setters.
        animation.set_start_time(10.0);
        animation.set_end_time(5.0); // end_time will be capped to 10.0
        REQUIRE(animation.start_time() == Catch::Approx(10.0));
        REQUIRE(animation.end_time() == Catch::Approx(10.0));
        REQUIRE(animation.length() == Catch::Approx(0.0)); // Not throwing, as setters prevent invalid state.

        // To truly test the throw in length(), one would need to bypass setters or
        // temporarily modify the class to allow m_start_time > m_end_time.
        // For now, we assume setters correctly maintain m_start_time <= m_end_time.
        // If the design changes, this test might need adjustment.
    }
}

TEST_CASE("Animation Sample Calculation", "[Animation]") {
    Animation animation;
    animation.set_start_time(0.0);
    animation.set_end_time(1.0); // Length = 1.0

    SECTION("No channels") {
        REQUIRE(animation.num_samples(10.0) == 0);
    }

    animation.create_channel("SampleChan");

    SECTION("Valid sample rate") {
        // length = 1.0, sample_rate = 10.0
        // samples = ceil(1.0 * 10.0) + 1 = 10 + 1 = 11
        REQUIRE(animation.num_samples(10.0) == 11);

        // length = 1.0, sample_rate = 1.0
        // samples = ceil(1.0 * 1.0) + 1 = 1 + 1 = 2
        REQUIRE(animation.num_samples(1.0) == 2);

        animation.set_end_time(0.9); // Length = 0.9
        // samples = ceil(0.9 * 10.0) + 1 = ceil(9.0) + 1 = 9 + 1 = 10
        REQUIRE(animation.num_samples(10.0) == 10);
        
        animation.set_end_time(0.95); // Length = 0.95
        // samples = ceil(0.95 * 10.0) + 1 = ceil(9.5) + 1 = 10 + 1 = 11
        REQUIRE(animation.num_samples(10.0) == 11);
    }
    
    SECTION("Zero length animation") {
        animation.set_end_time(0.0); // Length = 0.0
        // samples = ceil(0.0 * 10.0) + 1 = 0 + 1 = 1
        REQUIRE(animation.num_samples(10.0) == 1);
    }

    SECTION("Invalid sample rate") {
        REQUIRE_THROWS_AS(animation.num_samples(0.0), std::invalid_argument);
        REQUIRE_THROWS_AS(animation.num_samples(-10.0), std::invalid_argument);
    }
}

TEST_CASE("Animation API Comprehensive Test", "[Animation]") {
    SECTION("Complete Animation API functionality test") {
        // Create a test animation
        Animation animation("test_animation");
        
        // Test initial animation state
        REQUIRE(animation.name() == "test_animation");
        REQUIRE(animation.num_channels() == 0);
        REQUIRE(animation.empty());
        
        // Test animation name setter
        animation.set_name("renamed_animation");
        REQUIRE(animation.name() == "renamed_animation");
        
        // Test channel creation methods
        Channel& ch1 = animation.create_channel("pos_x");
        REQUIRE(ch1.name() == "pos_x");
        REQUIRE(animation.num_channels() == 1);
        REQUIRE_FALSE(animation.empty());
        
        // Test channel access by index and name
        REQUIRE(&animation.channel(0) == &ch1);
        REQUIRE(&animation.channel("pos_x") == &ch1);
        REQUIRE(&animation[0] == &ch1);
        REQUIRE(&animation["pos_x"] == &ch1);
        
        // Test channel creation with insertion at specific index
        Channel& ch2 = animation.create_channel("pos_y", 0);
        REQUIRE(ch2.name() == "pos_y");
        REQUIRE(animation.num_channels() == 2);
        REQUIRE(animation.channel(0).name() == "pos_y");
        REQUIRE(animation.channel(1).name() == "pos_x");
          // Test create additional channel
        animation.create_channel("rotation");
        REQUIRE(animation.num_channels() == 3);
        REQUIRE(animation.has_channel("rotation"));
        
        // Test create channel at position
        animation.create_channel("scale", 1);
        REQUIRE(animation.num_channels() == 4);
        REQUIRE(animation.channel(1).name() == "scale");
        
        // Test channel queries
        REQUIRE(animation.has_channel("pos_x"));
        REQUIRE(animation.has_channel("pos_y"));
        REQUIRE(animation.has_channel("rotation"));
        REQUIRE(animation.has_channel("scale"));
        REQUIRE_FALSE(animation.has_channel("non_existent"));
        
        // Test channel names
        auto names = animation.channel_names();
        REQUIRE(names.size() == 4);
        REQUIRE(std::find(names.begin(), names.end(), "pos_x") != names.end());        REQUIRE(std::find(names.begin(), names.end(), "pos_y") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "rotation") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "scale") != names.end());
        
        // Test channels accessor
        auto channels_ref = animation.channels();
        REQUIRE(channels_ref.size() == 4);
        
        const Animation& const_animation = animation;
        auto const_channels_ref = const_animation.channels();
        REQUIRE(const_channels_ref.size() == 4);
        
        // Test adding keyframes to channels through animation
        animation.channel("pos_x").create_keyframe(0.0, 0.0);
        animation.channel("pos_x").create_keyframe(1.0, 10.0);
        animation["pos_y"].create_keyframe(0.0, 0.0);
        animation["pos_y"].create_keyframe(1.0, 5.0);
        
        REQUIRE(animation.channel("pos_x").num_keyframes() == 2);
        REQUIRE(animation.channel("pos_y").num_keyframes() == 2);
        
        // Test animation timing properties
        animation.set_start_time(0.0);
        animation.set_end_time(2.0);
        REQUIRE(animation.start_time() == Catch::Approx(0.0));
        REQUIRE(animation.end_time() == Catch::Approx(2.0));
        REQUIRE(animation.length() == Catch::Approx(2.0));
        
        // Test setting length
        animation.set_length(3.0);
        REQUIRE(animation.length() == Catch::Approx(3.0));
        REQUIRE(animation.end_time() == Catch::Approx(3.0));
        
        // Test num_samples calculation
        size_t num_samples = animation.num_samples(30.0);
        REQUIRE(num_samples > 0);
        
        // Test channel removal
        animation.remove_channel("scale");
        REQUIRE(animation.num_channels() == 3);
        REQUIRE_FALSE(animation.has_channel("scale"));
        
        animation.remove_channel(0); // Remove first channel (pos_y)
        REQUIRE(animation.num_channels() == 2);
        REQUIRE(animation.channel(0).name() == "pos_x");
        
        // Test clear
        animation.clear();
        REQUIRE(animation.empty());
        REQUIRE(animation.num_channels() == 0);
    }
    
    SECTION("Animation API error conditions") {
        Animation animation("error_test");
        
        // Test errors on empty animation
        REQUIRE_THROWS_AS(animation.channel(0), std::out_of_range);
        REQUIRE_THROWS_AS(animation[0], std::out_of_range);
        REQUIRE_THROWS_AS(animation.channel("non_existent"), std::out_of_range);
        REQUIRE_THROWS_AS(animation["non_existent"], std::out_of_range);
        REQUIRE_THROWS_AS(animation.remove_channel(0), std::out_of_range);
        REQUIRE_THROWS_AS(animation.remove_channel("non_existent"), std::out_of_range);
        
        // Add some channels for further error testing
        animation.create_channel("test1");
        animation.create_channel("test2");
        
        // Test errors with invalid indices
        REQUIRE_THROWS_AS(animation.channel(5), std::out_of_range);
        REQUIRE_THROWS_AS(animation[5], std::out_of_range);        REQUIRE_THROWS_AS(animation.remove_channel(5), std::out_of_range);
        REQUIRE_THROWS_AS(animation.create_channel("test3", 5), std::out_of_range);
        REQUIRE_THROWS_AS(animation.create_channel("test4", 5), std::out_of_range);
        
        // Test timing errors
        REQUIRE_THROWS_AS(animation.set_length(-1.0), std::invalid_argument);
        REQUIRE_THROWS_AS(animation.num_samples(0.0), std::invalid_argument);
        REQUIRE_THROWS_AS(animation.num_samples(-1.0), std::invalid_argument);
    }
    
    SECTION("Animation timing edge cases") {
        Animation animation("timing_test");
        
        // Test start time greater than end time gets clamped
        animation.set_start_time(10.0);
        animation.set_end_time(5.0); // Should be clamped to start_time
        REQUIRE(animation.start_time() == Catch::Approx(10.0));
        REQUIRE(animation.end_time() == Catch::Approx(10.0));
        REQUIRE(animation.length() == Catch::Approx(0.0));
        
        // Test end time less than start time gets clamped
        animation.set_end_time(20.0);
        animation.set_start_time(25.0); // Should be clamped to end_time
        REQUIRE(animation.start_time() == Catch::Approx(20.0));
        REQUIRE(animation.end_time() == Catch::Approx(20.0));
        REQUIRE(animation.length() == Catch::Approx(0.0));
        
        // Test zero length animation samples
        animation.create_channel("test_channel");
        REQUIRE(animation.num_samples(30.0) == 1); // Zero length + 1 = 1 sample
        
        // Test normal timing
        animation.set_start_time(0.0);
        animation.set_end_time(1.0);
        REQUIRE(animation.num_samples(30.0) == 31); // 1 second at 30fps + 1 = 31 samples
    }
    
    SECTION("Animation with no channels edge cases") {
        Animation animation("no_channels");
        
        // Test num_samples with no channels
        REQUIRE(animation.num_samples(30.0) == 0);
        
        // Test timing still works
        animation.set_start_time(5.0);
        animation.set_end_time(10.0);
        REQUIRE(animation.length() == Catch::Approx(5.0));
        REQUIRE(animation.num_samples(30.0) == 0); // Still 0 because no channels
        
        // After adding a channel, samples should be calculated
        animation.create_channel("new_channel");
        REQUIRE(animation.num_samples(30.0) == 151); // 5 seconds at 30fps + 1 = 151 samples
    }
    
    SECTION("Animation const correctness") {
        Animation animation("const_test");
        animation.create_channel("ch1");
        animation.create_channel("ch2");
        
        const Animation& const_anim = animation;
        
        // Test const channel access
        REQUIRE(const_anim.channel(0).name() == "ch1");
        REQUIRE(const_anim[0].name() == "ch1");
        REQUIRE(const_anim.channel("ch2").name() == "ch2");
        REQUIRE(const_anim["ch2"].name() == "ch2");
        
        // Test const properties
        REQUIRE(const_anim.num_channels() == 2);
        REQUIRE(const_anim.size() == 2);
        REQUIRE_FALSE(const_anim.empty());
        REQUIRE(const_anim.has_channel("ch1"));
          auto const_names = const_anim.channel_names();
        REQUIRE(const_names.size() == 2);
        
        auto const_channels = const_anim.channels();
        REQUIRE(const_channels.size() == 2);
    }
}

