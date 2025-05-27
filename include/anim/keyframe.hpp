#ifndef ANIM_KEYFRAME_HPP
#define ANIM_KEYFRAME_HPP

#include "anim/bezier_handle.hpp"
#include "anim/tangent_mode.hpp"
#include <utility> // For std::move

namespace anim {

class Keyframe {
public:
    Keyframe(double time, double value, const BezierHandle& in_handle, const BezierHandle& out_handle, TangentMode mode);
    Keyframe(const Keyframe& other);
    Keyframe(Keyframe&& other) noexcept;
    Keyframe& operator=(const Keyframe& other);
    Keyframe& operator=(Keyframe&& other) noexcept;
    double time() const;
    double value() const;
    const BezierHandle& in_handle() const;
    const BezierHandle& out_handle() const;
    TangentMode mode() const;
    void set_time(double time);
    void set_value(double value);
    void set_in_handle(const BezierHandle& in_handle);
    void set_out_handle(const BezierHandle& out_handle);
    void set_mode(TangentMode mode);
    bool operator==(const Keyframe& other) const;
    bool operator!=(const Keyframe& other) const;
    bool operator<(const Keyframe& other) const;

private:
    double m_time;
    double m_value;
    BezierHandle m_in_tangent;
    BezierHandle m_out_tangent;
    TangentMode m_mode;
};

} // namespace anim

#endif // ANIM_KEYFRAME_HPP
