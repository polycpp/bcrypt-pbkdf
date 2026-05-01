Examples
========

Self-contained programs exercising the main features of bcrypt-pbkdf. Each
example compiles against the public API only — no private headers, no
non-exported targets.

.. toctree::
   :maxdepth: 1

   derive-key

Running an example
------------------

From the repository root:

.. code-block:: bash

   cmake -S . -B build -G Ninja -DPOLYCPP_BCRYPT_PBKDF_BUILD_EXAMPLES=ON
   cmake --build build --target derive_key
   ./build/examples/derive_key
