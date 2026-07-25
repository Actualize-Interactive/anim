#ifndef ANIM_HANDLE_UTILS_HPP
#define ANIM_HANDLE_UTILS_HPP

#include "point.hpp"
#include "keyframe.hpp"

namespace anim {

    /// @brief Identifies which handle of a keyframe is being dragged, driving alignment behavior.
    enum class GrabbedHandle {
        None,       ///< No specific handle; the implementation picks a source automatically.
        InHandle,   ///< The incoming handle is the one being manipulated.
        OutHandle   ///< The outgoing handle is the one being manipulated.
    };

    /**
     * @brief Compares two doubles for approximate equality.
     * @param a First value.
     * @param b Second value.
     * @param epsilon Maximum absolute difference treated as equal.
     * @return True if @c |a-b| < @p epsilon.
     */
    bool nearly_equal(double a, double b, double epsilon = 1e-9);

    /// @brief Euclidean distance between two points.
    double distance(const Point& p1, const Point& p2);

    /// @brief The vector from @p p1 to @p p2 (i.e. @c p2 - @c p1).
    Vector vector(const Point& p1, const Point& p2);

    /**
     * @brief Returns @p vec scaled to unit length.
     * @throws std::domain_error if @p vec has zero length.
     */
    Vector normalize(const Vector& vec);

    /// @brief Squared length of @p vec (avoids the sqrt of length()).
    double length_squared(const Vector& vec);

    /// @brief Length (magnitude) of @p vec.
    double length(const Vector& vec);

    /// @brief Midpoint between two points.
    Point midpoint(const Point& p1, const Point& p2);

    /// @brief Returns @p p with both components multiplied by @p scalar.
    Point scale(const Point& p, double scalar);

    /// @brief Returns @p p translated by @p vec.
    Point translate(const Point& p, const Vector& vec);

    /// @brief Rotates @p p about the origin by @p angle_degrees (counter-clockwise).
    Point rotate(const Point& p, double angle_degrees);

    /// @brief Dot product of two vectors.
    double dot_product(const Vector& v1, const Vector& v2);

    /// @brief 2D cross product (scalar z-component) of two vectors.
    double cross_product(const Vector& v1, const Vector& v2);

    /**
     * @brief Reflects @p vec across @p normal_unit_vector.
     * @param vec The incident vector.
     * @param normal_unit_vector The reflection normal; assumed to be unit length.
     */
    Vector reflect(const Vector& vec, const Vector& normal_unit_vector);

    /// @brief Returns @p vec with both components negated.
    Vector invert(const Vector& vec);

    /// @brief Clamps a keyframe's in-handle time into [prev keyframe time, keyframe time].
    void constrain_in_handle_time(Keyframe& keyframe, const Keyframe& prev_keyframe);

    /// @brief Clamps a keyframe's out-handle time into [keyframe time, next keyframe time].
    void constrain_out_handle_time(Keyframe& keyframe, const Keyframe& next_keyframe);

    /**
     * @brief Clamps both handle times within the neighbouring keyframes.
     * @param keyframe The keyframe whose handles are clamped.
     * @param prev_keyframe_ptr Previous keyframe, or nullptr if this is the first.
     * @param next_keyframe_ptr Next keyframe, or nullptr if this is the last.
     */
    void constrain_handles_time(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr);

    /**
     * @brief Adjusts a handle so its time does not cross a boundary keyframe.
     *
     * If the handle violates the boundary it is scaled back along the line from
     * the keyframe to the handle so it lands exactly on the boundary time.
     * Vertical handles (zero time delta) are left unchanged.
     * @param keyframe_pos The owning keyframe's position.
     * @param handle_to_adjust The handle to constrain (modified in place).
     * @param boundary_pos The neighbouring keyframe's position acting as the limit.
     * @param is_in_handle True for an in-handle (limited below), false for an out-handle (limited above).
     */
    void ensure_handle_time_boundary(
        const Point& keyframe_pos,
        Point& handle_to_adjust,
        const Point& boundary_pos,
        bool is_in_handle
    );

    /// @brief Applies the handle time constraints required for Function::Linear / Function::Constant segments.
    void ensure_linear_handles_time_boundary(
        Keyframe& keyframe,
        const Keyframe* prev_keyframe_ptr,
        const Keyframe* next_keyframe_ptr);

    /**
     * @brief Computes HandleMode::Flat handles for a keyframe.
     *
     * Handles take the keyframe's value (a horizontal tangent); their times are
     * adjustable but clamped within the neighbouring keyframes.
     * @param keyframe The keyframe to update (modified in place).
     * @param prev_keyframe_ptr Previous keyframe, or nullptr if this is the first.
     * @param next_keyframe_ptr Next keyframe, or nullptr if this is the last.
     */
    void apply_flat_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr);

    /**
     * @brief Computes HandleMode::Smooth handles for a keyframe.
     *
     * Produces collinear handles for a smooth (C1-continuous) transition, with
     * magnitude proportional to the distance to the adjacent keyframes.
     * @param keyframe The keyframe to update (modified in place).
     * @param prev_keyframe_ptr Previous keyframe, or nullptr if this is the first.
     * @param next_keyframe_ptr Next keyframe, or nullptr if this is the last.
     * @param smooth_factor Fraction of the neighbour distance used for the handle length.
     */
    void apply_smooth_handles(
        Keyframe& keyframe,
        const Keyframe* prev_keyframe_ptr,
        const Keyframe* next_keyframe_ptr,
        double smooth_factor = 1.0/3.0);

    /**
     * @brief Computes aligned handles (the HandleMode::Aligned family) for a keyframe.
     *
     * Keeps the in- and out-handles collinear through the keyframe. The
     * keyframe's HandleMode selects the exact behavior (strict symmetry vs.
     * independently clamped magnitudes), and @p grabbed_handle indicates which
     * handle drives the alignment.
     * @param keyframe The keyframe to update (modified in place).
     * @param prev_keyframe_ptr Previous keyframe, or nullptr if this is the first.
     * @param next_keyframe_ptr Next keyframe, or nullptr if this is the last.
     * @param grabbed_handle Which handle is the source of the alignment.
     */
    void apply_aligned_handles(
            Keyframe& keyframe,
            const Keyframe* prev_keyframe_ptr,
            const Keyframe* next_keyframe_ptr,
            GrabbedHandle grabbed_handle = GrabbedHandle::None);

} // namespace anim

#endif // ANIM_HANDLE_UTILS_HPP
