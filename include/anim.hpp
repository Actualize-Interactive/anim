#ifndef ANIM_HPP
#define ANIM_HPP

/**
 * @file anim.hpp
 * @brief Umbrella header for the anim animation-curve library.
 *
 * Including this single header pulls in the full public API: the anim::Animation
 * and anim::Channel containers, the anim::Keyframe value type, and the
 * Bézier evaluation helpers. Individual headers under @c anim/ may also be
 * included directly.
 */

#include "anim/keyframe.hpp"
#include "anim/bezier_utils.hpp"
#include "anim/channel.hpp"
#include "anim/animation.hpp"
#include "anim/extend.hpp"
#include "anim/range_end.hpp"

#endif // ANIM_HPP
