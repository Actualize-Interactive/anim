#ifndef ANIM_POINT_HPP
#define ANIM_POINT_HPP

#include <stdexcept>

namespace anim {

struct Point {
    double time;
    double value;

    Point(double time = 0.0, double value = 0.0) : time(time), value(value) {}

    Point operator+(const Point& other) const {
        return Point(time + other.time, value + other.value);
    }

    Point operator-(const Point& other) const {
        return Point(time - other.time, value - other.value);
    }

    Point operator*(double scalar) const {
        return Point(time * scalar, value * scalar);
    }

    Point operator/(double scalar) const {
        if (scalar == 0.0) {
            throw std::domain_error("Division by zero");
        }
        return Point(time / scalar, value / scalar);
    }
    bool operator==(const Point& other) const {
        return time == other.time && value == other.value;
    }
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
    Point& operator+=(const Point& other) {
        time += other.time;
        value += other.value;
        return *this;
    }
    Point& operator-=(const Point& other) {
        time -= other.time;
        value -= other.value;
        return *this;
    }
    Point& operator*=(double scalar) {
        time *= scalar;
        value *= scalar;
        return *this;
    }
    Point& operator/=(double scalar) {
        if (scalar == 0.0) {
            throw std::domain_error("Division by zero");
        }
        time /= scalar;
        value /= scalar;
        return *this;
    }
    bool is_zero() const {
        return time == 0.0 && value == 0.0;
    }
    void reset() {
        time = 0.0;
        value = 0.0;
    }
    void set(double new_time, double new_value) {
        time = new_time;
        value = new_value;
    }
};

using Vector = Point; // Alias for vector operations

} // namespace anim

#endif // ANIM_POINT_HPP