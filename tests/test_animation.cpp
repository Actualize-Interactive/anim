#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/animation.hpp>
#include <anim/channel.hpp>
#include <memory>
#include <vector>


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
        
        auto& channels = animation.channels();
        REQUIRE(channels.size() == 2);
        channels[0].get()->set_name("NewC1");
        REQUIRE(animation.channel(0).name() == "NewC1");

        const Animation& const_anim = animation;
        auto& const_channels = const_anim.channels();
        REQUIRE(const_channels.size() == 2);
        REQUIRE(const_channels[0].get()->name() == "NewC1");
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
        auto& channels = animation.channels();
        REQUIRE(channels.size() == 4);
        
        const Animation& const_animation = animation;
        auto& const_channels = const_animation.channels();
        REQUIRE(const_channels.size() == 4);
        
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
        
        auto& const_channels = const_anim.channels();
        REQUIRE(const_channels.size() == 2);
    }
}

TEST_CASE("Animation Copy Functionality", "[Animation][Copy]") {
    SECTION("Copy empty animation") {
        Animation original("TestAnimation");
        original.set_start_time(5.0);
        original.set_end_time(25.0);
        
        Animation copied = original.copy();
        
        REQUIRE(copied.name() == "TestAnimation_copy");
        REQUIRE(copied.start_time() == Catch::Approx(5.0));
        REQUIRE(copied.end_time() == Catch::Approx(25.0));
        REQUIRE(copied.empty());
        REQUIRE(copied.num_channels() == 0);
    }
    
    SECTION("Copy empty animation with custom name") {
        Animation original("TestAnimation");
        
        Animation copied = original.copy("CustomCopyName");
        
        REQUIRE(copied.name() == "CustomCopyName");
        REQUIRE(copied.start_time() == original.start_time());
        REQUIRE(copied.end_time() == original.end_time());
        REQUIRE(copied.empty());
    }
    
    SECTION("Copy animation with channels") {
        Animation original("AnimWithChannels");
        original.set_start_time(1.0);
        original.set_end_time(10.0);
        
        // Create channels with keyframes
        Channel& ch1 = original.create_channel("Channel1");
        ch1.create_keyframe(0.0, 0.0);
        ch1.create_keyframe(5.0, 10.0);
        
        Channel& ch2 = original.create_channel("Channel2");
        ch2.create_keyframe(2.0, 5.0);
        ch2.create_keyframe(8.0, 15.0);
        
        Animation copied = original.copy();
        
        // Verify basic properties
        REQUIRE(copied.name() == "AnimWithChannels_copy");
        REQUIRE(copied.start_time() == Catch::Approx(1.0));
        REQUIRE(copied.end_time() == Catch::Approx(10.0));
        REQUIRE(copied.num_channels() == 2);
        
        // Verify channels exist with same names
        REQUIRE(copied.has_channel("Channel1"));
        REQUIRE(copied.has_channel("Channel2"));
        
        // Verify channel content
        const Channel& copied_ch1 = copied.channel("Channel1");
        const Channel& copied_ch2 = copied.channel("Channel2");
        
        REQUIRE(copied_ch1.num_keyframes() == 2);
        REQUIRE(copied_ch2.num_keyframes() == 2);
        
        // Verify keyframe data
        REQUIRE(copied_ch1.keyframe(0).time() == Catch::Approx(0.0));
        REQUIRE(copied_ch1.keyframe(0).value() == Catch::Approx(0.0));
        REQUIRE(copied_ch1.keyframe(1).time() == Catch::Approx(5.0));
        REQUIRE(copied_ch1.keyframe(1).value() == Catch::Approx(10.0));
        
        REQUIRE(copied_ch2.keyframe(0).time() == Catch::Approx(2.0));
        REQUIRE(copied_ch2.keyframe(0).value() == Catch::Approx(5.0));
        REQUIRE(copied_ch2.keyframe(1).time() == Catch::Approx(8.0));
        REQUIRE(copied_ch2.keyframe(1).value() == Catch::Approx(15.0));
    }
    
    SECTION("Copy preserves channel independence") {
        Animation original("Original");
        Channel& orig_ch = original.create_channel("TestChannel");
        orig_ch.create_keyframe(0.0, 0.0);
        
        Animation copied = original.copy("Copy");
        
        // Verify channels have different IDs (thus are independent)
        const Channel& orig_channel = original.channel("TestChannel");
        const Channel& copy_channel = copied.channel("TestChannel");
        
        REQUIRE(orig_channel.id() != copy_channel.id());
        
        // Modify original and verify copy is unaffected
        original.create_channel("NewChannel");
        REQUIRE(original.num_channels() == 2);
        REQUIRE(copied.num_channels() == 1);
        
        // Modify original channel and verify copy is unaffected
        orig_ch.create_keyframe(1.0, 1.0);
        REQUIRE(orig_channel.num_keyframes() == 2);
        REQUIRE(copy_channel.num_keyframes() == 1);
    }
    
    SECTION("Copy maintains channel order") {
        Animation original("OrderTest");
        original.create_channel("First");
        original.create_channel("Second");
        original.create_channel("Third");
        
        Animation copied = original.copy();
        
        REQUIRE(copied.num_channels() == 3);
        REQUIRE(copied.channel(0).name() == "First");
        REQUIRE(copied.channel(1).name() == "Second");
        REQUIRE(copied.channel(2).name() == "Third");
    }
}

TEST_CASE("Animation Deep Copy Validation with Smart Pointers", "[Animation][Copy][DeepCopy]") {
    SECTION("Deep copy with unique_ptr - basic validation") {
        // Create original animation in unique_ptr
        auto original = std::make_unique<Animation>("OriginalAnim");
        original->set_start_time(1.0);
        original->set_end_time(10.0);
        
        Channel& orig_ch = original->create_channel("TestChannel");
        orig_ch.create_keyframe(0.0, 5.0);
        orig_ch.create_keyframe(2.0, 15.0);
        
        // Copy to new unique_ptr
        auto copied = std::make_unique<Animation>(original->copy("CopiedAnim"));
        
        // Verify basic properties are copied correctly
        REQUIRE(copied->name() == "CopiedAnim");
        REQUIRE(copied->start_time() == Catch::Approx(1.0));
        REQUIRE(copied->end_time() == Catch::Approx(10.0));
        REQUIRE(copied->num_channels() == 1);
        REQUIRE(copied->has_channel("TestChannel"));
        
        // Verify channels have different IDs (independence)
        const Channel& orig_channel = original->channel("TestChannel");
        const Channel& copy_channel = copied->channel("TestChannel");
        REQUIRE(orig_channel.id() != copy_channel.id());
        
        // Verify keyframe data is copied correctly
        REQUIRE(copy_channel.num_keyframes() == 2);
        REQUIRE(copy_channel.keyframe(0).time() == Catch::Approx(0.0));
        REQUIRE(copy_channel.keyframe(0).value() == Catch::Approx(5.0));
        REQUIRE(copy_channel.keyframe(1).time() == Catch::Approx(2.0));
        REQUIRE(copy_channel.keyframe(1).value() == Catch::Approx(15.0));
        
        // Test independence - modify original and verify copy is unaffected
        original->create_channel("NewChannel");
        orig_ch.create_keyframe(4.0, 25.0);
        original->set_start_time(0.5);
        
        // Copy should remain unchanged
        REQUIRE(copied->num_channels() == 1);
        REQUIRE(copy_channel.num_keyframes() == 2);
        REQUIRE(copied->start_time() == Catch::Approx(1.0));
        REQUIRE_FALSE(copied->has_channel("NewChannel"));
        
        // Test independence - modify copy and verify original is unaffected
        copied->create_channel("CopyOnlyChannel");
        Channel& copy_test_ch = copied->channel("TestChannel");
        copy_test_ch.create_keyframe(6.0, 35.0);
        copied->set_end_time(20.0);
        
        // Original should remain unchanged
        REQUIRE(original->num_channels() == 2);
        REQUIRE(orig_channel.num_keyframes() == 3);
        REQUIRE(original->end_time() == Catch::Approx(10.0));
        REQUIRE_FALSE(original->has_channel("CopyOnlyChannel"));
    }
    
    SECTION("Deep copy with vector of unique_ptr") {
        // Create vector of animations
        std::vector<std::unique_ptr<Animation>> original_anims;
        
        // Create first animation
        auto anim1 = std::make_unique<Animation>("Anim1");
        anim1->set_start_time(0.0);
        anim1->set_end_time(5.0);
        Channel& ch1 = anim1->create_channel("Channel1");
        ch1.create_keyframe(0.0, 10.0);
        ch1.create_keyframe(1.0, 20.0);
        
        // Create second animation
        auto anim2 = std::make_unique<Animation>("Anim2");
        anim2->set_start_time(5.0);
        anim2->set_end_time(15.0);
        Channel& ch2 = anim2->create_channel("Channel2");
        ch2.create_keyframe(2.0, 30.0);
        ch2.create_keyframe(3.0, 40.0);
        Channel& ch2_extra = anim2->create_channel("ExtraChannel");
        ch2_extra.create_keyframe(1.0, 100.0);
        
        original_anims.push_back(std::move(anim1));
        original_anims.push_back(std::move(anim2));
        
        // Create vector of copied animations
        std::vector<std::unique_ptr<Animation>> copied_anims;
        for (const auto& orig : original_anims) {
            copied_anims.push_back(std::make_unique<Animation>(orig->copy(orig->name() + "_copy")));
        }
        
        // Verify copies are correct
        REQUIRE(copied_anims.size() == 2);
        
        // Verify first animation copy
        const auto& copy1 = copied_anims[0];
        REQUIRE(copy1->name() == "Anim1_copy");
        REQUIRE(copy1->start_time() == Catch::Approx(0.0));
        REQUIRE(copy1->end_time() == Catch::Approx(5.0));
        REQUIRE(copy1->num_channels() == 1);
        const Channel& copy_ch1 = copy1->channel("Channel1");
        REQUIRE(copy_ch1.num_keyframes() == 2);
        REQUIRE(copy_ch1.keyframe(0).time() == Catch::Approx(0.0));
        REQUIRE(copy_ch1.keyframe(0).value() == Catch::Approx(10.0));
        
        // Verify second animation copy
        const auto& copy2 = copied_anims[1];
        REQUIRE(copy2->name() == "Anim2_copy");
        REQUIRE(copy2->start_time() == Catch::Approx(5.0));
        REQUIRE(copy2->end_time() == Catch::Approx(15.0));
        REQUIRE(copy2->num_channels() == 2);
        const Channel& copy_ch2 = copy2->channel("Channel2");
        const Channel& copy_ch2_extra = copy2->channel("ExtraChannel");
        REQUIRE(copy_ch2.num_keyframes() == 2);
        REQUIRE(copy_ch2_extra.num_keyframes() == 1);
        
        // Verify channel independence by checking IDs
        const Channel& orig_ch1 = original_anims[0]->channel("Channel1");
        const Channel& orig_ch2 = original_anims[1]->channel("Channel2");
        const Channel& orig_ch2_extra = original_anims[1]->channel("ExtraChannel");
        
        REQUIRE(orig_ch1.id() != copy_ch1.id());
        REQUIRE(orig_ch2.id() != copy_ch2.id());
        REQUIRE(orig_ch2_extra.id() != copy_ch2_extra.id());
        
        // Test independence - modify originals and verify copies are unaffected
        original_anims[0]->create_channel("NewChannelOrig1");
        Channel& orig_modify_ch = original_anims[1]->channel("Channel2");
        orig_modify_ch.create_keyframe(4.0, 50.0);
        
        REQUIRE(copy1->num_channels() == 1);
        REQUIRE_FALSE(copy1->has_channel("NewChannelOrig1"));
        REQUIRE(copy_ch2.num_keyframes() == 2);
        
        // Test independence - modify copies and verify originals are unaffected
        copied_anims[0]->create_channel("NewChannelCopy1");
        Channel& copy_modify_ch = copied_anims[1]->channel("Channel2");
        copy_modify_ch.create_keyframe(5.0, 60.0);
        
        REQUIRE(original_anims[0]->num_channels() == 2);
        REQUIRE_FALSE(original_anims[0]->has_channel("NewChannelCopy1"));
        REQUIRE(orig_modify_ch.num_keyframes() == 3);
    }
    
    SECTION("Deep copy with std::move operations") {
        // Create original animation
        Animation original("MoveTest");
        original.set_start_time(2.0);
        original.set_end_time(8.0);
        Channel& orig_ch = original.create_channel("MoveChannel");
        orig_ch.create_keyframe(0.0, 100.0);
        orig_ch.create_keyframe(1.0, 200.0);
        
        // Create copy and store in unique_ptr
        auto copied_ptr = std::make_unique<Animation>(original.copy("MovedCopy"));
        
        // Move to vector
        std::vector<std::unique_ptr<Animation>> anim_vector;
        anim_vector.push_back(std::move(copied_ptr));
        
        // Verify the move worked and data is intact
        REQUIRE(anim_vector.size() == 1);
        REQUIRE(copied_ptr == nullptr); // Should be moved from
        
        const auto& moved_anim = anim_vector[0];
        REQUIRE(moved_anim->name() == "MovedCopy");
        REQUIRE(moved_anim->start_time() == Catch::Approx(2.0));
        REQUIRE(moved_anim->end_time() == Catch::Approx(8.0));
        REQUIRE(moved_anim->num_channels() == 1);
        
        const Channel& moved_ch = moved_anim->channel("MoveChannel");
        REQUIRE(moved_ch.num_keyframes() == 2);
        REQUIRE(moved_ch.keyframe(0).time() == Catch::Approx(0.0));
        REQUIRE(moved_ch.keyframe(0).value() == Catch::Approx(100.0));
        REQUIRE(moved_ch.keyframe(1).time() == Catch::Approx(1.0));
        REQUIRE(moved_ch.keyframe(1).value() == Catch::Approx(200.0));
        
        // Verify independence - original should still be independent
        const Channel& orig_channel = original.channel("MoveChannel");
        REQUIRE(orig_channel.id() != moved_ch.id());
        
        // Modify original and verify moved copy is unaffected
        original.create_channel("OriginalOnlyChannel");
        orig_ch.create_keyframe(2.0, 300.0);
        
        REQUIRE(moved_anim->num_channels() == 1);
        REQUIRE(moved_ch.num_keyframes() == 2);
        REQUIRE_FALSE(moved_anim->has_channel("OriginalOnlyChannel"));
        
        // Move back to single unique_ptr
        auto single_ptr = std::move(anim_vector[0]);
        anim_vector.clear();
        
        // Verify data is still intact after multiple moves
        REQUIRE(single_ptr->name() == "MovedCopy");
        REQUIRE(single_ptr->start_time() == Catch::Approx(2.0));
        REQUIRE(single_ptr->num_channels() == 1);
        
        const Channel& final_ch = single_ptr->channel("MoveChannel");
        REQUIRE(final_ch.num_keyframes() == 2);
        REQUIRE(final_ch.keyframe(0).value() == Catch::Approx(100.0));
        REQUIRE(final_ch.keyframe(1).value() == Catch::Approx(200.0));
        
        // Test moving between vectors
        std::vector<std::unique_ptr<Animation>> vector1;
        std::vector<std::unique_ptr<Animation>> vector2;
        
        vector1.push_back(std::move(single_ptr));
        
        // Move from vector1 to vector2
        vector2.push_back(std::move(vector1[0]));
        vector1.clear();
        
        // Verify data integrity after vector-to-vector move
        REQUIRE(vector2.size() == 1);
        const auto& final_anim = vector2[0];
        REQUIRE(final_anim->name() == "MovedCopy");
        REQUIRE(final_anim->num_channels() == 1);
        const Channel& final_moved_ch = final_anim->channel("MoveChannel");
        REQUIRE(final_moved_ch.num_keyframes() == 2);
        REQUIRE(final_moved_ch.keyframe(0).value() == Catch::Approx(100.0));
        
        // Verify independence is still maintained
        REQUIRE(orig_channel.id() != final_moved_ch.id());
    }
    
    SECTION("Deep copy keyframe data independence") {
        // Create original with detailed keyframe data
        Animation original("KeyframeTest");
        Channel& orig_ch = original.create_channel("TestChannel");
        
        // Create keyframes
        const auto& kf1 = orig_ch.create_keyframe(0.0, 10.0);
        const auto& kf2 = orig_ch.create_keyframe(2.0, 20.0);
        const auto& kf3 = orig_ch.create_keyframe(4.0, 30.0);
        
        // Copy animation
        auto copied_ptr = std::make_unique<Animation>(original.copy("KeyframeCopy"));
        const Channel& copy_ch = copied_ptr->channel("TestChannel");
        
        // Verify keyframe data is copied correctly
        REQUIRE(copy_ch.num_keyframes() == 3);
        
        const auto& copy_kf1 = copy_ch.keyframe(0);
        const auto& copy_kf2 = copy_ch.keyframe(1);
        const auto& copy_kf3 = copy_ch.keyframe(2);
        
        REQUIRE(copy_kf1.time() == Catch::Approx(0.0));
        REQUIRE(copy_kf1.value() == Catch::Approx(10.0));
        REQUIRE(copy_kf2.time() == Catch::Approx(2.0));
        REQUIRE(copy_kf2.value() == Catch::Approx(20.0));
        REQUIRE(copy_kf3.time() == Catch::Approx(4.0));
        REQUIRE(copy_kf3.value() == Catch::Approx(30.0));
        
        // Verify channel IDs are different (independence)
        REQUIRE(orig_ch.id() != copy_ch.id());
        
        // Test independence - modify original keyframes and verify copy is unaffected
        orig_ch.set_keyframe_position(0, Point(0.1, 11.0));
        orig_ch.create_keyframe(3.0, 25.0);  // Add new keyframe
        
        // Copy should remain unchanged
        const auto& copy_kf1_after = copy_ch.keyframe(0);
        REQUIRE(copy_kf1_after.time() == Catch::Approx(0.0));
        REQUIRE(copy_kf1_after.value() == Catch::Approx(10.0));
        REQUIRE(copy_ch.num_keyframes() == 3);  // Should still have 3 keyframes
        
        // Test independence - modify copy keyframes and verify original is unaffected  
        Channel& copy_modify_ch = copied_ptr->channel("TestChannel");
        copy_modify_ch.set_keyframe_position(1, Point(2.1, 21.0));
        copy_modify_ch.create_keyframe(5.0, 35.0);  // Add new keyframe
        
        // Original should remain unchanged (except for our earlier modification)
        const auto& orig_kf1_after = orig_ch.keyframe(0);
        const auto& orig_kf2_after = orig_ch.keyframe(1);
        REQUIRE(orig_kf1_after.time() == Catch::Approx(0.1));  // Our modification
        REQUIRE(orig_kf1_after.value() == Catch::Approx(11.0));  // Our modification
        REQUIRE(orig_kf2_after.time() == Catch::Approx(2.0));  // Should be unchanged
        REQUIRE(orig_kf2_after.value() == Catch::Approx(20.0));  // Should be unchanged
        REQUIRE(orig_ch.num_keyframes() == 4);  // Should have 4 keyframes (3 original + 1 added)
    }
}

TEST_CASE("Animation Equality Operators", "[Animation][equality]") {
    SECTION("Empty animations with same settings are equal") {
        Animation anim1("TestAnim");
        Animation anim2("TestAnim");
        
        REQUIRE(anim1 == anim2);
        REQUIRE_FALSE(anim1 != anim2);
    }

    SECTION("Empty animations with different names are not equal") {
        Animation anim1("TestAnim1");
        Animation anim2("TestAnim2");
        
        REQUIRE_FALSE(anim1 == anim2);
        REQUIRE(anim1 != anim2);
    }

    SECTION("Animations with different start times are not equal") {
        Animation anim1("TestAnim");
        Animation anim2("TestAnim");
        
        anim1.set_start_time(5.0);
        anim2.set_start_time(10.0);
        
        REQUIRE_FALSE(anim1 == anim2);
        REQUIRE(anim1 != anim2);
    }

    SECTION("Animations with different end times are not equal") {
        Animation anim1("TestAnim");
        Animation anim2("TestAnim");
        
        anim1.set_end_time(25.0);
        anim2.set_end_time(30.0);
        
        REQUIRE_FALSE(anim1 == anim2);
        REQUIRE(anim1 != anim2);
    }

    SECTION("Animations with same channels are equal") {
        Animation anim1("TestAnim");
        Animation anim2("TestAnim");
        
        anim1.create_channel("Ch1").create_keyframe(1.0, 10.0);
        anim1.create_channel("Ch2").create_keyframe(2.0, 20.0);
        
        anim2.create_channel("Ch1").create_keyframe(1.0, 10.0);
        anim2.create_channel("Ch2").create_keyframe(2.0, 20.0);
        
        REQUIRE(anim1 == anim2);
        REQUIRE_FALSE(anim1 != anim2);
    }

    SECTION("Animations with different number of channels are not equal") {
        Animation anim1("TestAnim");
        Animation anim2("TestAnim");
        
        anim1.create_channel("Ch1");
        anim2.create_channel("Ch1");
        anim2.create_channel("Ch2");
        
        REQUIRE_FALSE(anim1 == anim2);
        REQUIRE(anim1 != anim2);
    }

    SECTION("Animations with different channel order are not equal") {
        Animation anim1("TestAnim");
        Animation anim2("TestAnim");
        
        anim1.create_channel("Ch1");
        anim1.create_channel("Ch2");
        
        anim2.create_channel("Ch2");
        anim2.create_channel("Ch1");
        
        REQUIRE_FALSE(anim1 == anim2);
        REQUIRE(anim1 != anim2);
    }

    SECTION("Animations with channels having different keyframes are not equal") {
        Animation anim1("TestAnim");
        Animation anim2("TestAnim");
        
        anim1.create_channel("Ch1").create_keyframe(1.0, 10.0);
        anim2.create_channel("Ch1").create_keyframe(1.0, 15.0); // Different value
        
        REQUIRE_FALSE(anim1 == anim2);
        REQUIRE(anim1 != anim2);
    }

    SECTION("Copied animations are equal") {
        Animation original("OriginalAnim");
        original.set_start_time(5.0);
        original.set_end_time(25.0);
        original.create_channel("Ch1").create_keyframe(1.0, 10.0);
        original.create_channel("Ch2").create_keyframe(2.0, 20.0);
        
        Animation copied = original.copy();
        
        // copy() creates a name with "_copy" suffix, so they won't be equal by default
        REQUIRE_FALSE(original == copied);
        REQUIRE(original != copied);
        
        // But if we set the same name, they should be equal
        copied.set_name("OriginalAnim");
        REQUIRE(original == copied);
        REQUIRE_FALSE(original != copied);
    }

    SECTION("Copied animations with different names can be equal except for name") {
        Animation original("OriginalAnim");
        original.set_start_time(5.0);
        original.set_end_time(25.0);
        original.create_channel("Ch1").create_keyframe(1.0, 10.0);
        
        Animation copied = original.copy("CopiedAnim");
        
        // Should not be equal due to different names
        REQUIRE_FALSE(original == copied);
        REQUIRE(original != copied);
        
        // But if we set the same name, they should be equal
        copied.set_name("OriginalAnim");
        REQUIRE(original == copied);
        REQUIRE_FALSE(original != copied);
    }

    SECTION("Complex animations with multiple channels and keyframes") {
        Animation anim1("ComplexAnim");
        Animation anim2("ComplexAnim");
        
        anim1.set_start_time(2.0);
        anim1.set_end_time(20.0);
        
        anim2.set_start_time(2.0);
        anim2.set_end_time(20.0);
        
        // Add complex channels with multiple keyframes
        Channel& ch1_1 = anim1.create_channel("Position");
        ch1_1.create_keyframe(0.0, 0.0);
        ch1_1.create_keyframe(5.0, 50.0);
        ch1_1.create_keyframe(10.0, 100.0);
        
        Channel& ch1_2 = anim1.create_channel("Rotation");
        ch1_2.create_keyframe(0.0, 0.0, Function::linear);
        ch1_2.create_keyframe(10.0, 360.0, Function::linear);
        
        Channel& ch2_1 = anim2.create_channel("Position");
        ch2_1.create_keyframe(0.0, 0.0);
        ch2_1.create_keyframe(5.0, 50.0);
        ch2_1.create_keyframe(10.0, 100.0);
        
        Channel& ch2_2 = anim2.create_channel("Rotation");
        ch2_2.create_keyframe(0.0, 0.0, Function::linear);
        ch2_2.create_keyframe(10.0, 360.0, Function::linear);
        
        REQUIRE(anim1 == anim2);
        REQUIRE_FALSE(anim1 != anim2);
        
        // Modify one keyframe and verify they're no longer equal
        ch2_1.set_keyframe_value(1, 55.0); // Change middle keyframe value
        
        REQUIRE_FALSE(anim1 == anim2);
        REQUIRE(anim1 != anim2);
    }
}

