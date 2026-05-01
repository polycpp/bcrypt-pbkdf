#pragma once

/**
 * @file
 * @brief Top-level aggregator for the polycpp `bcrypt-pbkdf` companion.
 *
 * Includes the public `pbkdf`/`bcryptHash` API and the typed `PbkdfError`
 * surface. Internal Blowfish state lives under
 * `include/polycpp/bcrypt_pbkdf/detail/` and is not part of the public API.
 */

#include <polycpp/bcrypt_pbkdf/bcrypt_pbkdf.hpp>
#include <polycpp/bcrypt_pbkdf/error.hpp>
