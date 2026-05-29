Introduction
============

**anim** is a modern C++20 library for creating, managing, and evaluating
animation curves built from keyframes and cubic Bézier interpolation.

Core concepts
-------------

``Animation``
   A named container that owns a set of :cpp:class:`anim::Channel` curves and
   is the only way to create them. It also carries an overall time range used
   for sampling.

``Channel``
   A named, time-ordered sequence of keyframes that can be evaluated as a
   curve. Each channel has a unique, immutable :cpp:struct:`anim::Id` that
   survives copies. Channels keep their keyframes sorted by time and apply the
   appropriate handle constraints automatically.

``Keyframe``
   A value at a point in time, plus the data needed to interpolate towards its
   neighbours: an interpolation :cpp:enum:`anim::Function`, a
   :cpp:enum:`anim::HandleMode`, and two Bézier handles.

``Point``
   The fundamental ``(time, value)`` coordinate type. It doubles as a 2D
   vector via the ``anim::Vector`` alias.

Interpolation and handles
-------------------------

Each keyframe chooses how the segment leaving it is interpolated via
:cpp:enum:`anim::Function`:

* ``Constant`` — hold the value until the next keyframe (step)
* ``Linear`` — straight-line interpolation
* ``Bezier`` — cubic Bézier interpolation driven by the handles

For Bézier segments, :cpp:enum:`anim::HandleMode` controls how the handles are
computed and constrained — from fully automatic (``Smooth``) through aligned
variants (``Aligned``, ``AlignStrict``, ``AlignFlex``, ``AlignAdjustable``) to
fully manual (``Free``).

Extending beyond the range
--------------------------

Times outside a channel's keyframe range are resolved per
:cpp:enum:`anim::Extend`, set independently for the start and end:

* ``Hold`` — clamp to the nearest end value
* ``Repeat`` — loop the range periodically
* ``Mirror`` — ping-pong the range
