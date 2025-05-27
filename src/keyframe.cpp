#include "anim/keyframe.hpp"

namespace anim {

Keyframe::Keyframe(double time, double value,
                   const BezierHandle& in_handle,
                   const BezierHandle& out_handle,
                   TangentMode mode)
    : m_time(time), m_value(value),
      m_in_tangent(in_handle), m_out_tangent(out_handle),
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

const BezierHandle& Keyframe::in_handle() const { 
    return m_in_tangent; 
}

const BezierHandle& Keyframe::out_handle() const { 
    return m_out_tangent; 
}

TangentMode Keyframe::mode() const { 
    return m_mode; 
}

void Keyframe::set_time(double time) { 
    double old_time = m_time;
    m_time = time; 
    
    // Update the handles to maintain their relative positions
    double time_diff = m_time - old_time;
    m_in_tangent.time += time_diff;
    m_out_tangent.time += time_diff;
}

void Keyframe::set_value(double value) { 
    double old_value = m_value;
    m_value = value; 
    
    // For flat mode, adjust handle values to match the keyframe value
    if (m_mode == TangentMode::flat) {
        m_in_tangent.value = m_value;
        m_out_tangent.value = m_value;
    }
    else {
        // For other modes, update handle values to maintain relative distances
        double value_diff = m_value - old_value;
        m_in_tangent.value += value_diff;
        m_out_tangent.value += value_diff;
    }
}

void Keyframe::set_in_handle(const BezierHandle& in_handle) { 
    // For flat mode, keep the same value as the keyframe
    if (m_mode == TangentMode::flat) {
        m_in_tangent.time = in_handle.time;
        m_in_tangent.value = m_value;
    }
    // For manual mode, update the out handle to maintain collinearity
    else if (m_mode == TangentMode::manual) {
        m_in_tangent = in_handle;
        
        // Calculate the vector from keyframe to in_handle
        double in_dx = m_in_tangent.time - m_time;
        double in_dy = m_in_tangent.value - m_value;
        double in_length = std::sqrt(in_dx * in_dx + in_dy * in_dy);
        
        if (in_length > 0) {
            // Keep the same length for out_handle but in opposite direction
            double out_dx = -in_dx;
            double out_dy = -in_dy;
            double out_length = std::sqrt(out_dx * out_dx + out_dy * out_dy);
            
            // Scale to match in_handle length
            double scale = in_length / out_length;
            m_out_tangent.time = m_time + out_dx * scale;
            m_out_tangent.value = m_value + out_dy * scale;
        }
    }
    else {
        // For other modes (including broken), directly set the handle
        m_in_tangent = in_handle;
    }
}

void Keyframe::set_out_handle(const BezierHandle& out_handle) { 
    // For flat mode, keep the same value as the keyframe
    if (m_mode == TangentMode::flat) {
        m_out_tangent.time = out_handle.time;
        m_out_tangent.value = m_value;
    }
    // For manual mode, update the in handle to maintain collinearity
    else if (m_mode == TangentMode::manual) {
        m_out_tangent = out_handle;
        
        // Calculate the vector from keyframe to out_handle
        double out_dx = m_out_tangent.time - m_time;
        double out_dy = m_out_tangent.value - m_value;
        double out_length = std::sqrt(out_dx * out_dx + out_dy * out_dy);
        
        if (out_length > 0) {
            // Keep the same length for in_handle but in opposite direction
            double in_dx = -out_dx;
            double in_dy = -out_dy;
            double in_length = std::sqrt(in_dx * in_dx + in_dy * in_dy);
            
            // Scale to match out_handle length
            double scale = out_length / in_length;
            m_in_tangent.time = m_time + in_dx * scale;
            m_in_tangent.value = m_value + in_dy * scale;
        }
    }
    else {
        // For other modes (including broken), directly set the handle
        m_out_tangent = out_handle;
    }
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
