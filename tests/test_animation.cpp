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

