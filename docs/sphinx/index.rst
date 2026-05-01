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
      ``polycpp::Error``. Buffers are ``polycpp::Buffer``.

Getting started
---------------

.. code-block:: bash

   # With FetchContent (recommended)
   FetchContent_Declare(
       polycpp_bcrypt_pbkdf
       GIT_REPOSITORY https://github.com/polycpp/bcrypt-pbkdf.git
       GIT_TAG        master
   )
   FetchContent_MakeAvailable(polycpp_bcrypt_pbkdf)
   target_link_libraries(my_app PRIVATE polycpp::bcrypt_pbkdf)

:doc:`Installation <getting-started/installation>` | :doc:`Quickstart <getting-started/quickstart>` | :doc:`Guides <guides/index>` | :doc:`API reference <api/index>` | :doc:`Examples <examples/index>`

.. toctree::
   :hidden:
   :caption: Getting started

   getting-started/installation
   getting-started/quickstart

.. toctree::
   :hidden:
   :caption: How-to guides

   guides/index

.. toctree::
   :hidden:
   :caption: API reference

   api/index

.. toctree::
   :hidden:
   :caption: Examples

   examples/index
