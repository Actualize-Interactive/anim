#ifndef ANIM_KEYFRAME_H
#define ANIM_KEYFRAME_H

#include "point2d.h"
#include "tangent_mode.h"
#include <utility> // For std::move

namespace anim {

/**
 * @brief Represents an immutable anchor point on the animation curve with its value and tangent definitions.
 */
class Keyframe {
public:
    /**
     * @brief Constructor for a keyframe
     * 
     * @param time The time value of the keyframe
     * @param value The animated value at the keyframe
     * @param in_tangent The in-tangent handle (absolute time/value coordinates)
     * @param out_tangent The out-tangent handle (absolute time/value coordinates)
     * @param mode The tangent mode for this keyframe
     */
    Keyframe(double time, double value,
                     const BezierHandle& in_tangent,
                     const BezierHandle& out_tangent,
                     TangentMode mode)
        : m_time(time), m_value(value),
          m_in_tangent(in_tangent), m_out_tangent(out_tangent),
          m_mode(mode) {}
          
    /**
     * @brief Copy constructor
     */
    Keyframe(const Keyframe& other)
        : m_time(other.m_time), m_value(other.m_value),
          m_in_tangent(other.m_in_tangent), m_out_tangent(other.m_out_tangent),
          m_mode(other.m_mode) {}
          
    /**
     * @brief Move constructor
     */
    Keyframe(Keyframe&& other) noexcept
        : m_time(other.m_time), m_value(other.m_value),
          m_in_tangent(std::move(other.m_in_tangent)), 
          m_out_tangent(std::move(other.m_out_tangent)),
          m_mode(other.m_mode) {}
    
    /**
     * @brief Copy assignment operator
     */
    Keyframe& operator=(const Keyframe& other) {
        if (this != &other) {
            m_time = other.m_time;
            m_value = other.m_value;
            m_in_tangent = other.m_in_tangent;
            m_out_tangent = other.m_out_tangent;
            m_mode = other.m_mode;
        }
        return *this;
    }
    
    /**
     * @brief Move assignment operator
     */
    Keyframe& operator=(Keyframe&& other) noexcept {
        if (this != &other) {
            m_time = other.m_time;
            m_value = other.m_value;
            m_in_tangent = std::move(other.m_in_tangent);
            m_out_tangent = std::move(other.m_out_tangent);
            m_mode = other.m_mode;
        }
        return *this;
    }

    /**
     * @brief Get the time value of the keyframe
     * 
     * @return double The time value
     */
    double time() const { return m_time; }

    /**
     * @brief Get the animated value at the keyframe
     * 
     * @return double The value
     */
    double value() const { return m_value; }

    /**
     * @brief Get the in-tangent handle
     * 
     * @return const BezierHandle& The in-tangent handle
     */
    const BezierHandle& in_tangent() const { return m_in_tangent; }

    /**
     * @brief Get the out-tangent handle
     * 
     * @return const BezierHandle& The out-tangent handle
     */
    const BezierHandle& out_tangent() const { return m_out_tangent; }

    /**
     * @brief Get the tangent mode
     * 
     * @return TangentMode The tangent mode
     */
    TangentMode mode() const { return m_mode; }

    /**
     * @brief Equality operator for Keyframe
     * 
     * @param other The keyframe to compare with
     * @return bool True if the keyframes are equal, false otherwise
     */
    bool operator==(const Keyframe& other) const {
        return m_time == other.m_time &&
               m_value == other.m_value &&
               m_in_tangent == other.m_in_tangent &&
               m_out_tangent == other.m_out_tangent &&
               m_mode == other.m_mode;
    }

    /**
     * @brief Inequality operator for Keyframe
     * 
     * @param other The keyframe to compare with
     * @return bool True if the keyframes are not equal, false otherwise
     */
    bool operator!=(const Keyframe& other) const {
        return !(*this == other);
    }

    /**
     * @brief Comparison operator for sorting keyframes by time
     * 
     * @param other The keyframe to compare with
     * @return bool True if this keyframe's time is less than other's time
     */
    bool operator<(const Keyframe& other) const {
        return m_time < other.m_time;
    }

private:
    double m_time;                  ///< The time value of the keyframe
    double m_value;                 ///< The animated value at the keyframe
    BezierHandle m_in_tangent;      ///< The in-tangent handle (absolute time/value coordinates)
    BezierHandle m_out_tangent;     ///< The out-tangent handle (absolute time/value coordinates)
    TangentMode m_mode;             ///< The tangent mode for this keyframe
};

} // namespace anim

#endif // ANIM_KEYFRAME_H
