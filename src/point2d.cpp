#include "anim/point2d.hpp"
#include <cmath>
#include <stdexcept>

namespace anim {

Point2D::Point2D(double time, double value) : time(time), value(value) {}

Point2D Point2D::operator+(const Point2D& other) const {
    return Point2D(time + other.time, value + other.value);
}

Point2D Point2D::operator-(const Point2D& other) const {
    return Point2D(time - other.time, value - other.value);
}

Point2D Point2D::operator*(double scalar) const {
    return Point2D(time * scalar, value * scalar);
}

Point2D Point2D::operator/(double scalar) const {
    if (std::abs(scalar) < 1e-10) {
        throw std::invalid_argument("Division by zero or near-zero in Point2D");
    }
    return Point2D(time / scalar, value / scalar);
}

bool Point2D::operator==(const Point2D& other) const {
    constexpr double epsilon = 1e-10;
    return std::abs(time - other.time) < epsilon && 
           std::abs(value - other.value) < epsilon;
}

bool Point2D::operator!=(const Point2D& other) const {
    return !(*this == other);
}

double Point2D::length() const {
    return std::sqrt(time * time + value * value);
}

Point2D Point2D::normalized() const {
    double l = length();
    if (l > 1e-9) { 
        return Point2D(time / l, value / l);
    }
    return Point2D(0.0, 0.0);
}

} // namespace anim
