#ifndef ANIM_HPP
#define ANIM_HPP

/**
 * @file anim.hpp
 * @brief A modern C++ library for creating, managing, and evaluating animation channels.
 * 
 * This library provides a comprehensive set of tools for working with animation based on
 * Bézier curves and immutable keyframe objects. It supports various tangent modes, keyframe
 * manipulation, and evaluation of animation curves.
 * 
 * @mainpage Anim Library Documentation
 * 
 * @section intro_sec Introduction
 * 
 * The Anim library is a modern C++ animation toolkit designed for creating and 
 * manipulating animation curves with precision and flexibility. It provides a 
 * comprehensive set of tools for keyframe animation with support for various 
 * interpolation modes.
 * 
 * @section features_sec Key Features
 * 
 * - Multiple tangent modes (linear, flat, constant, smooth, etc.)
 * - Bézier curve interpolation between keyframes
 * - Flexible keyframe manipulation
 * - Named animation channels with intuitive API
 * - Animation composition with multiple channels
 * - Evaluation at specific times or time ranges
 * 
 * @section components_sec Main Components
 * 
 * - BezierHandle: Handle point for Bézier curve control
 * - Keyframe: Defines a value at a specific time with tangent information
 * - Channel: A sequence of keyframes representing a single animatable property
 * - Animation: Collection of channels forming a complete animation
 * - BezierUtils: Utilities for working with Bézier curves
 * 
 * @section usage_sec Basic Usage
 * 
 * ```cpp
 * // Create a channel for position X
 * anim::Channel position_x("position.x");
 * 
 * // Add keyframes
 * position_x.set_keyframe_at_time(0.0, 0.0, 
 *     anim::BezierHandle(-0.3, 0.0), anim::BezierHandle(0.3, 0.0), 
 *     anim::TangentMode::smooth);
 * position_x.set_keyframe_at_time(1.0, 10.0, 
 *     anim::BezierHandle(0.7, 10.0), anim::BezierHandle(1.3, 10.0), 
 *     anim::TangentMode::smooth);
 * 
 * // Evaluate at a specific time
 * double value = position_x.evaluate(0.5); // Get value at t=0.5
 * 
 * // Create an animation with multiple channels
 * anim::Animation animation;
 * animation.add_channel(position_x);
 * ```
 */

#include "anim/bezier_handle.hpp"
#include "anim/tangent_mode.hpp"
#include "anim/keyframe.hpp"
#include "anim/bezier_utils.hpp"
#include "anim/channel.hpp"
#include "anim/animation.hpp"

/**
 * @namespace anim
 * @brief The main namespace for the animation library.
 * 
 * All components of the anim library are contained within this namespace.
 */

#endif // ANIM_HPP
