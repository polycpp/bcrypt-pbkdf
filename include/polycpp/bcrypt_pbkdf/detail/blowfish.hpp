#pragma once

// Internal Blowfish state used by the bcrypt_pbkdf construction.
//
// This header is private. It is not part of the polycpp::bcrypt_pbkdf public
// surface and must not be included from public docs or examples. The Blowfish
// expansion schedule used here (`expand0state` / `expandstate`) is specific to
// OpenBSD's bcrypt construction and is not exposed by OpenSSL's EVP cipher
// API. Callers who need general-purpose Blowfish should use a dedicated cipher
// library; this implementation exists only to back `bcrypt_pbkdf`.

#include <array>
#include <cstddef>
#include <cstdint>

namespace polycpp::bcrypt_pbkdf::detail {

/**
 * @class Blowfish
 * @brief Blowfish state machine specialized for OpenBSD bcrypt expansion.
 *
 * Holds the 18-word P-array and the four 256-word S-boxes in their initial
 * Blowfish values. Each instance also owns `j_`, the keystream pointer used by
 * `stream2word`; this is per-instance (rather than module-level as in the
 * upstream JS) so concurrent `pbkdf` calls do not interfere.
 */
class Blowfish {
public:
    /** @brief Constructs a Blowfish state initialized to the standard P-array and S-boxes. */
    Blowfish() noexcept;

    /**
     * @brief Encrypts one 64-bit block (`x[0]` || `x[1]`) in place.
     *
     * Mirrors upstream's `Blowfish.prototype.encipher`. The byte view used by
     * the F-function indexes `x` as little-endian bytes regardless of host
     * endianness, matching the upstream JS `Uint32Array`/`Uint8Array` aliasing.
     */
    void encipher(std::uint32_t* x) noexcept;

    /**
     * @brief OpenBSD bcrypt expansion of state from `(data, key)`.
     *
     * Mirrors upstream `expandstate`. Resets the keystream pointer to the
     * beginning of `key`, XORs the P-array against keystream words, then
     * iteratively enciphers a 64-bit accumulator while XORing in keystream
     * words from `data`.
     */
    void expandstate(const std::uint8_t* data, std::size_t databytes,
                     const std::uint8_t* key, std::size_t keybytes) noexcept;

    /**
     * @brief OpenBSD bcrypt expansion of state from `key` only.
     *
     * Mirrors upstream `expand0state`. Same as `expandstate` but seeded with a
     * zero-initialized 64-bit accumulator instead of `data`.
     */
    void expand0state(const std::uint8_t* key, std::size_t keybytes) noexcept;

    /**
     * @brief Encrypts `blocks` 64-bit blocks of `data` in ECB mode.
     *
     * `data` is `2 * blocks` words; each pair of words is enciphered in place.
     * Mirrors upstream `Blowfish.prototype.enc`.
     */
    void enc(std::uint32_t* data, std::size_t blocks) noexcept;

private:
    /** @brief Reads four bytes from `data` starting at the keystream pointer, advancing it. */
    std::uint32_t streamToWord(const std::uint8_t* data, std::size_t databytes) noexcept;

    std::array<std::array<std::uint32_t, 256>, 4> s_;
    std::array<std::uint32_t, 18> p_;
    std::size_t j_ = 0;
};

/**
 * @brief Compute the inner bcrypt_hash step end-to-end.
 *
 * Mirrors upstream `bcrypt_hash`. `sha2pass` and `sha2salt` must each point to
 * exactly 64 bytes; `out` must point to exactly `polycpp::bcrypt_pbkdf::HASHSIZE`
 * (32) writable bytes. This function lives in the detail namespace because it
 * is shared between the public `bcryptHash` wrapper and the `pbkdf` driver,
 * but it is not part of the public API.
 */
void bcryptHashRaw(const std::uint8_t* sha2pass,
                   const std::uint8_t* sha2salt,
                   std::uint8_t* out) noexcept;

}  // namespace polycpp::bcrypt_pbkdf::detail
