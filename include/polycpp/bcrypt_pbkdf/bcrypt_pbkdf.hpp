#pragma once

#include <polycpp/bcrypt_pbkdf/error.hpp>
#include <polycpp/buffer.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @namespace polycpp::bcrypt_pbkdf
 * @brief OpenBSD bcrypt_pbkdf compatibility functions, constants, and errors.
 */
namespace polycpp::bcrypt_pbkdf {

/**
 * @brief Number of 64-bit Blowfish blocks in the inner cipher state.
 *
 * Same value as upstream `bcrypt-pbkdf`'s `BLOCKS` export. Exposed so
 * callers writing parity tests can refer to the same constant the
 * algorithm uses internally.
 *
 * @since 1.0.0
 */
inline constexpr std::size_t BLOCKS = 8;

/**
 * @brief Byte length of `bcryptHash`'s output, in bytes.
 *
 * Same value as upstream `bcrypt-pbkdf`'s `HASHSIZE` export. The inner
 * `bcryptHash` step always emits exactly this many bytes; the outer
 * `pbkdf` driver stitches multiple `HASHSIZE`-byte blocks together to
 * reach the requested `keylen`.
 *
 * @since 1.0.0
 */
inline constexpr std::size_t HASHSIZE = 32;

/**
 * @brief Maximum permitted derived key length, in bytes (= 1024).
 *
 * Matches upstream's check `keylen > out.byteLength * out.byteLength`
 * where `out.byteLength == HASHSIZE`. Inputs above this throw
 * `PbkdfError` with code `PbkdfErrorCode::KeylenTooLarge`.
 * @since 1.0.0
 */
inline constexpr std::size_t MAX_KEYLEN = HASHSIZE * HASHSIZE;

/**
 * @brief Maximum permitted salt length, in bytes (= 1 MiB).
 *
 * Matches upstream's check `saltlen > (1 << 20)`. Inputs above this
 * throw `PbkdfError` with code `PbkdfErrorCode::SaltTooLarge`.
 * @since 1.0.0
 */
inline constexpr std::size_t MAX_SALTLEN = std::size_t{1} << 20;

/**
 * @brief Derives a key of `keylen` bytes from `password` and `salt`
 *        using OpenBSD's `bcrypt_pbkdf` construction.
 *
 * Mirrors the behavior of upstream npm `bcrypt-pbkdf@1.0.2`'s
 * `pbkdf(pass, passlen, salt, saltlen, key, keylen, rounds)`. The output
 * is byte-identical to upstream for any
 * `(password, salt, rounds, keylen)` tuple within the documented bounds.
 *
 * The C++ form removes the in/out `key` argument and the explicit
 * length arguments — `polycpp::Buffer` already carries length and the
 * derived key is returned. The upstream integer return code (`-1` on
 * failure, `0` on success) is replaced by `PbkdfError` exceptions; this
 * function never returns on invalid input, and never produces a
 * partial output buffer.
 *
 * @param password Byte sequence used as the password input. Must be
 *                 non-empty. Treated as opaque bytes; embedded NULs are
 *                 part of the password and no encoding conversion or
 *                 Unicode normalization is performed.
 * @param salt     Byte sequence used as the salt input. Must be
 *                 non-empty and no larger than `MAX_SALTLEN` bytes.
 * @param rounds   Number of bcrypt rounds. Must be at least 1. OpenSSH
 *                 uses 16 (or 24 for some older keys); use a similar
 *                 range when picking your own value.
 * @param keylen   Length of the returned key in bytes. Must be at least
 *                 1 and no larger than `MAX_KEYLEN` (1024) bytes.
 * @return A `polycpp::Buffer` of exactly `keylen` bytes.
 *
 * @throws PbkdfError if any input is out of range. The thrown exception
 *         carries a typed `PbkdfErrorCode` so callers can branch on the
 *         specific failure mode without parsing the message string.
 *
 * Validation order before allocation or hashing:
 *
 * @code
 * rounds < 1                    -> PbkdfErrorCode::InvalidRounds
 * password.length() == 0        -> PbkdfErrorCode::EmptyPassword
 * salt.length() == 0            -> PbkdfErrorCode::EmptySalt
 * keylen == 0                   -> PbkdfErrorCode::EmptyKeylen
 * keylen > MAX_KEYLEN           -> PbkdfErrorCode::KeylenTooLarge
 * salt.length() > MAX_SALTLEN   -> PbkdfErrorCode::SaltTooLarge
 * @endcode
 *
 * The function stops at the first failure. A throw never leaks a partial
 * output buffer or partial keying material.
 *
 * @par Thread safety
 * Reentrant. The Blowfish keystream pointer is per-instance, so
 * concurrent `pbkdf` calls on different threads are independent.
 *
 * @par Complexity
 * Roughly `O(rounds * stride)` SHA-512 + `bcryptHash` invocations,
 * where `stride = ceil(keylen / HASHSIZE)`. For OpenSSH's typical
 * shape (16 rounds, 48-byte key) this returns in well under 100 ms on
 * a typical desktop CPU. The work is CPU-only and synchronous; if you
 * need to move it off the main thread, wrap the call in `std::async`
 * at the call site.
 *
 * @par Security
 * Internal scratch buffers (`SHA-512(password)`, the per-block
 * intermediate hashes) are scrubbed on every exit path — including
 * exception unwinding — using `polycpp::ssl::secureZero`, which
 * delegates to `OPENSSL_cleanse` and is documented not to be elided
 * by the optimizer. The returned `polycpp::Buffer` is the caller's;
 * scrub it with `polycpp::ssl::secureZero(key.data(), key.length())`
 * from `<polycpp/ssl/memory.hpp>` when no longer needed.
 *
 * @par Example: derive an OpenSSH cipher key + IV
 * @code{.cpp}
 *   // 32-byte AES-256 key + 16-byte AES-CTR IV = 48 bytes
 *   auto password = polycpp::Buffer::from("correct horse battery staple");
 *   auto salt     = polycpp::Buffer::from("0123456789abcdef");
 *   auto key      = polycpp::bcrypt_pbkdf::pbkdf(password, salt, 16, 48);
 *   // key.data()[0..32) is the AES key; key.data()[32..48) is the IV.
 * @endcode
 * @since 1.0.0
 */
polycpp::Buffer pbkdf(const polycpp::Buffer& password,
                      const polycpp::Buffer& salt,
                      std::uint32_t rounds,
                      std::uint32_t keylen);

/**
 * @brief Convenience overload that accepts password bytes in `std::string`.
 *
 * Treats the string's bytes as the password. **No re-encoding is
 * performed** — the bytes are passed through unchanged, including
 * embedded NULs. `bcrypt_pbkdf` is byte-oriented and will produce
 * different keys for different encodings or Unicode normalization forms
 * of the same displayed password.
 *
 * Use this overload when the password is already a `std::string` and
 * you want to skip the explicit `polycpp::Buffer::from` wrap.
 *
 * @param password Password as a byte string.
 * @param salt     Salt as bytes.
 * @param rounds   Number of bcrypt rounds.
 * @param keylen   Output key length in bytes.
 * @return A `polycpp::Buffer` of exactly `keylen` bytes.
 * @throws PbkdfError on invalid input. See the `Buffer` overload for
 *         the full table of error codes.
 * @since 1.0.0
 */
polycpp::Buffer pbkdf(const std::string& password,
                      const polycpp::Buffer& salt,
                      std::uint32_t rounds,
                      std::uint32_t keylen);

/**
 * @brief Inner Blowfish-based hash step used by `pbkdf`.
 *
 * Mirrors the upstream `bcrypt-pbkdf.hash(sha2pass, sha2salt, out)`
 * function. Both inputs must be exactly 64 bytes (the size of a
 * SHA-512 digest); the returned buffer is always exactly `HASHSIZE`
 * (32) bytes.
 *
 * Most callers should use `pbkdf` instead. `bcryptHash` is exposed
 * primarily for:
 *
 * - parity testing against the upstream JS oracle, and
 * - downstream callers that have already pre-computed
 *   `SHA-512(password)` and `SHA-512(salt || counter)` and want to
 *   build a non-standard outer driver on top of the inner step.
 *
 * @param sha2pass The 64-byte SHA-512 digest of the password.
 * @param sha2salt The 64-byte SHA-512 digest of the salt-with-counter.
 * @return A `HASHSIZE`-byte `polycpp::Buffer`.
 * @throws PbkdfError with code `PbkdfErrorCode::InvalidHashInput` if
 *         either input is not exactly 64 bytes.
 *
 * @par Thread safety
 * Reentrant; constructs a fresh Blowfish state on every call.
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
 *   // inner.length() == HASHSIZE
 * @endcode
 * @since 1.0.0
 */
polycpp::Buffer bcryptHash(const polycpp::Buffer& sha2pass,
                           const polycpp::Buffer& sha2salt);

}  // namespace polycpp::bcrypt_pbkdf
