#pragma once

#include <polycpp/bcrypt_pbkdf/error.hpp>
#include <polycpp/buffer.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

namespace polycpp::bcrypt_pbkdf {

/**
 * @brief Number of 64-bit Blowfish blocks in the inner cipher state.
 *
 * Same value as upstream `bcrypt-pbkdf`'s `BLOCKS` export.
 */
inline constexpr std::size_t BLOCKS = 8;

/**
 * @brief Byte length of `bcryptHash`'s output.
 *
 * Same value as upstream `bcrypt-pbkdf`'s `HASHSIZE` export.
 */
inline constexpr std::size_t HASHSIZE = 32;

/**
 * @brief Maximum permitted derived key length, in bytes.
 *
 * Matches upstream's check `keylen > out.byteLength * out.byteLength` where
 * `out.byteLength == HASHSIZE`. Inputs above this throw `PbkdfError` with code
 * `PbkdfErrorCode::KeylenTooLarge`.
 */
inline constexpr std::size_t MAX_KEYLEN = HASHSIZE * HASHSIZE;

/**
 * @brief Maximum permitted salt length, in bytes.
 *
 * Matches upstream's check `saltlen > (1 << 20)`. Inputs above this throw
 * `PbkdfError` with code `PbkdfErrorCode::SaltTooLarge`.
 */
inline constexpr std::size_t MAX_SALTLEN = std::size_t{1} << 20;

/**
 * @brief Derives a key of `keylen` bytes from `password` and `salt` using
 *        OpenBSD's `bcrypt_pbkdf` construction.
 *
 * Mirrors the behavior of the upstream npm `bcrypt-pbkdf@1.0.2`
 * `pbkdf(pass, passlen, salt, saltlen, key, keylen, rounds)` function. The
 * output is byte-identical to upstream for any `(password, salt, rounds, keylen)`
 * tuple.
 *
 * The C++ form removes the in/out `key` argument and the explicit length
 * arguments — `polycpp::Buffer` already carries length and the derived key is
 * returned. The upstream integer return code (`-1` on failure) is replaced by
 * `PbkdfError` exceptions; this function never returns on invalid input.
 *
 * @param password Byte sequence used as the password input. Must be non-empty.
 * @param salt     Byte sequence used as the salt input. Must be non-empty and
 *                 no larger than `MAX_SALTLEN` bytes.
 * @param rounds   Number of bcrypt rounds. Must be at least 1.
 * @param keylen   Length of the returned key in bytes. Must be at least 1 and
 *                 no larger than `MAX_KEYLEN` bytes.
 * @return A `polycpp::Buffer` of exactly `keylen` bytes.
 * @throws PbkdfError if any input is out of range. The thrown exception
 *         carries a typed `PbkdfErrorCode`.
 *
 * @par Example
 * @code{.cpp}
 *   auto password = polycpp::Buffer::from("correct horse battery staple");
 *   auto salt     = polycpp::Buffer::from("0123456789abcdef");
 *   auto key      = polycpp::bcrypt_pbkdf::pbkdf(password, salt, 16, 48);
 * @endcode
 */
polycpp::Buffer pbkdf(const polycpp::Buffer& password,
                      const polycpp::Buffer& salt,
                      std::uint32_t rounds,
                      std::uint32_t keylen);

/**
 * @brief Convenience overload that accepts ASCII-style passwords as
 *        `std::string`.
 *
 * Treats the string's bytes as the password. No re-encoding is performed; the
 * bytes are passed through unchanged.
 *
 * @param password Password as a byte string.
 * @param salt     Salt as bytes.
 * @param rounds   Number of bcrypt rounds.
 * @param keylen   Output key length in bytes.
 * @return A `polycpp::Buffer` of exactly `keylen` bytes.
 * @throws PbkdfError on invalid input.
 */
polycpp::Buffer pbkdf(const std::string& password,
                      const polycpp::Buffer& salt,
                      std::uint32_t rounds,
                      std::uint32_t keylen);

/**
 * @brief Inner Blowfish-based hash step used by `pbkdf`.
 *
 * Mirrors the upstream `bcrypt-pbkdf.hash(sha2pass, sha2salt, out)` function.
 * Both inputs must be exactly 64 bytes (the size of a SHA-512 digest); the
 * returned buffer is always exactly `HASHSIZE` (32) bytes.
 *
 * Exposed primarily for parity testing and for callers that have already
 * pre-computed SHA-512 of password and salt. Most callers should use `pbkdf`.
 *
 * @param sha2pass The 64-byte SHA-512 digest of the password.
 * @param sha2salt The 64-byte SHA-512 digest of the salt-with-counter.
 * @return A `HASHSIZE`-byte `polycpp::Buffer`.
 * @throws PbkdfError with code `PbkdfErrorCode::InvalidHashInput` if either
 *         input is not exactly 64 bytes.
 *
 * @par Example
 * @code{.cpp}
 *   auto sha2pass = polycpp::crypto::createHash("sha512")
 *                       .update(polycpp::Buffer::from("password"))
 *                       .digestBuffer();
 *   auto sha2salt = polycpp::crypto::createHash("sha512")
 *                       .update(polycpp::Buffer::from("salt"))
 *                       .digestBuffer();
 *   auto inner = polycpp::bcrypt_pbkdf::bcryptHash(sha2pass, sha2salt);
 * @endcode
 */
polycpp::Buffer bcryptHash(const polycpp::Buffer& sha2pass,
                           const polycpp::Buffer& sha2salt);

}  // namespace polycpp::bcrypt_pbkdf
