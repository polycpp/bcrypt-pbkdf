#pragma once

/**
 * @file
 * @brief Top-level aggregator for the polycpp `bcrypt-pbkdf` companion.
 *
 * Includes the public `pbkdf`/`bcryptHash` API and the typed `PbkdfError`
 * surface. Internal Blowfish state lives under
 * `include/polycpp/bcrypt_pbkdf/detail/` and is not part of the public API.
 *
 * @par What the algorithm does
 * `bcrypt_pbkdf` derives an arbitrary-length cryptographic key from a
 * password and salt by combining Blowfish (in OpenBSD's special expansion
 * mode) with SHA-512 as the inner hash and the constant string
 * `"OxychromaticBlowfishSwatDynamite"` as the cipher input. It is the key
 * derivation function OpenSSH `ssh-keygen` uses to derive cipher material
 * for encrypted new-format private keys.
 *
 * @par Compatibility
 * Output is byte-identical to the upstream npm `bcrypt-pbkdf@1.0.2`
 * package (Joyent revision `9ae4b4912099b2dc2ff38ab34934324523ab8025`)
 * for every recorded test vector. The C++ port replaces upstream's
 * integer return codes with typed exceptions and uses
 * `polycpp::crypto::createHash("sha512")` (OpenSSL) instead of
 * `tweetnacl` for SHA-512; see `docs/divergences.md` for details.
 *
 * @par Thread safety
 * All public functions are reentrant and thread-safe with respect to
 * each other. There is no shared global state — every call constructs
 * its own Blowfish state on the stack.
 *
 * @see polycpp::bcrypt_pbkdf::pbkdf
 * @see polycpp::bcrypt_pbkdf::bcryptHash
 * @see polycpp::bcrypt_pbkdf::PbkdfError
 */

#include <polycpp/bcrypt_pbkdf/bcrypt_pbkdf.hpp>
#include <polycpp/bcrypt_pbkdf/error.hpp>
