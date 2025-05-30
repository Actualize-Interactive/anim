#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/animation.hpp>
#include <anim/channel.hpp>


using namespace anim;

TEST_CASE("Animation Constructors and Name", "[Animation]") {
    SECTION("Default constructor") {
        Animation anim;
        REQUIRE(anim.name().empty());
        REQUIRE(anim.empty());
        REQUIRE(anim.num_channels() == 0);
        REQUIRE(anim.start_time() == Catch::Approx(0.0));
        REQUIRE(anim.end_time() == Catch::Approx(30.0));
    }

    SECTION("Named constructor") {
        Animation anim("TestAnim");
        REQUIRE(anim.name() == "TestAnim");
        REQUIRE(anim.empty());
        REQUIRE(anim.num_channels() == 0);
    }

    SECTION("Set and get name") {
        Animation anim;
        anim.set_name("NewName");
        REQUIRE(anim.name() == "NewName");
    }
}

TEST_CASE("Animation Channel Management", "[Animation]") {
    Animation anim("ChannelTestAnim");

    SECTION("Create channel") {
        Channel& ch1 = anim.create_channel("Chan1");
        REQUIRE(anim.num_channels() == 1);
        REQUIRE(anim.has_channel("Chan1"));
        REQUIRE(ch1.name() == "Chan1");
        REQUIRE(&anim.channel(0) == &ch1);
        REQUIRE(&anim.channel("Chan1") == &ch1);

        Channel& ch2 = anim.create_channel("Chan2", 0);
        REQUIRE(anim.num_channels() == 2);
        REQUIRE(anim.size() == 2);
        REQUIRE(anim.has_channel("Chan2"));
        REQUIRE(ch2.name() == "Chan2");
        REQUIRE(anim.channel(0).name() == "Chan2");
        REQUIRE(anim.channel(1).name() == "Chan1");

        REQUIRE_THROWS_AS(anim.create_channel("Chan3", 5), std::out_of_range);
    }

    SECTION("Emplace channel") {
        anim.emplace_channel(Channel("EmplacedChan"));
        REQUIRE(anim.num_channels() == 1);
        REQUIRE(anim.has_channel("EmplacedChan"));
        REQUIRE(anim.channel(0).name() == "EmplacedChan");
    }

    SECTION("Insert channel") {
        Channel ch_const("ConstInsertedChan");
        anim.insert_channel(0, ch_const);
        REQUIRE(anim.num_channels() == 1);
        REQUIRE(anim.size() == 1);
        REQUIRE(anim.has_channel("ConstInsertedChan"));
        REQUIRE(anim.channel(0).name() == "ConstInsertedChan");

        anim.insert_channel(1, Channel("RValInsertedChan"));
        REQUIRE(anim.num_channels() == 2);
        REQUIRE(anim.has_channel("RValInsertedChan"));
        REQUIRE(anim.channel(1).name() == "RValInsertedChan");

        REQUIRE_THROWS_AS(anim.insert_channel(5, ch_const), std::out_of_range);
        REQUIRE_THROWS_AS(anim.insert_channel(5, Channel("RVal")), std::out_of_range);
    }

    SECTION("Access channels") {
        anim.create_channel("Ch1");
        anim.create_channel("Ch2");

        REQUIRE(anim.channel(0).name() == "Ch1");
        REQUIRE(anim[0].name() == "Ch1");
        REQUIRE(anim.channel("Ch2").name() == "Ch2");
        REQUIRE(anim["Ch2"].name() == "Ch2");

        const Animation& const_anim = anim;
        REQUIRE(const_anim.channel(0).name() == "Ch1");
        REQUIRE(const_anim[0].name() == "Ch1");
        REQUIRE(const_anim.channel("Ch2").name() == "Ch2");
        REQUIRE(const_anim["Ch2"].name() == "Ch2");

        REQUIRE_THROWS_AS(anim.channel(5), std::out_of_range);
        REQUIRE_THROWS_AS(anim.channel("NonExistent"), std::out_of_range);
        REQUIRE_THROWS_AS(const_anim.channel(5), std::out_of_range);
        REQUIRE_THROWS_AS(const_anim.channel("NonExistent"), std::out_of_range);
    }
    
    SECTION("Modify accessed channel") {
        anim.create_channel("ModCh");
        anim.channel("ModCh").create_keyframe(0.0, 1.0f);
        REQUIRE(anim.channel("ModCh").size() == 1);
        anim[0].create_keyframe(1.0, 2.0f);
        REQUIRE(anim[0].size() == 2);
    }


    SECTION("Channel information") {
        REQUIRE(anim.empty());
        REQUIRE(anim.size() == 0);
        REQUIRE(anim.num_channels() == 0);

        anim.create_channel("InfoCh1");
        anim.create_channel("InfoCh2");

        REQUIRE_FALSE(anim.empty());
        REQUIRE(anim.size() == 2);
        REQUIRE(anim.num_channels() == 2);
        REQUIRE(anim.has_channel("InfoCh1"));
        REQUIRE(anim.has_channel("InfoCh2"));
        REQUIRE_FALSE(anim.has_channel("NonExistent"));

        std::vector<std::string> names = anim.channel_names();
        REQUIRE(names.size() == 2);
        REQUIRE(std::find(names.begin(), names.end(), "InfoCh1") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "InfoCh2") != names.end());
    }

    SECTION("Remove channels") {
        anim.create_channel("RemCh1");
        anim.create_channel("RemCh2");
        anim.create_channel("RemCh3");
        REQUIRE(anim.num_channels() == 3);

        anim.remove_channel(1); // Removes RemCh2
        REQUIRE(anim.num_channels() == 2);
        REQUIRE_FALSE(anim.has_channel("RemCh2"));
        REQUIRE(anim.channel(0).name() == "RemCh1");
        REQUIRE(anim.channel(1).name() == "RemCh3");
        REQUIRE_THROWS_AS(anim.remove_channel(5), std::out_of_range);

        anim.remove_channel("RemCh1");
        REQUIRE(anim.num_channels() == 1);
        REQUIRE_FALSE(anim.has_channel("RemCh1"));
        REQUIRE(anim.channel(0).name() == "RemCh3");
        REQUIRE_THROWS_AS(anim.remove_channel("NonExistent"), std::out_of_range);

        anim.clear();
        REQUIRE(anim.empty());
        REQUIRE(anim.num_channels() == 0);
    }
    
    SECTION("Channels accessor") {
        anim.create_channel("C1");
        anim.create_channel("C2");
        
        std::vector<Channel>& channels_ref = anim.channels();
        REQUIRE(channels_ref.size() == 2);
        channels_ref[0].set_name("NewC1");
        REQUIRE(anim.channel(0).name() == "NewC1");

        const Animation& const_anim = anim;
        const std::vector<Channel>& const_channels_ref = const_anim.channels();
        REQUIRE(const_channels_ref.size() == 2);
        REQUIRE(const_channels_ref[0].name() == "NewC1");
    }
}

TEST_CASE("Animation Time Management", "[Animation]") {
    Animation anim;

    SECTION("Default times") {
        REQUIRE(anim.start_time() == Catch::Approx(0.0));
        REQUIRE(anim.end_time() == Catch::Approx(30.0));
        REQUIRE(anim.length() == Catch::Approx(30.0));
    }

    SECTION("Set start time") {
        anim.set_start_time(5.0);
        REQUIRE(anim.start_time() == Catch::Approx(5.0));
        REQUIRE(anim.end_time() == Catch::Approx(30.0)); // End time should remain
        REQUIRE(anim.length() == Catch::Approx(25.0));

        anim.set_start_time(35.0); // Should be capped by end_time
        REQUIRE(anim.start_time() == Catch::Approx(30.0));
        REQUIRE(anim.end_time() == Catch::Approx(30.0));
        REQUIRE(anim.length() == Catch::Approx(0.0));
    }

    SECTION("Set end time") {
        anim.set_start_time(0.0); // Reset for clarity
        anim.set_end_time(10.0);
        REQUIRE(anim.start_time() == Catch::Approx(0.0));
        REQUIRE(anim.end_time() == Catch::Approx(10.0));
        REQUIRE(anim.length() == Catch::Approx(10.0));

        anim.set_end_time(-5.0); // Should be capped by start_time
        REQUIRE(anim.start_time() == Catch::Approx(0.0));
        REQUIRE(anim.end_time() == Catch::Approx(0.0));
        REQUIRE(anim.length() == Catch::Approx(0.0));
    }

    SECTION("Set length") {
        anim.set_start_time(10.0);
        anim.set_end_time(20.0); // length is 10

        anim.set_length(15.0);
        REQUIRE(anim.start_time() == Catch::Approx(10.0));
        REQUIRE(anim.end_time() == Catch::Approx(25.0));
        REQUIRE(anim.length() == Catch::Approx(15.0));

        REQUIRE_THROWS_AS(anim.set_length(-5.0), std::invalid_argument);
    }
    
    SECTION("Length calculation with invalid times") {
        // This state should ideally not be reachable if setters are used correctly,
        // but testing the length() getter's robustness.
        // Manually setting private members is not possible here, so we rely on setters.
        anim.set_start_time(10.0);
        anim.set_end_time(5.0); // end_time will be capped to 10.0
        REQUIRE(anim.start_time() == Catch::Approx(10.0));
        REQUIRE(anim.end_time() == Catch::Approx(10.0));
        REQUIRE(anim.length() == Catch::Approx(0.0)); // Not throwing, as setters prevent invalid state.

        // To truly test the throw in length(), one would need to bypass setters or
        // temporarily modify the class to allow m_start_time > m_end_time.
        // For now, we assume setters correctly maintain m_start_time <= m_end_time.
        // If the design changes, this test might need adjustment.
    }
}

TEST_CASE("Animation Sample Calculation", "[Animation]") {
    Animation anim;
    anim.set_start_time(0.0);
    anim.set_end_time(1.0); // Length = 1.0

    SECTION("No channels") {
        REQUIRE(anim.num_samples(10.0) == 0);
    }

    anim.create_channel("SampleChan");

    SECTION("Valid sample rate") {
        // length = 1.0, sample_rate = 10.0
        // samples = ceil(1.0 * 10.0) + 1 = 10 + 1 = 11
        REQUIRE(anim.num_samples(10.0) == 11);

        // length = 1.0, sample_rate = 1.0
        // samples = ceil(1.0 * 1.0) + 1 = 1 + 1 = 2
        REQUIRE(anim.num_samples(1.0) == 2);

        anim.set_end_time(0.9); // Length = 0.9
        // samples = ceil(0.9 * 10.0) + 1 = ceil(9.0) + 1 = 9 + 1 = 10
        REQUIRE(anim.num_samples(10.0) == 10);
        
        anim.set_end_time(0.95); // Length = 0.95
        // samples = ceil(0.95 * 10.0) + 1 = ceil(9.5) + 1 = 10 + 1 = 11
        REQUIRE(anim.num_samples(10.0) == 11);
    }
    
    SECTION("Zero length animation") {
        anim.set_end_time(0.0); // Length = 0.0
        // samples = ceil(0.0 * 10.0) + 1 = 0 + 1 = 1
        REQUIRE(anim.num_samples(10.0) == 1);
    }

    SECTION("Invalid sample rate") {
        REQUIRE_THROWS_AS(anim.num_samples(0.0), std::invalid_argument);
        REQUIRE_THROWS_AS(anim.num_samples(-10.0), std::invalid_argument);
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
        
        // Test emplace channel
        animation.emplace_channel(Channel("rotation"));
        REQUIRE(animation.num_channels() == 3);
        REQUIRE(animation.has_channel("rotation"));
        
        // Test insert channel
        Channel scale_channel("scale");
        animation.insert_channel(1, scale_channel);
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
        REQUIRE(std::find(names.begin(), names.end(), "pos_x") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "pos_y") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "rotation") != names.end());
        REQUIRE(std::find(names.begin(), names.end(), "scale") != names.end());
        
        // Test channels accessor
        std::vector<Channel>& channels_ref = animation.channels();
        REQUIRE(channels_ref.size() == 4);
        
        const Animation& const_animation = animation;
        const std::vector<Channel>& const_channels_ref = const_animation.channels();
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
        REQUIRE_THROWS_AS(animation[5], std::out_of_range);
        REQUIRE_THROWS_AS(animation.remove_channel(5), std::out_of_range);
        REQUIRE_THROWS_AS(animation.create_channel("test3", 5), std::out_of_range);
        REQUIRE_THROWS_AS(animation.insert_channel(5, Channel("test4")), std::out_of_range);
        
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
        
        const std::vector<Channel>& const_channels = const_anim.channels();
        REQUIRE(const_channels.size() == 2);
    }
}

