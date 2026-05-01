#pragma once

#include <polycpp/core/error.hpp>

#include <string>

namespace polycpp::bcrypt_pbkdf {

/**
 * @enum PbkdfErrorCode
 * @brief Discriminator for `PbkdfError` failure modes.
 *
 * The C++ port replaces upstream's integer return code (`-1` on bad input, `0`
 * on success) with a typed exception. Each invalid input maps to one of these
 * codes so callers can branch on the failure cause without parsing the message
 * string.
 *
 * @see polycpp::bcrypt_pbkdf::PbkdfError
 */
enum class PbkdfErrorCode {
    /** `rounds` was less than 1. */
    InvalidRounds,
    /** `password` was empty. */
    EmptyPassword,
    /** `salt` was empty. */
    EmptySalt,
    /** `keylen` was 0. */
    EmptyKeylen,
    /** `keylen` exceeded 1024 bytes (matches upstream's HASHSIZE * HASHSIZE limit). */
    KeylenTooLarge,
    /** `salt.length()` exceeded 1 << 20 bytes. */
    SaltTooLarge,
    /** `bcryptHash` was called with an input that was not exactly 64 bytes. */
    InvalidHashInput,
};

/**
 * @brief Returns the canonical name of a `PbkdfErrorCode`.
 *
 * @param code A `PbkdfErrorCode` value.
 * @return A static C string with the enum-tag name (e.g. `"InvalidRounds"`).
 */
const char* pbkdfErrorCodeName(PbkdfErrorCode code) noexcept;

/**
 * @class PbkdfError
 * @brief Typed exception thrown by `pbkdf` and `bcryptHash` for invalid arguments.
 *
 * Inherits from `polycpp::Error`. Carries a `PbkdfErrorCode` so callers can
 * branch on the specific failure mode. The C++ API never returns a sentinel
 * error value — every parameter validation failure is raised as an exception
 * before any keying material is computed.
 *
 * @par Example
 * @code{.cpp}
 *   try {
 *       polycpp::bcrypt_pbkdf::pbkdf(password, salt, 0, 32);
 *   } catch (const polycpp::bcrypt_pbkdf::PbkdfError& e) {
 *       // e.code() == PbkdfErrorCode::InvalidRounds
 *   }
 * @endcode
 */
class PbkdfError : public polycpp::Error {
public:
    /**
     * @brief Constructs a `PbkdfError` with a code and a human-readable message.
     *
     * @param code Discriminator for the failure mode.
     * @param message Human-readable explanation of the failure.
     */
    PbkdfError(PbkdfErrorCode code, const std::string& message);

    /**
     * @brief Returns the typed error code.
     *
     * @return The `PbkdfErrorCode` for this exception.
     */
    PbkdfErrorCode code() const noexcept { return code_; }

private:
    PbkdfErrorCode code_;
};

}  // namespace polycpp::bcrypt_pbkdf
