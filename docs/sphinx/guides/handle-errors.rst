How do I handle errors from ``pbkdf``?
=======================================

Every invalid input throws a typed
:cpp:class:`polycpp::bcrypt_pbkdf::PbkdfError` carrying a
:cpp:enum:`polycpp::bcrypt_pbkdf::PbkdfErrorCode` discriminator. There is no
sentinel return value; the C++ port replaces upstream's integer ``-1`` with
exceptions raised before any keying material is computed.

.. code-block:: cpp

   #include <polycpp/bcrypt_pbkdf.hpp>
   #include <polycpp/buffer.hpp>

   #include <iostream>

   try {
       auto key = polycpp::bcrypt_pbkdf::pbkdf(password, salt, /*rounds=*/0, 32);
   } catch (const polycpp::bcrypt_pbkdf::PbkdfError& e) {
       switch (e.code()) {
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::InvalidRounds:
               std::cerr << "rounds must be >= 1\n";
               break;
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::EmptyPassword:
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::EmptySalt:
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::EmptyKeylen:
               std::cerr << "missing required input\n";
               break;
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::KeylenTooLarge:
               std::cerr << "keylen exceeds 1024 bytes\n";
               break;
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::SaltTooLarge:
               std::cerr << "salt exceeds 1 MiB\n";
               break;
           case polycpp::bcrypt_pbkdf::PbkdfErrorCode::InvalidHashInput:
               std::cerr << "bcryptHash inputs must be exactly 64 bytes each\n";
               break;
       }
   }

The full list of codes is documented under :doc:`../api/error`.

You can also use :cpp:func:`polycpp::bcrypt_pbkdf::pbkdfErrorCodeName` to
turn a code into a stable string name (``"InvalidRounds"``, ``"EmptyPassword"``,
…) — useful for log lines.
