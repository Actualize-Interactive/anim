#ifndef ANIM_BEZIER_HANDLE_HPP
#define ANIM_BEZIER_HANDLE_HPP

#include <cmath>
#include <stdexcept>

namespace anim {

struct BezierHandle {
    double time;
    double value;
    
    BezierHandle(double time = 0.0, double value = 0.0);
    BezierHandle operator+(const BezierHandle& other) const;
    BezierHandle operator-(const BezierHandle& other) const;
    BezierHandle operator*(double scalar) const;
    BezierHandle operator/(double scalar) const;
    bool operator==(const BezierHandle& other) const;
    bool operator!=(const BezierHandle& other) const;
    double length() const;
    BezierHandle normalized() const;
};

} // namespace anim

#endif // ANIM_BEZIER_HANDLE_HPP