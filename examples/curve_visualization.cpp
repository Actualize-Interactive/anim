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
    anim::Channel channel;


    // Print each curve
    std::cout << "Test channel:\n";
    print_curve_ascii(channel, 0.0, 2.0, 200, 40);
    std::cout << "\n";

    return 0;
}


