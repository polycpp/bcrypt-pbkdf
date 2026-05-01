Quickstart
==========

Derive a 48-byte key (a 32-byte AES-256 key plus a 16-byte AES-CTR IV — the
shape OpenSSH uses for encrypted private keys) from a password and salt.

Full example
------------

.. code-block:: cpp

   #include <polycpp/bcrypt_pbkdf.hpp>
   #include <polycpp/buffer.hpp>

   #include <iostream>

   int main() {
       const polycpp::Buffer password =
           polycpp::Buffer::from("correct horse battery staple");
       const polycpp::Buffer salt =
           polycpp::Buffer::from("0123456789abcdef");

       const polycpp::Buffer derived =
           polycpp::bcrypt_pbkdf::pbkdf(password, salt, /*rounds=*/16, /*keylen=*/48);

       std::cout << "derived key length: " << derived.length() << " bytes\n";
       return 0;
   }

Build it with the CMake wiring from :doc:`installation`:

.. code-block:: bash

   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ./build/my_app

Expected output:

.. code-block:: text

   derived key length: 48 bytes

What just happened
------------------

``polycpp::bcrypt_pbkdf::pbkdf`` ran the OpenBSD ``bcrypt_pbkdf`` construction:

- SHA-512 of the password is computed once.
- For each output block of ``HASHSIZE`` (32) bytes, SHA-512 of the salt
  concatenated with a 4-byte big-endian counter is computed, then fed
  together with the SHA-512 of the password into the inner Blowfish-based
  hash step.
- That step is iterated ``rounds`` (16) times, XORing each iteration's
  output into the running block.
- Bytes are interleaved across blocks with a stride that matches the
  upstream construction byte for byte.

For ``rounds=16, keylen=48`` the call returns in well under 100 ms on a
typical desktop CPU. The work is CPU-only and synchronous; if you need to
move it off the main thread, wrap the call in ``std::async`` at the call
site.

Next steps
----------

- :doc:`../guides/decrypt-openssh-key` — how to use ``pbkdf`` to decrypt an
  OpenSSH new-format encrypted private key.
- :doc:`../api/bcrypt_pbkdf` — every public function and option.
- :doc:`../examples/index` — runnable programs you can drop into a sandbox.
