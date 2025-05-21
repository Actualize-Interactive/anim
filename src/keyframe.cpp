#include "anim/keyframe.hpp"

namespace anim {

Keyframe::Keyframe(double time, double value,
                   const BezierHandle& in_tangent,
                   const BezierHandle& out_tangent,
                   TangentMode mode)
    : m_time(time), m_value(value),
      m_in_tangent(in_tangent), m_out_tangent(out_tangent),
      m_mode(mode) {}
      
Keyframe::Keyframe(const Keyframe& other)
    : m_time(other.m_time), m_value(other.m_value),
      m_in_tangent(other.m_in_tangent), m_out_tangent(other.m_out_tangent),
      m_mode(other.m_mode) {}
      
Keyframe::Keyframe(Keyframe&& other) noexcept
    : m_time(other.m_time), m_value(other.m_value),
      m_in_tangent(std::move(other.m_in_tangent)), 
      m_out_tangent(std::move(other.m_out_tangent)),
      m_mode(other.m_mode) {}

Keyframe& Keyframe::operator=(const Keyframe& other) {
    if (this != &other) {
        m_time = other.m_time;
        m_value = other.m_value;
        m_in_tangent = other.m_in_tangent;
        m_out_tangent = other.m_out_tangent;
        m_mode = other.m_mode;
    }
    return *this;
}

Keyframe& Keyframe::operator=(Keyframe&& other) noexcept {
    if (this != &other) {
        m_time = other.m_time;
        m_value = other.m_value;
        m_in_tangent = std::move(other.m_in_tangent);
        m_out_tangent = std::move(other.m_out_tangent);
        m_mode = other.m_mode;
    }
    return *this;
}

double Keyframe::time() const { 
    return m_time; 
}

double Keyframe::value() const { 
    return m_value; 
}

const BezierHandle& Keyframe::in_tangent() const { 
    return m_in_tangent; 
}

const BezierHandle& Keyframe::out_tangent() const { 
    return m_out_tangent; 
}

TangentMode Keyframe::mode() const { 
    return m_mode; 
}

void Keyframe::set_time(double time) { 
    m_time = time; 
}

void Keyframe::set_value(double value) { 
    m_value = value; 
}

void Keyframe::set_in_tangent(const BezierHandle& in_tangent) { 
    m_in_tangent = in_tangent; 
}

void Keyframe::set_out_tangent(const BezierHandle& out_tangent) { 
    m_out_tangent = out_tangent; 
}

void Keyframe::set_mode(TangentMode mode) { 
    m_mode = mode; 
}

bool Keyframe::operator==(const Keyframe& other) const {
    return m_time == other.m_time &&
           m_value == other.m_value &&
           m_in_tangent == other.m_in_tangent &&
           m_out_tangent == other.m_out_tangent &&
           m_mode == other.m_mode;
}

bool Keyframe::operator!=(const Keyframe& other) const {
    return !(*this == other);
}

bool Keyframe::operator<(const Keyframe& other) const {
    return m_time < other.m_time;
}

} // namespace anim
