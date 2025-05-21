#ifndef ANIM_POINT2D_HPP
#define ANIM_POINT2D_HPP

#include <cmath>
#include <stdexcept>

namespace anim {

struct Point2D {
    double time;
    double value;

    Point2D(double time = 0.0, double value = 0.0);

    Point2D operator+(const Point2D& other) const;
    Point2D operator-(const Point2D& other) const;
    Point2D operator*(double scalar) const;
    Point2D operator/(double scalar) const;
    bool operator==(const Point2D& other) const;
    bool operator!=(const Point2D& other) const;

    double length() const;
    Point2D normalized() const;
};

using BezierHandle = Point2D;

} // namespace anim

#endif // ANIM_POINT2D_HPP
