Examples
========

With ``ANIM_BUILD_EXAMPLES`` enabled (the default for a standalone build), the
``examples/`` directory builds two programs.

test_anim_integration
----------------------

A minimal, dependency-free program that exercises the core API — creating an
animation and channel, adding keyframes, querying the time range, evaluating,
and modifying keyframes. It is the best starting point for seeing the library
used end to end:

.. code-block:: cpp

   anim::Animation animation("Test Animation");
   anim::Channel& channel = animation.create_channel("TestChannel");

   channel.create_keyframe(0.0, 0.0);
   channel.create_keyframe(1.0, 1.0);
   channel.create_keyframe(2.0, 4.0);

   double value_at_1_5 = channel.evaluate(1.5);

curve_visualization
-------------------

An interactive viewer built on ImGui/ImPlot for inspecting curves and handle
modes visually. It demonstrates sampling a channel with ``evaluate_range`` and
plotting the result.

Building and running
--------------------

.. code-block:: bash

   cmake -B build -S . -DANIM_BUILD_EXAMPLES=ON
   cmake --build build
   # binaries are placed under build/examples/
