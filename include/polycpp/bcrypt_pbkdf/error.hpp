#pragma once

#include <polycpp/core/error.hpp>

#include <string>

namespace polycpp::bcrypt_pbkdf {

/**
 * @enum PbkdfErrorCode
 * @brief Discriminator for `PbkdfError` failure modes.
 *
 * The C++ port replaces upstream's integer return code (`-1` on bad
 * input, `0` on success) with a typed exception. Each invalid input
 * maps to one of these codes so callers can branch on the failure
 * cause without parsing the message string.
 *
 * Validation in `pbkdf` runs in this order, returning on the first
 * failure: rounds, password, salt, keylen, keylen-bound, salt-bound.
 * `bcryptHash` only reports `InvalidHashInput`. All checks fire
 * before any allocation or hashing, so the throw never produces a
 * partial output buffer.
 *
 * @see PbkdfError
 * @see pbkdf
 * @see bcryptHash
 * @see pbkdfErrorCodeName
 * @since 1.0.0
 */
enum class PbkdfErrorCode {
    /**
     * `rounds` was less than 1. Pick a value of 16 for OpenSSH-style
     * use, or higher if you want more KDF work per derived key.
     */
    InvalidRounds,
    /** `password` was empty. Pass at least one byte. */
    EmptyPassword,
    /** `salt` was empty. Pass at least one byte; ideally 16 random bytes. */
    EmptySalt,
    /** `keylen` was 0. Pass the size of the key you actually want. */
    EmptyKeylen,
    /**
     * `keylen` exceeded `MAX_KEYLEN` (1024 bytes). Matches upstream's
     * `HASHSIZE * HASHSIZE` limit. Real-world callers rarely need more
     * than 64 bytes (32-byte AES-256 key + 16-byte IV plus padding).
     */
    KeylenTooLarge,
    /** `salt.length()` exceeded `MAX_SALTLEN` (1 MiB). Use a 16-byte salt. */
    SaltTooLarge,
    /**
     * `bcryptHash` was called with an input that was not exactly 64
     * bytes — `bcryptHash` expects raw SHA-512 digests as input. If
     * you have a password and a salt rather than digests, call
     * `pbkdf` instead.
     */
    InvalidHashInput,
};

/**
 * @brief Returns the canonical name of a `PbkdfErrorCode` as a stable
 *        C string suitable for log lines.
 *
 * Returned strings are static and include only the enum-tag spelling
 * (`"InvalidRounds"`, `"EmptyPassword"`, …). They are guaranteed to
 * be non-empty and never `"Unknown"` for a valid enum value.
 *
 * @param code A `PbkdfErrorCode` value.
 * @return A static C string with the enum-tag name. Lifetime is the
 *         program's; the caller does not own and must not free.
 *
 * @par Example
 * @code{.cpp}
 *   try {
 *       polycpp::bcrypt_pbkdf::pbkdf(password, salt, 0, 32);
 *   } catch (const polycpp::bcrypt_pbkdf::PbkdfError& e) {
 *       LOG(WARNING) << "bcrypt_pbkdf failed: code="
 *                    << polycpp::bcrypt_pbkdf::pbkdfErrorCodeName(e.code())
 *                    << " message=" << e.what();
 *   }
 * @endcode
 *
 * @see PbkdfErrorCode
 * @since 1.0.0
 */
const char* pbkdfErrorCodeName(PbkdfErrorCode code) noexcept;

/**
 * @class PbkdfError
 * @brief Typed exception thrown by `pbkdf` and `bcryptHash` for
 *        invalid arguments.
 *
 * Inherits from `polycpp::Error` (which itself derives from
 * `std::exception`), so generic catch-blocks at the request boundary
 * still see the failure:
 *
 * @code{.cpp}
 *   try {
 *       auto key = polycpp::bcrypt_pbkdf::pbkdf(password, salt, rounds, keylen);
 *   } catch (const polycpp::bcrypt_pbkdf::PbkdfError& e) {
 *       // typed: e.code() is available
 *   } catch (const polycpp::Error& e) {
 *       // polycpp-shaped catch
 *   } catch (const std::exception& e) {
 *       // generic catch
 *   }
 * @endcode
 *
 * Carries a `PbkdfErrorCode` so callers can branch on the specific
 * failure mode. The C++ API never returns a sentinel error value —
 * every parameter validation failure is raised as an exception
 * before any keying material is computed.
 *
 * The exception's `name` field is set to `"PbkdfError"` so that
 * polycpp's stringified form (`"PbkdfError: <message>"`) round-trips
 * the type.
 *
 * @par Example
 * @code{.cpp}
 *   try {
 *       polycpp::bcrypt_pbkdf::pbkdf(password, salt, 0, 32);
 *   } catch (const polycpp::bcrypt_pbkdf::PbkdfError& e) {
 *       assert(e.code() == polycpp::bcrypt_pbkdf::PbkdfErrorCode::InvalidRounds);
 *   }
 * @endcode
 *
 * @see PbkdfErrorCode
 * @see pbkdfErrorCodeName
 * @since 1.0.0
 */
class PbkdfError : public polycpp::Error {
public:
    /**
     * @brief Constructs a `PbkdfError` with a code and a
     *        human-readable message.
     *
     * Most callers will catch `PbkdfError` rather than construct it.
     * Construction is exposed so downstream wrapper libraries can
     * adapt their own input-validation paths to throw the same type.
     *
     * @param code    Discriminator for the failure mode.
     * @param message Human-readable explanation of the failure;
     *                surfaces through `what()` and `message`.
     */
    PbkdfError(PbkdfErrorCode code, const std::string& message);

    /**
     * @brief Returns the typed error code.
     *
     * @return The `PbkdfErrorCode` for this exception. Use a switch
     *         statement to branch on the failure mode without
     *         parsing `what()`.
     */
    PbkdfErrorCode code() const noexcept { return code_; }

private:
    PbkdfErrorCode code_;
};

}  // namespace polycpp::bcrypt_pbkdf
