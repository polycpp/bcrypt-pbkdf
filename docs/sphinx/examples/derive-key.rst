``derive_key``: derive an OpenSSH-style key
============================================

Source: ``examples/derive_key.cpp``.

This example derives a 48-byte key (32 bytes for an AES-256 key plus 16 bytes
for an AES-CTR IV — the parameter shape OpenSSH uses for encrypted private
keys), prints its length, and prints the derived bytes as a hex string.

Building
--------

.. code-block:: bash

   cmake -S . -B build -G Ninja -DPOLYCPP_BCRYPT_PBKDF_BUILD_EXAMPLES=ON
   cmake --build build --target derive_key

Running
-------

.. code-block:: bash

   ./build/examples/derive_key

Expected output:

.. code-block:: text

   derived key length: 48 bytes
   derived key (hex): 94848f7f274c333fc71b71f46d7ae94e916dc549be8d530cd0bc5f64760394ee342d656e10ba6041c4358409a758aeac

Source listing
--------------

.. literalinclude:: ../../../examples/derive_key.cpp
   :language: cpp
