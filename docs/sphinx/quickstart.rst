Quick Start
===========

Include the umbrella header (or the individual headers under ``anim/``):

.. code-block:: cpp

   #include <anim.hpp>

Creating and evaluating a channel
---------------------------------

Channels are created and owned by an ``Animation``; you do not construct them
directly. Add keyframes with ``create_keyframe``, then evaluate the curve.

.. code-block:: cpp

   #include <anim.hpp>
   #include <iostream>

   int main() {
       anim::Animation animation("demo");

       // Channels are created through the animation, which keeps them sorted
       // and assigns each a unique id.
       anim::Channel& channel = animation.create_channel("value");

       // create_keyframe(time, value, function, handle_mode)
       channel.create_keyframe(0.0, 0.0, anim::Function::Linear);
       channel.create_keyframe(1.0, 1.0);                            // default: Bezier / Smooth
       channel.create_keyframe(2.0, 0.0, anim::Function::Bezier,
                                          anim::HandleMode::Flat);

       // Evaluate single times
       std::cout << "t=0.5 -> " << channel.evaluate(0.5) << "\n";
       std::cout << "t=1.5 -> " << channel.evaluate(1.5) << "\n";

       // Evaluate a range: 5 evenly spaced samples from t=0 to t=2
       for (double v : channel.evaluate_range(0.0, 2.0, 5)) {
           std::cout << v << " ";
       }
       std::cout << "\n";
       return 0;
   }

Keyframes with explicit Bézier handles
--------------------------------------

Handles are ``anim::Point`` values in ``(time, value)`` space:

.. code-block:: cpp

   anim::Channel& ch = animation.create_channel("curve");

   // create_keyframe(time, value, in_handle, out_handle, function, handle_mode)
   ch.create_keyframe(0.0, 0.0,
       anim::Point(-0.3, 0.0),   // in-handle
       anim::Point( 0.3, 0.0),   // out-handle
       anim::Function::Bezier,
       anim::HandleMode::Aligned);

Multiple channels
-----------------

.. code-block:: cpp

   anim::Animation transform("transform");

   anim::Channel& pos_x = transform.create_channel("position.x");
   anim::Channel& pos_y = transform.create_channel("position.y");

   pos_x.create_keyframe(0.0, 0.0, anim::Function::Linear);
   pos_x.create_keyframe(1.0, 10.0, anim::Function::Linear);

   pos_y.create_keyframe(0.0, 0.0);
   pos_y.create_keyframe(1.0, 5.0);

   double x = transform.channel("position.x").evaluate(0.5);
   double y = transform.channel("position.y").evaluate(0.5);

Extending beyond the keyframe range
-----------------------------------

Each channel decides how to evaluate times before its first keyframe and after
its last:

.. code-block:: cpp

   anim::Channel& ch = animation.create_channel("looping");
   ch.create_keyframe(0.0, 0.0, anim::Function::Linear);
   ch.create_keyframe(1.0, 1.0, anim::Function::Linear);

   ch.set_extend_start(anim::Extend::Repeat);  // loop before the start
   ch.set_extend_end(anim::Extend::Mirror);    // ping-pong after the end
