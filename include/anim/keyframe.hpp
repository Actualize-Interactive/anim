#ifndef ANIM_KEYFRAME_HPP
#define ANIM_KEYFRAME_HPP

#include "anim/bezier_handle.hpp"
#include "anim/tangent_mode.hpp"
#include <utility> // For std::move

namespace anim {

/**
 * @brief A keyframe in an animation curve.
 * 
 * Keyframes define specific values at specific points in time,
 * with Bézier handles controlling the interpolation between keyframes.
 */
class Keyframe {
public:
    /**
     * @brief Construct a new keyframe.
     * 
     * @param time The time position of the keyframe
     * @param value The value at this keyframe
     * @param in_handle The incoming Bézier handle
     * @param out_handle The outgoing Bézier handle
     * @param mode The tangent mode that controls how handles behave
     */
    Keyframe(double time, double value,
             const BezierHandle& in_handle,
             const BezierHandle& out_handle,
             TangentMode mode);
          
    /**
     * @brief Copy constructor
     */
    Keyframe(const Keyframe& other);
    
    /**
     * @brief Move constructor
     */
    Keyframe(Keyframe&& other) noexcept;
    
    /**
     * @brief Copy assignment operator
     */
    Keyframe& operator=(const Keyframe& other);
    
    /**
     * @brief Move assignment operator
     */
    Keyframe& operator=(Keyframe&& other) noexcept;

    /**
     * @brief Get the time position of this keyframe
     * @return The keyframe time
     */
    double time() const;
    
    /**
     * @brief Get the value at this keyframe
     * @return The keyframe value
     */
    double value() const;
    
    /**
     * @brief Get the incoming Bézier handle
     * @return Reference to the in-handle
     */
    const BezierHandle& in_handle() const;
    
    /**
     * @brief Get the outgoing Bézier handle
     * @return Reference to the out-handle
     */
    const BezierHandle& out_handle() const;
    
    /**
     * @brief Get the tangent mode
     * @return The current tangent mode
     */
    TangentMode mode() const;

    /**
     * @brief Set the time position
     * @param time New time value
     */
    void set_time(double time);
    
    /**
     * @brief Set the keyframe value
     * @param value New value
     */
    void set_value(double value);
    
    /**
     * @brief Set the incoming Bézier handle
     * @param in_handle New in-handle
     */
    void set_in_handle(const BezierHandle& in_handle);
    
    /**
     * @brief Set the outgoing Bézier handle
     * @param out_handle New out-handle
     */
    void set_out_handle(const BezierHandle& out_handle);
    
    /**
     * @brief Set the tangent mode
     * @param mode New tangent mode
     */
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
