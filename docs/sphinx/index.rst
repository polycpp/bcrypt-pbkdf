bcrypt-pbkdf
============

**C++ companion port of the OpenBSD bcrypt-pbkdf key derivation function.**

``bcrypt-pbkdf`` derives a cryptographic key from a password and salt using
the OpenBSD ``bcrypt_pbkdf`` construction (Blowfish + SHA-512). It is the key
derivation function OpenSSH ``ssh-keygen`` uses to encrypt new-format private
keys (the ``-----BEGIN OPENSSH PRIVATE KEY-----`` envelope).

.. code-block:: cpp

   #include <polycpp/bcrypt_pbkdf.hpp>
   #include <polycpp/buffer.hpp>

   auto password = polycpp::Buffer::from("correct horse battery staple");
   auto salt     = polycpp::Buffer::from("0123456789abcdef");
   auto key      = polycpp::bcrypt_pbkdf::pbkdf(password, salt, 16, 48);

.. grid:: 2

   .. grid-item-card:: Byte-identical to upstream
      :margin: 1

      Reproduces the exact output of npm ``bcrypt-pbkdf@1.0.2`` for every
      published test vector. Suitable for decrypting OpenSSH private keys
      produced by ``ssh-keygen``.

   .. grid-item-card:: C++20 native
      :margin: 1

      Synchronous typed API with ``polycpp::Buffer`` inputs and outputs.
      No callbacks, no Promises, no global state — concurrent calls are
      safe.

   .. grid-item-card:: Fail-closed by design
      :margin: 1

      Every invalid input throws a typed ``PbkdfError`` with a specific
      ``PbkdfErrorCode`` before any keying material is computed. No
      sentinel ``-1`` returns.

   .. grid-item-card:: Plays well with polycpp
      :margin: 1

      SHA-512 comes from ``polycpp::crypto`` (OpenSSL). Errors derive from
      ``polycpp::Error``. Buffers are ``polycpp::Buffer``. Sensitive
      scratch is wiped via ``polycpp::ssl::secureZero``.

What's here
-----------

- :doc:`installation` — pull the library into your CMake build.
- :doc:`usage` — derive a key, decrypt an OpenSSH private key, handle
  errors, and the performance and threading model.
- :doc:`api/index` — every public function, constant, type, and enum
  value, generated from the Doxygen comments in the public headers.

.. toctree::
   :hidden:

   installation
   usage
   api/index
