#include <anim.hpp>
#include <iostream>
#include <vector>

int main() {
    std::cout << "Testing anim library integration..." << std::endl;
    
    try {
        // Test creating a channel
        anim::Animation animation("Test Animation");
        anim::Channel& channel = animation.create_channel("TestChannel");
        std::cout << "✓ Channel created successfully" << std::endl;
        
        // Test creating keyframes
        channel.create_keyframe(0.0, 0.0);  // time=0, value=0
        channel.create_keyframe(1.0, 1.0);  // time=1, value=1
        channel.create_keyframe(2.0, 4.0);  // time=2, value=4
        std::cout << "✓ Keyframes created successfully" << std::endl;
        
        // Test keyframe count
        size_t count = channel.num_keyframes();
        std::cout << "✓ Keyframe count: " << count << std::endl;
        
        // Test time range
        double start = channel.start_time();
        double end = channel.end_time();
        std::cout << "✓ Time range: " << start << " to " << end << std::endl;
        
        // Test evaluation
        double value_at_1_5 = channel.evaluate(1.5);
        std::cout << "✓ Value at t=1.5: " << value_at_1_5 << std::endl;
        
        // Test keyframe access
        for (size_t i = 0; i < count; ++i) {
            const anim::Keyframe& kf = channel.keyframe(i);
            std::cout << "✓ Keyframe " << i << ": time=" << kf.position.time 
                      << ", value=" << kf.position.value << std::endl;
        }
        
        // Test keyframe modification
        channel.set_keyframe_time(1, 1.5);
        channel.set_keyframe_value(1, 1.5);
        std::cout << "✓ Keyframe modification successful" << std::endl;
        
        // Test evaluation again after modification
        double new_value = channel.evaluate(1.75);
        std::cout << "✓ Value at t=1.75 after modification: " << new_value << std::endl;
        
        std::cout << "\n🎉 All anim library tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown error occurred" << std::endl;
        return 1;
    }
}
