Installation
============

Requirements
------------

* A C++20-compatible compiler (MSVC 2022, GCC 11+, Clang 14+)
* CMake 3.25 or newer

Building from source
--------------------

Helper scripts configure, build, and run the tests:

.. code-block:: bash

   ./build.sh        # Linux / macOS

.. code-block:: powershell

   .\build.ps1       # Windows (PowerShell)

Or drive CMake directly:

.. code-block:: bash

   cmake -B build -S . -DANIM_BUILD_TESTS=ON
   cmake --build build
   ctest --test-dir build

Using anim in your project
--------------------------

As a CMake subdirectory:

.. code-block:: cmake

   add_subdirectory(path/to/anim)
   target_link_libraries(your_target PRIVATE anim)

Via an installed package:

.. code-block:: cmake

   find_package(anim REQUIRED)
   target_link_libraries(your_target PRIVATE anim::anim_static)  # or anim::anim_shared

CMake options
-------------

.. list-table::
   :header-rows: 1
   :widths: 30 30 40

   * - Option
     - Default
     - Description
   * - ``ANIM_BUILD_STATIC``
     - ``ON``
     - Build the static library.
   * - ``ANIM_BUILD_SHARED``
     - ``OFF``
     - Build the shared library.
   * - ``ANIM_BUILD_TESTS``
     - ``ON`` standalone / ``OFF`` submodule
     - Build the test suite.
   * - ``ANIM_BUILD_EXAMPLES``
     - ``ON`` standalone / ``OFF`` submodule
     - Build the examples.

When ``anim`` is added with ``add_subdirectory``, tests and examples default
to ``OFF`` so they do not build as part of your project.
