Usage
=====

This page covers the only thing this library does: derive a key from a
password and a salt using the OpenBSD ``bcrypt_pbkdf`` construction. The
two main use cases are decrypting OpenSSH-format private keys and any
application that wants the OpenBSD KDF behavior specifically.

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

   // 16 rounds, 48-byte output (32-byte AES-256 key + 16-byte AES-CTR IV).
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

   auto derived = polycpp::bcrypt_pbkdf::pbkdf(std::string("password"),
                                               salt, 16, 48);

The string's bytes are passed through unchanged — no UTF-16 re-encoding,
no NUL terminator handling. Pick this overload when the password is
already in a ``std::string`` and skip the explicit ``Buffer::from`` wrap.

Deriving an OpenSSH private-key cipher key
------------------------------------------

OpenSSH ``ssh-keygen`` encrypts new-format private keys
(``-----BEGIN OPENSSH PRIVATE KEY-----``) with a symmetric cipher
(``aes256-ctr`` by default) keyed by ``bcrypt_pbkdf``. The key file
embeds:

- ``kdf == "bcrypt"``
- ``kdfoptions == { salt: <16 bytes>, rounds: <N> }`` (typically 16)
- ``ciphername == "aes256-ctr"`` (32-byte key + 16-byte IV = 48 bytes)

To decrypt a key produced by ``ssh-keygen``:

.. code-block:: cpp

   #include <polycpp/bcrypt_pbkdf.hpp>
   #include <polycpp/buffer.hpp>

   polycpp::Buffer derive_aes256_ctr_key_iv(const polycpp::Buffer& password,
                                            const polycpp::Buffer& salt,
                                            std::uint32_t rounds) {
       return polycpp::bcrypt_pbkdf::pbkdf(password, salt, rounds, /*keylen=*/48);
   }

The first 32 bytes of the returned buffer are the AES-256 key; the
remaining 16 bytes are the AES-CTR IV. Pass them into your AES decrypt
primitive (for example ``polycpp::crypto::createDecipheriv("aes-256-ctr",
key, iv)``) to decrypt the key blob.

.. note::

   ``rounds`` for ``ssh-keygen``-produced keys is typically 16 (the
   default since OpenSSH 9.5). Some older keys use 24. Always read the
   value from the key file rather than hard-coding it.

.. note::

   The salt length in OpenSSH keys is 16 bytes. ``bcrypt_pbkdf`` itself
   accepts up to 1 MiB (``MAX_SALTLEN``); the public OpenSSH format uses
   exactly 16.

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
