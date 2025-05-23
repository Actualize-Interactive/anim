#include <anim.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

// Helper function to print an animation curve as ASCII art
void print_curve_ascii(const anim::Channel& channel, double start_time, double end_time, int width = 80, int height = 20) {
    // Sample the curve
    std::vector<double> samples = channel.evaluate_range(start_time, end_time, width);
    
    // Find min and max values to scale the output
    double min_val = samples[0];
    double max_val = samples[0];
    for (double val : samples) {
        min_val = std::min(min_val, val);
        max_val = std::max(max_val, val);
    }
    
    // Add some padding to min/max
    double padding = (max_val - min_val) * 0.1;
    min_val -= padding;
    max_val += padding;
    
    // Make sure we have a range
    if (std::abs(max_val - min_val) < 1e-6) {
        min_val -= 0.5;
        max_val += 0.5;
    }
    
    // Create a grid for the ASCII art
    std::vector<std::string> grid(height, std::string(width, ' '));
    
    // Plot the curve
    for (int x = 0; x < width; ++x) {
        double value = samples[x];
        int y = static_cast<int>((height - 1) * (1.0 - (value - min_val) / (max_val - min_val)));
        y = std::max(0, std::min(height - 1, y));
        grid[y][x] = '*';
    }
    
    // Print the grid
    std::cout << "Value range: [" << min_val << ", " << max_val << "]" << std::endl;
    std::cout << "Time range: [" << start_time << ", " << end_time << "]" << std::endl;
    std::cout << std::string(width + 2, '-') << std::endl;
    for (const auto& line : grid) {
        std::cout << "|" << line << "|" << std::endl;
    }
    std::cout << std::string(width + 2, '-') << std::endl;
}

int main() {
    // Example 1: Comparing different tangent modes
    std::cout << "Example 1: Comparing Different Tangent Modes\n";
    std::cout << "==========================================\n\n";
      // Create channels for each tangent mode
    anim::Channel linear_channel;
    anim::Channel flat_channel;
    anim::Channel smooth_auto_channel;
    anim::Channel smooth_manual_channel;
    anim::Channel stepped_channel;
    anim::Channel broken_channel;
    
    // Set keyframes for each channel (same time/values, different modes)
    // Two keyframes at t=0.0 and t=1.0 with values 0.0 and 1.0
    // Then a third keyframe at t=2.0 with value 0.0
      // LINEAR mode
    linear_channel.set_keyframe_at_time(0.0, 0.0, 
        anim::Point2D(-0.3, 0.0), anim::Point2D(0.3, 0.0), 
        anim::TangentMode::linear);
    linear_channel.set_keyframe_at_time(1.0, 1.0, 
        anim::Point2D(0.7, 1.0), anim::Point2D(1.3, 1.0), 
        anim::TangentMode::linear);
    linear_channel.set_keyframe_at_time(2.0, 0.0, 
        anim::Point2D(1.7, 0.0), anim::Point2D(2.3, 0.0), 
        anim::TangentMode::linear);
      // FLAT mode
    flat_channel.set_keyframe_at_time(0.0, 0.0, 
        anim::Point2D(-0.3, 0.0), anim::Point2D(0.3, 0.0), 
        anim::TangentMode::flat);
    flat_channel.set_keyframe_at_time(1.0, 1.0, 
        anim::Point2D(0.7, 1.0), anim::Point2D(1.3, 1.0), 
        anim::TangentMode::flat);
    flat_channel.set_keyframe_at_time(2.0, 0.0, 
        anim::Point2D(1.7, 0.0), anim::Point2D(2.3, 0.0), 
        anim::TangentMode::flat);
      // SMOOTH_AUTO mode
    smooth_auto_channel.set_keyframe_at_time(0.0, 0.0, 
        anim::Point2D(-0.3, 0.0), anim::Point2D(0.3, 0.0), 
        anim::TangentMode::smoothAuto);
    smooth_auto_channel.set_keyframe_at_time(1.0, 1.0, 
        anim::Point2D(0.7, 1.0), anim::Point2D(1.3, 1.0), 
        anim::TangentMode::smoothAuto);
    smooth_auto_channel.set_keyframe_at_time(2.0, 0.0, 
        anim::Point2D(1.7, 0.0), anim::Point2D(2.3, 0.0), 
        anim::TangentMode::smoothAuto);
      // SMOOTH_MANUAL mode (with specifically placed handles)
    smooth_manual_channel.set_keyframe_at_time(0.0, 0.0, 
        anim::Point2D(-0.3, 0.0), anim::Point2D(0.3, 0.3), 
        anim::TangentMode::smoothManual);
    smooth_manual_channel.set_keyframe_at_time(1.0, 1.0, 
        anim::Point2D(0.7, 0.7), anim::Point2D(1.3, 0.7), 
        anim::TangentMode::smoothManual);
    smooth_manual_channel.set_keyframe_at_time(2.0, 0.0, 
        anim::Point2D(1.7, 0.3), anim::Point2D(2.3, 0.0), 
        anim::TangentMode::smoothManual);
      // STEPPED mode
    stepped_channel.set_keyframe_at_time(0.0, 0.0, 
        anim::Point2D(-0.3, 0.0), anim::Point2D(0.3, 0.0), 
        anim::TangentMode::stepped);
    stepped_channel.set_keyframe_at_time(1.0, 1.0, 
        anim::Point2D(0.7, 1.0), anim::Point2D(1.3, 1.0), 
        anim::TangentMode::stepped);
    stepped_channel.set_keyframe_at_time(2.0, 0.0, 
        anim::Point2D(1.7, 0.0), anim::Point2D(2.3, 0.0), 
        anim::TangentMode::stepped);
      // BROKEN mode (with handles creating asymmetric curves)
    broken_channel.set_keyframe_at_time(0.0, 0.0, 
        anim::Point2D(-0.3, 0.0), anim::Point2D(0.3, 0.5), 
        anim::TangentMode::broken);
    broken_channel.set_keyframe_at_time(1.0, 1.0, 
        anim::Point2D(0.7, 0.5), anim::Point2D(1.3, 0.5), 
        anim::TangentMode::broken);
    broken_channel.set_keyframe_at_time(2.0, 0.0, 
        anim::Point2D(1.7, 0.5), anim::Point2D(2.3, 0.0), 
        anim::TangentMode::broken);
    
    // Print each curve
    std::cout << "LINEAR mode:\n";
    print_curve_ascii(linear_channel, 0.0, 2.0);
    std::cout << "\n";
    
    std::cout << "FLAT mode:\n";
    print_curve_ascii(flat_channel, 0.0, 2.0);
    std::cout << "\n";
    
    std::cout << "SMOOTH_AUTO mode:\n";
    print_curve_ascii(smooth_auto_channel, 0.0, 2.0);
    std::cout << "\n";
    
    std::cout << "SMOOTH_MANUAL mode:\n";
    print_curve_ascii(smooth_manual_channel, 0.0, 2.0);
    std::cout << "\n";
    
    std::cout << "STEPPED mode:\n";
    print_curve_ascii(stepped_channel, 0.0, 2.0);
    std::cout << "\n";
    
    std::cout << "BROKEN mode:\n";
    print_curve_ascii(broken_channel, 0.0, 2.0);
    std::cout << "\n";
      // Example 2: Animation with multiple channels
    std::cout << "Example 2: Animation with Multiple Channels\n";
    std::cout << "========================================\n\n";
    
    anim::Animation animation;
    // Create X position channel (accelerating motion)
    anim::Channel x_channel("position.x");
    x_channel.set_keyframe_at_time(0.0, 0.0, 
        anim::Point2D(-0.3, 0.0), anim::Point2D(0.3, 0.0), 
        anim::TangentMode::smoothAuto);
    x_channel.set_keyframe_at_time(1.0, 1.0, 
        anim::Point2D(0.7, 1.0), anim::Point2D(1.3, 1.0), 
        anim::TangentMode::smoothAuto);
    
    // Create Y position channel (bounce curve)
    anim::Channel y_channel("position.y");
    y_channel.set_keyframe_at_time(0.0, 0.0, 
        anim::Point2D(-0.3, 0.0), anim::Point2D(0.3, 0.0), 
        anim::TangentMode::smoothAuto);
    y_channel.set_keyframe_at_time(0.5, 1.0, 
        anim::Point2D(0.4, 1.0), anim::Point2D(0.6, 1.0), 
        anim::TangentMode::smoothAuto);
    y_channel.set_keyframe_at_time(1.0, 0.0, 
        anim::Point2D(0.7, 0.0), anim::Point2D(1.3, 0.0), 
        anim::TangentMode::smoothAuto);
    
    // Create scale channel (starts and ends at 1.0, contracts in the middle)
    anim::Channel scale_channel("scale");
    scale_channel.set_keyframe_at_time(0.0, 1.0, 
        anim::Point2D(-0.3, 1.0), anim::Point2D(0.3, 1.0), 
        anim::TangentMode::flat);
    scale_channel.set_keyframe_at_time(0.5, 0.5, 
        anim::Point2D(0.4, 0.5), anim::Point2D(0.6, 0.5), 
        anim::TangentMode::flat);
    scale_channel.set_keyframe_at_time(1.0, 1.0, 
        anim::Point2D(0.7, 1.0), anim::Point2D(1.3, 1.0), 
        anim::TangentMode::flat);
    
    // Add channels to animation
    animation.append_channel(x_channel);
    animation.append_channel(y_channel);
    animation.append_channel(scale_channel);
    
    // Display the curves
    std::cout << "Position X Channel:\n";
    print_curve_ascii(x_channel, 0.0, 1.0);
    std::cout << "\n";
    
    std::cout << "Position Y Channel:\n";
    print_curve_ascii(y_channel, 0.0, 1.0);
    std::cout << "\n";
    
    std::cout << "Scale Channel:\n";
    print_curve_ascii(scale_channel, 0.0, 1.0);
    std::cout << "\n";
    
    // Display animation frames
    std::cout << "Animation Frames:\n";
    std::cout << std::string(60, '-') << std::endl;
    std::cout << std::setw(10) << "Time" << " | " 
              << std::setw(10) << "X" << " | " 
              << std::setw(10) << "Y" << " | " 
              << std::setw(10) << "Scale" << std::endl;
    std::cout << std::string(60, '-') << std::endl;
    
    for (double t = 0.0; t <= 1.0; t += 0.1) {
        auto values = animation.evaluate_channels(t);
        std::cout << std::fixed << std::setprecision(2);
        std::cout << std::setw(10) << t << " | " 
                  << std::setw(10) << values["position.x"] << " | " 
                  << std::setw(10) << values["position.y"] << " | " 
                  << std::setw(10) << values["scale"] << std::endl;
    }
    
    std::cout << std::string(60, '-') << std::endl;
      // Example 3: Demonstrating animation.length() and num_samples()
    std::cout << "Example 3: Animation Length and Sample Count\n";
    std::cout << "==========================================\n\n";
    
    std::cout << "Animation length: " << animation.length() << " time units\n";
    std::cout << "Number of samples at 30Hz: " << animation.num_samples(30.0) << " samples\n";
    std::cout << "Number of samples at 60Hz: " << animation.num_samples(60.0) << " samples\n\n";
    
    return 0;
}


