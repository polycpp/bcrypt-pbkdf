``polycpp/bcrypt_pbkdf/bcrypt_pbkdf.hpp``
=========================================

The ``pbkdf`` and ``bcryptHash`` functions plus the public numeric constants.

Functions
---------

.. doxygenfunction:: polycpp::bcrypt_pbkdf::pbkdf(const polycpp::Buffer&, const polycpp::Buffer&, std::uint32_t, std::uint32_t)

.. doxygenfunction:: polycpp::bcrypt_pbkdf::pbkdf(const std::string&, const polycpp::Buffer&, std::uint32_t, std::uint32_t)

.. doxygenfunction:: polycpp::bcrypt_pbkdf::bcryptHash

Constants
---------

.. doxygenvariable:: polycpp::bcrypt_pbkdf::BLOCKS

.. doxygenvariable:: polycpp::bcrypt_pbkdf::HASHSIZE

.. doxygenvariable:: polycpp::bcrypt_pbkdf::MAX_KEYLEN

.. doxygenvariable:: polycpp::bcrypt_pbkdf::MAX_SALTLEN
