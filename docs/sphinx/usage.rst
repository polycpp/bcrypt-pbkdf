Usage
=====

This page covers the only thing this library does: derive a key from a
password and a salt using the OpenBSD ``bcrypt_pbkdf`` construction. The
main use case is compatibility with OpenSSH/OpenBSD data formats that
already require this exact KDF.

Deriving a key
--------------

The signature is:

.. code-block:: cpp

   polycpp::Buffer
   polycpp::bcrypt_pbkdf::pbkdf(const polycpp::Buffer& password,
                                const polycpp::Buffer& salt,
                                std::uint32_t rounds,
                                std::uint32_t keylen);

A minimal call:

.. code-block:: cpp

   #include <polycpp/bcrypt_pbkdf.hpp>
   #include <polycpp/buffer.hpp>

   auto password = polycpp::Buffer::from("correct horse battery staple");
   auto salt     = polycpp::Buffer::from("0123456789abcdef");

   // Demo parameters only. Read or generate salt/rounds for real use.
   auto derived = polycpp::bcrypt_pbkdf::pbkdf(password, salt,
                                               /*rounds=*/16,
                                               /*keylen=*/48);

What the call does, in order:

1. SHA-512 of the password (computed once, scrubbed before return).
2. For each ``HASHSIZE``-byte (32-byte) output block: SHA-512 of
   ``salt || counter`` (4-byte big-endian counter), then the inner
   Blowfish-based hash step (``bcryptHash``) applied to that and the
   password digest.
3. The inner step is iterated ``rounds`` times, XORing each iteration's
   output into the running block.
4. Output bytes are interleaved across blocks with a stride that matches
   the upstream construction byte for byte.

There is a convenience overload that takes ``std::string``:

.. code-block:: cpp

   #include <polycpp/bcrypt_pbkdf.hpp>
   #include <polycpp/buffer.hpp>
   #include <string>

   std::string password("pass\0word", 9);
   auto salt = polycpp::Buffer::from("0123456789abcdef");
   auto derived = polycpp::bcrypt_pbkdf::pbkdf(password,
                                               salt, 16, 48);

The ``std::string`` is treated as raw bytes. Embedded NUL bytes are part
of the password, and no encoding conversion or Unicode normalization is
performed. Different byte encodings or normalization forms for the same
displayed password produce different derived keys.

Deriving OpenSSH private-key cipher material
--------------------------------------------

OpenSSH ``ssh-keygen`` encrypts new-format private keys
(``-----BEGIN OPENSSH PRIVATE KEY-----``) with a symmetric cipher
(``aes256-ctr`` by default) keyed by material derived with
``bcrypt_pbkdf``. This library derives that key material only. It does
not parse an OpenSSH private-key envelope, decrypt the encrypted private
blob, validate checkints, or decode the private-key fields.

After another parser has read the OpenSSH envelope, use the embedded
values:

- ``kdf == "bcrypt"``
- ``kdfoptions == { salt: <16 bytes>, rounds: <N> }``
- ``ciphername`` to determine ``keylen == cipher key length + IV length``

For ``aes256-ctr``:

.. code-block:: cpp

   #include <polycpp/bcrypt_pbkdf.hpp>
   #include <polycpp/buffer.hpp>
   #include <polycpp/ssl/memory.hpp>

   struct Aes256CtrMaterial {
       polycpp::Buffer key;
       polycpp::Buffer iv;
   };

   Aes256CtrMaterial derive_aes256_ctr_material(const polycpp::Buffer& password,
                                                const polycpp::Buffer& salt,
                                                std::uint32_t rounds) {
       auto derived = polycpp::bcrypt_pbkdf::pbkdf(password, salt,
                                                   rounds, /*keylen=*/48);
       Aes256CtrMaterial material{
           derived.subarray(0, 32),   // AES-256 key
           derived.subarray(32, 48),  // AES-CTR IV
       };

       polycpp::ssl::secureZero(derived.data(), derived.length());
       return material;
   }

Pass the returned key and IV buffers to your AES-256-CTR decrypt primitive
to decrypt the OpenSSH encrypted private-key blob.

Common OpenSSH cipher material lengths:

.. list-table::
   :header-rows: 1

   * - ``ciphername``
     - Key bytes
     - IV bytes
     - ``pbkdf`` ``keylen``
   * - ``aes128-ctr``
     - 16
     - 16
     - 32
   * - ``aes192-ctr``
     - 24
     - 16
     - 40
   * - ``aes256-ctr``
     - 32
     - 16
     - 48
   * - ``aes256-cbc``
     - 32
     - 16
     - 48

.. note::

   Always read ``salt`` and ``rounds`` from the OpenSSH key file. The
   fixed values shown above are examples, not real key-file parameters.
   `OpenSSH ssh-keygen(1) <https://man.openbsd.org/ssh-keygen.1>`_
   documents ``ssh-keygen -a`` as defaulting to 16 KDF rounds when saving
   a private key.

.. note::

   The salt length in OpenSSH keys is 16 bytes. ``bcrypt_pbkdf`` itself
   accepts up to 1 MiB (``MAX_SALTLEN``); the public OpenSSH format uses
   exactly 16.

Security notes
--------------

- Fixed passwords and salts in examples are demo-only. For new
  compatibility formats, generate fresh random salts with a CSPRNG,
  typically 16 bytes.
- For OpenSSH private keys, read ``salt`` and ``rounds`` from the file;
  do not hard-code either value.
- If the OpenSSH file is untrusted, reject unreasonable ``rounds`` before
  calling ``pbkdf`` so a crafted file cannot force excessive CPU work.
  Use a configured cap appropriate for your service.
- Do not use this library as a general password database hash unless
  OpenBSD/OpenSSH ``bcrypt_pbkdf`` compatibility is specifically required.
- The library scrubs internal scratch buffers. The returned
  ``polycpp::Buffer`` is caller-owned key material and must be scrubbed by
  the caller when no longer needed.

Use ``polycpp::ssl::secureZero`` from ``<polycpp/ssl/memory.hpp>`` for
returned key buffers and any split copies:

.. code-block:: cpp

   #include <polycpp/bcrypt_pbkdf.hpp>
   #include <polycpp/buffer.hpp>
   #include <polycpp/ssl/memory.hpp>

   auto derived = polycpp::bcrypt_pbkdf::pbkdf(password, salt, rounds, 48);
   auto key = derived.subarray(0, 32);
   auto iv  = derived.subarray(32, 48);

   // ... use key and iv ...

   polycpp::ssl::secureZero(derived.data(), derived.length());
   polycpp::ssl::secureZero(key.data(), key.length());
   polycpp::ssl::secureZero(iv.data(), iv.length());

Error handling
--------------

Every invalid input throws a typed
:cpp:class:`polycpp::bcrypt_pbkdf::PbkdfError` carrying a
:cpp:enum:`polycpp::bcrypt_pbkdf::PbkdfErrorCode`. The C++ port replaces
upstream's integer ``-1`` with exceptions raised before any keying
material is computed, so a partial buffer never escapes.

.. code-block:: cpp

   try {
       auto key = polycpp::bcrypt_pbkdf::pbkdf(password, salt, /*rounds=*/0, 32);
   } catch (const polycpp::bcrypt_pbkdf::PbkdfError& e) {
       switch (e.code()) {
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::InvalidRounds:
               // rounds < 1
               break;
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::EmptyPassword:
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::EmptySalt:
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::EmptyKeylen:
               // any of the three required inputs is empty
               break;
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::KeylenTooLarge:
               // keylen > 1024
               break;
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::SaltTooLarge:
               // salt.length() > 1 MiB
               break;
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::InvalidHashInput:
               // bcryptHash inputs were not exactly 64 bytes each
               break;
       }
   }

``PbkdfError`` derives from ``polycpp::Error`` (and therefore
``std::exception``), so generic logging code at the request boundary
sees the failure too:

.. code-block:: cpp

   try {
       auto key = polycpp::bcrypt_pbkdf::pbkdf(password, salt, rounds, keylen);
   } catch (const std::exception& e) {
       LOG(WARNING) << "bcrypt_pbkdf failed: " << e.what();
       throw;
   }

For log lines that record the failure mode without parsing
``e.what()``, use
:cpp:func:`polycpp::bcrypt_pbkdf::pbkdfErrorCodeName` to turn a code
into a stable string (``"InvalidRounds"``, ``"EmptyPassword"``, …).

The full table of codes is documented in
:doc:`api/error`.

Performance notes
-----------------

The work is CPU-only and synchronous. There is no callback variant and no
``std::async`` wrapper baked into the API; if you need to move the call
off the main thread, wrap it at the call site.

For the parameter ranges OpenSSH actually uses (16-32 rounds, 32-64-byte
keys), a single ``pbkdf`` call returns in well under 100 ms on a typical
desktop CPU. The cost is roughly ``O(rounds * stride)`` SHA-512 +
bcrypt_hash invocations, where ``stride = ceil(keylen / HASHSIZE)``.

Concurrent ``pbkdf`` calls on different threads are independent: the
Blowfish keystream pointer is per-instance, not shared. This is exercised
by ``Pbkdf.ThreadSafety`` in the test suite.
