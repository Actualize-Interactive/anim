#include "anim/bezier_handle.hpp"

namespace anim {

BezierHandle::BezierHandle(double time, double value)
    : time(time), value(value) {}

BezierHandle BezierHandle::operator+(const BezierHandle& other) const {
    return BezierHandle(time + other.time, value + other.value);
}

BezierHandle BezierHandle::operator-(const BezierHandle& other) const {
    return BezierHandle(time - other.time, value - other.value);
}

BezierHandle BezierHandle::operator*(double scalar) const {
    return BezierHandle(time * scalar, value * scalar);
}

BezierHandle BezierHandle::operator/(double scalar) const {
    if (scalar == 0.0) {
        throw std::domain_error("Division by zero");
    }
    return BezierHandle(time / scalar, value / scalar);
}

bool BezierHandle::operator==(const BezierHandle& other) const {
    return time == other.time && value == other.value;
}

bool BezierHandle::operator!=(const BezierHandle& other) const {
    return !(*this == other);
}

double BezierHandle::length() const {
    return std::sqrt(time * time + value * value);
}

BezierHandle BezierHandle::normalized() const {
    double len = length();
    if (len == 0.0) {
        throw std::domain_error("Cannot normalize a zero-length vector");
    }
    return *this / len;
}

} // namespace anim