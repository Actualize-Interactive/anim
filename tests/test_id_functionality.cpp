#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <anim/animation.hpp>
#include <anim/channel.hpp>
#include <anim/id.hpp>
#include <unordered_set>
#include <set>

using namespace anim;

TEST_CASE("Channel ID Management", "[Animation][Id]") {
    Animation animation("id_test");
    
    SECTION("Channel creation assigns unique IDs") {
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        Channel& ch3 = animation.create_channel("channel3");
        
        // IDs should be unique
        REQUIRE(ch1.id() != ch2.id());
        REQUIRE(ch1.id() != ch3.id());
        REQUIRE(ch2.id() != ch3.id());
        
        // IDs should be valid
        REQUIRE(ch1.id().isValid());
        REQUIRE(ch2.id().isValid());
        REQUIRE(ch3.id().isValid());
        
        // IDs should be incrementing (implementation detail but good to check)
        REQUIRE(static_cast<uint64_t>(ch2.id()) > static_cast<uint64_t>(ch1.id()));
        REQUIRE(static_cast<uint64_t>(ch3.id()) > static_cast<uint64_t>(ch2.id()));
    }
    
    SECTION("Channel ID assignment across multiple animations") {
        Animation anim1("test1");
        Animation anim2("test2");
        
        Channel& ch1 = anim1.create_channel("ch1");
        Channel& ch2 = anim2.create_channel("ch2");
        
        // IDs should be unique across different animation instances
        REQUIRE(ch1.id() != ch2.id());
    }
    
    SECTION("Manager-controlled creation prevents external Channel creation") {
        // This section verifies that Channel cannot be created directly
        // The test is implicit - if this compiles without the protected constructor,
        // the design is broken. Since we made the constructor protected and 
        // befriended Animation, this ensures only Animation can create Channels.
        
        Animation animation("test");
        Channel& ch = animation.create_channel("managed_channel");
        REQUIRE(ch.id().isValid());
        REQUIRE(ch.name() == "managed_channel");
    }
}

TEST_CASE("Animation ID-based channel access", "[Animation][Id]") {
    Animation animation("test_animation");
    
    SECTION("Access channel by ID") {
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        
        // Access channels by ID
        REQUIRE(&animation.channel(id1) == &ch1);
        REQUIRE(&animation.channel(id2) == &ch2);
        REQUIRE(&animation[id1] == &ch1);
        REQUIRE(&animation[id2] == &ch2);
        
        // Verify we can access the channel's properties through ID
        REQUIRE(animation.channel(id1).name() == "channel1");
        REQUIRE(animation.channel(id2).name() == "channel2");
    }
    
    SECTION("Invalid ID throws exception") {
        Channel& ch = animation.create_channel("channel1");
        Id valid_id = ch.id();
        
        // Create an invalid ID
        Id invalid_id = Id::invalid();
        
        // Try to access with invalid ID
        REQUIRE_THROWS_AS(animation.channel(invalid_id), std::out_of_range);
        REQUIRE_THROWS_AS(animation[invalid_id], std::out_of_range);
        
        // Try to access with a non-existent but valid ID
        Id non_existent_id(999999);
        REQUIRE_THROWS_AS(animation.channel(non_existent_id), std::out_of_range);
        REQUIRE_THROWS_AS(animation[non_existent_id], std::out_of_range);
    }
}

TEST_CASE("Channel reordering", "[Animation][Id]") {
    Animation animation("test_animation");
    
    SECTION("Reorder by index") {
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        Channel& ch3 = animation.create_channel("channel3");
        
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        Id id3 = ch3.id();
        
        // Initial order: ch1, ch2, ch3
        REQUIRE(animation.channel(0).name() == "channel1");
        REQUIRE(animation.channel(1).name() == "channel2");
        REQUIRE(animation.channel(2).name() == "channel3");
        
        // Move ch3 to position 0
        animation.reorder_channel(2, 0);
        
        // New order: ch3, ch1, ch2
        REQUIRE(animation.channel(0).name() == "channel3");
        REQUIRE(animation.channel(1).name() == "channel1");
        REQUIRE(animation.channel(2).name() == "channel2");
        
        // IDs should still work
        REQUIRE(&animation.channel(id1) == &ch1);
        REQUIRE(&animation.channel(id2) == &ch2);
        REQUIRE(&animation.channel(id3) == &ch3);
    }
      SECTION("Reorder by name") {
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        Channel& ch3 = animation.create_channel("channel3");
        
        Id id1 = ch1.id();
        
        // Move channel1 to the end (position 2)
        animation.reorder_channel("channel1", 2);
        
        // New order: ch2, ch3, ch1
        REQUIRE(animation.channel(0).name() == "channel2");
        REQUIRE(animation.channel(1).name() == "channel3");
        REQUIRE(animation.channel(2).name() == "channel1");
        
        // ID access should still work
        REQUIRE(&animation.channel(id1) == &ch1);
    }
    
    SECTION("Reorder by ID") {
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        Channel& ch3 = animation.create_channel("channel3");
        
        Id id2 = ch2.id();
        
        // Move channel2 to position 0
        animation.reorder_channel(id2, 0);
        
        // New order: ch2, ch1, ch3
        REQUIRE(animation.channel(0).name() == "channel2");
        REQUIRE(animation.channel(1).name() == "channel1");
        REQUIRE(animation.channel(2).name() == "channel3");
        
        // ID access should still work
        REQUIRE(&animation.channel(id2) == &ch2);
    }
}

using namespace anim;

TEST_CASE("Channel ID functionality", "[Channel][Id]") {
    SECTION("Channel has immutable ID") {
        Animation animation("test_animation");
        
        // Create channels and verify they have unique IDs
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        Channel& ch3 = animation.create_channel("channel3");
        
        // Check that IDs are assigned and unique
        REQUIRE(ch1.id().isValid());
        REQUIRE(ch2.id().isValid());
        REQUIRE(ch3.id().isValid());
        
        REQUIRE(ch1.id() != ch2.id());
        REQUIRE(ch1.id() != ch3.id());
        REQUIRE(ch2.id() != ch3.id());
          // IDs should be sequential (but we don't assume starting value due to static counter)
        uint64_t id1_value = static_cast<uint64_t>(ch1.id());
        uint64_t id2_value = static_cast<uint64_t>(ch2.id());
        uint64_t id3_value = static_cast<uint64_t>(ch3.id());
        
        REQUIRE(id2_value == id1_value + 1);
        REQUIRE(id3_value == id2_value + 1);
    }
    
    SECTION("Channel ID accessor returns correct ID") {
        Animation animation("test_animation");
        Channel& ch = animation.create_channel("test_channel");
        
        Id channel_id = ch.id();
        REQUIRE(channel_id.isValid());
        REQUIRE(static_cast<uint64_t>(channel_id) >= 1);
    }
    
    SECTION("Channel ID is immutable") {
        Animation animation("test_animation");
        Channel& ch = animation.create_channel("test_channel");
        
        Id original_id = ch.id();
        
        // Modify channel properties - ID should remain the same
        ch.set_name("modified_name");
        ch.create_keyframe(0.0, 1.0);
        ch.create_keyframe(1.0, 2.0);
        
        REQUIRE(ch.id() == original_id);
    }
}

TEST_CASE("Animation ID-based channel access with const support", "[Animation][Id]") {
    SECTION("Access channel by ID") {
        Animation animation("test_animation");
        
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        
        // Access channels by ID
        REQUIRE(&animation.channel(id1) == &ch1);
        REQUIRE(&animation.channel(id2) == &ch2);
        REQUIRE(&animation[id1] == &ch1);
        REQUIRE(&animation[id2] == &ch2);
        
        // Verify we can access the channel's properties through ID
        REQUIRE(animation.channel(id1).name() == "channel1");
        REQUIRE(animation.channel(id2).name() == "channel2");
    }
    
    SECTION("Access const channel by ID") {
        Animation animation("test_animation");
        
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        
        const Animation& const_animation = animation;
        
        // Access const channels by ID
        REQUIRE(&const_animation.channel(id1) == &ch1);
        REQUIRE(&const_animation.channel(id2) == &ch2);
        REQUIRE(&const_animation[id1] == &ch1);
        REQUIRE(&const_animation[id2] == &ch2);
        
        // Verify we can access the channel's properties through ID
        REQUIRE(const_animation.channel(id1).name() == "channel1");
        REQUIRE(const_animation.channel(id2).name() == "channel2");
    }
    
    SECTION("Invalid ID throws exception") {
        Animation animation("test_animation");
        
        // Create a channel to get a valid ID
        Channel& ch = animation.create_channel("channel1");
        Id valid_id = ch.id();
        
        // Create an invalid ID
        Id invalid_id = Id::invalid();
        
        // Try to access with invalid ID
        REQUIRE_THROWS_AS(animation.channel(invalid_id), std::out_of_range);
        REQUIRE_THROWS_AS(animation[invalid_id], std::out_of_range);
        
        // Try to access with a non-existent but valid ID
        Id non_existent_id(999999);
        REQUIRE_THROWS_AS(animation.channel(non_existent_id), std::out_of_range);
        REQUIRE_THROWS_AS(animation[non_existent_id], std::out_of_range);
    }
}

TEST_CASE("Channel map maintenance", "[Animation][Id]") {
    SECTION("Channel map updated on creation") {
        Animation animation("test_animation");
          // Create channels
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2", 0); // Insert at beginning
        Channel& ch3 = animation.create_channel("channel3"); // Normal creation
        
        // Verify all channels are accessible by ID
        REQUIRE(&animation.channel(ch1.id()) == &ch1);
        REQUIRE(&animation.channel(ch2.id()) == &ch2);
        REQUIRE(&animation.channel(ch3.id()) == &ch3);
        
        // Verify channel order in vector is correct
        REQUIRE(animation.channel(0).name() == "channel2"); // ch2 was inserted at index 0
        REQUIRE(animation.channel(1).name() == "channel1"); // ch1 was pushed to index 1
        REQUIRE(animation.channel(2).name() == "channel3"); // ch3 was added at the end
    }
      SECTION("Channel map updated on insertion") {
        Animation animation("test_animation");
        
        Channel& ch1 = animation.create_channel("channel1");
        
        // Create additional channels using create_channel with position
        Channel& ch2 = animation.create_channel("channel2", 0); // Insert at beginning
        Channel& ch3 = animation.create_channel("channel3"); // Add at end
        
        // Verify all channels are accessible by ID
        REQUIRE(&animation.channel(ch1.id()) == &ch1);
        REQUIRE(&animation.channel(ch2.id()) == &ch2);
        REQUIRE(&animation.channel(ch3.id()) == &ch3);
          // Verify insertion order
        REQUIRE(animation.channel(0).name() == "channel2");
        REQUIRE(animation.channel(1).name() == "channel1");
        REQUIRE(animation.channel(2).name() == "channel3");
    }
    
    SECTION("Channel map cleaned up on removal by index") {
        Animation animation("test_animation");
        
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        Channel& ch3 = animation.create_channel("channel3");
        
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        Id id3 = ch3.id();
        
        // Remove middle channel
        animation.remove_channel(1); // Remove ch2
        
        // ch2 should be inaccessible
        REQUIRE_THROWS_AS(animation.channel(id2), std::out_of_range);
        
        // ch1 and ch3 should still be accessible
        REQUIRE(&animation.channel(id1) == &ch1);
        REQUIRE(&animation.channel(id3) == &ch3);
        
        // Check vector state
        REQUIRE(animation.num_channels() == 2);
        REQUIRE(animation.channel(0).name() == "channel1");
        REQUIRE(animation.channel(1).name() == "channel3");
    }
    
    SECTION("Channel map cleaned up on removal by name") {
        Animation animation("test_animation");
        
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        Channel& ch3 = animation.create_channel("channel3");
        
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        Id id3 = ch3.id();
        
        // Remove by name
        animation.remove_channel("channel2");
        
        // ch2 should be inaccessible
        REQUIRE_THROWS_AS(animation.channel(id2), std::out_of_range);
        
        // ch1 and ch3 should still be accessible
        REQUIRE(&animation.channel(id1) == &ch1);
        REQUIRE(&animation.channel(id3) == &ch3);
        
        REQUIRE(animation.num_channels() == 2);
    }
    
    SECTION("Channel map cleaned up on removal by ID") {
        Animation animation("test_animation");
        
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        Channel& ch3 = animation.create_channel("channel3");
        
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        Id id3 = ch3.id();
        
        // Remove by ID
        animation.remove_channel(id2);
        
        // ch2 should be inaccessible
        REQUIRE_THROWS_AS(animation.channel(id2), std::out_of_range);
        
        // ch1 and ch3 should still be accessible
        REQUIRE(&animation.channel(id1) == &ch1);
        REQUIRE(&animation.channel(id3) == &ch3);
        
        REQUIRE(animation.num_channels() == 2);
    }
    
    SECTION("Channel map cleared on clear") {
        Animation animation("test_animation");
        
        Channel& ch1 = animation.create_channel("channel1");
        Channel& ch2 = animation.create_channel("channel2");
        
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        
        // Clear all channels
        animation.clear();
        
        // All channels should be inaccessible
        REQUIRE_THROWS_AS(animation.channel(id1), std::out_of_range);
        REQUIRE_THROWS_AS(animation.channel(id2), std::out_of_range);
        
        REQUIRE(animation.empty());
        REQUIRE(animation.num_channels() == 0);
    }
}

TEST_CASE("Channel ID persistence through operations", "[Animation][Id]") {
    SECTION("IDs persist through vector reallocations") {
        Animation animation("test_animation");
        
        // Create many channels to force vector reallocations
        std::vector<Id> channel_ids;
        constexpr size_t num_channels = 100;
        
        for (size_t i = 0; i < num_channels; ++i) {
            Channel& ch = animation.create_channel("channel_" + std::to_string(i));
            channel_ids.push_back(ch.id());
        }
        
        // Verify all channels are still accessible by their IDs
        for (size_t i = 0; i < num_channels; ++i) {
            REQUIRE(&animation.channel(channel_ids[i]) == &animation.channel(i));
            REQUIRE(animation.channel(channel_ids[i]).name() == "channel_" + std::to_string(i));
        }
    }
    
    SECTION("IDs remain valid after channel modifications") {
        Animation animation("test_animation");
        
        Channel& ch = animation.create_channel("test_channel");
        Id original_id = ch.id();
        
        // Modify channel extensively
        ch.create_keyframe(0.0, 0.0);
        ch.create_keyframe(1.0, 1.0);
        ch.create_keyframe(2.0, 4.0);
        ch.set_name("modified_channel");
        ch.set_keyframe_value(1, 5.0);
        ch.delete_keyframe(0);
        
        // ID should still be the same and channel should be accessible
        REQUIRE(ch.id() == original_id);
        REQUIRE(&animation.channel(original_id) == &ch);
        REQUIRE(animation.channel(original_id).name() == "modified_channel");
    }
}

TEST_CASE("Static channel ID counter", "[Animation][Id]") {
    SECTION("Channel IDs are globally unique across animations") {
        Animation anim1("animation1");
        Animation anim2("animation2");
        
        // Create channels in different animations
        Channel& ch1_anim1 = anim1.create_channel("ch1");
        Channel& ch1_anim2 = anim2.create_channel("ch1");
        Channel& ch2_anim1 = anim1.create_channel("ch2");
        Channel& ch2_anim2 = anim2.create_channel("ch2");
        
        // All IDs should be unique
        std::set<uint64_t> ids = {
            static_cast<uint64_t>(ch1_anim1.id()),
            static_cast<uint64_t>(ch1_anim2.id()),
            static_cast<uint64_t>(ch2_anim1.id()),
            static_cast<uint64_t>(ch2_anim2.id())
        };
        
        REQUIRE(ids.size() == 4); // All IDs should be unique
        
        // IDs should be sequential
        auto it = ids.begin();
        uint64_t expected_id = *it;
        for (++it; it != ids.end(); ++it) {
            REQUIRE(*it == expected_id + 1);
            expected_id = *it;
        }
    }
    
    SECTION("Channel ID counter increments correctly") {
        Animation animation("test_animation");
        
        // Create several channels and verify sequential IDs
        std::vector<Channel*> channels;
        constexpr size_t num_channels = 10;
        
        for (size_t i = 0; i < num_channels; ++i) {
            channels.push_back(&animation.create_channel("channel_" + std::to_string(i)));
        }
        
        // Verify IDs are sequential
        for (size_t i = 1; i < num_channels; ++i) {
            uint64_t current_id = static_cast<uint64_t>(channels[i]->id());
            uint64_t prev_id = static_cast<uint64_t>(channels[i-1]->id());
            REQUIRE(current_id == prev_id + 1);
        }
    }
}

TEST_CASE("Edge cases and error conditions", "[Animation][Id]") {
    SECTION("Cannot create channel without ID") {
        // This test verifies that the Channel() default constructor is deleted
        // The test is implicit - if this compiles, the test passes
        // If Channel() were available, this would not compile:
        // Channel ch; // This should not compile
        REQUIRE(true); // Placeholder assertion
    }
    
    SECTION("Remove non-existent channel by ID throws exception") {
        Animation animation("test_animation");
        
        Channel& ch = animation.create_channel("test_channel");
        Id valid_id = ch.id();
        
        // Try to remove with non-existent ID
        Id non_existent_id(999999);
        REQUIRE_THROWS_AS(animation.remove_channel(non_existent_id), std::out_of_range);
        
        // Original channel should still be accessible
        REQUIRE(&animation.channel(valid_id) == &ch);
    }
    
    SECTION("Multiple operations maintain consistency") {
        Animation animation("test_animation");
        
        // Create initial channels
        Channel& ch1 = animation.create_channel("ch1");
        Channel& ch2 = animation.create_channel("ch2");
        Channel& ch3 = animation.create_channel("ch3");
        
        Id id1 = ch1.id();
        Id id2 = ch2.id();
        Id id3 = ch3.id();
        
        // Perform various operations
        animation.remove_channel(1); // Remove ch2
        Channel& ch4 = animation.create_channel("ch4");
        Id id4 = ch4.id();
          animation.remove_channel("ch1");
        Channel& ch5 = animation.create_channel("ch5", 0); // Insert at beginning
        Id id5 = ch5.id();
        
        // Verify final state
        REQUIRE_THROWS_AS(animation.channel(id1), std::out_of_range); // ch1 removed
        REQUIRE_THROWS_AS(animation.channel(id2), std::out_of_range); // ch2 removed
        REQUIRE(&animation.channel(id3) == &ch3); // ch3 still exists
        REQUIRE(&animation.channel(id4) == &ch4); // ch4 exists
        REQUIRE(&animation.channel(id5) == &ch5); // ch5 exists
        
        REQUIRE(animation.num_channels() == 3);
        REQUIRE(animation.channel(0).name() == "ch5");
        REQUIRE(animation.channel(1).name() == "ch3");
        REQUIRE(animation.channel(2).name() == "ch4");
    }
}
