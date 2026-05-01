Examples
========

Self-contained programs exercising the main features of bcrypt-pbkdf. Each
released example must compile against the public API only - no private
headers, no non-exported targets.

.. toctree::
   :maxdepth: 1

   planned

Running an example
------------------

From the repository root:

.. code-block:: bash

   cmake -S . -B build -G Ninja -DPOLYCPP_BCRYPT_PBKDF_BUILD_EXAMPLES=ON
   cmake --build build --target <example_target>
   ./build/examples/<example_binary>

Replace ``<example_target>`` and ``<example_binary>`` with the concrete
target and binary names from the example page.
