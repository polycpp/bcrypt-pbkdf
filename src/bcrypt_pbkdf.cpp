#include <polycpp/bcrypt_pbkdf/bcrypt_pbkdf.hpp>
#include <polycpp/bcrypt_pbkdf/detail/blowfish.hpp>
#include <polycpp/bcrypt_pbkdf/error.hpp>

#include <polycpp/buffer.hpp>
#include <polycpp/crypto.hpp>
#include <polycpp/ssl/memory.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

namespace polycpp::bcrypt_pbkdf::detail {

// Constant cipher input used by the inner Blowfish hash step. Identical to
// upstream JS `ciphertext`: the 32-byte ASCII string
// "OxychromaticBlowfishSwatDynamite". Defined here at file scope so both the
// helper and the bcryptHashRaw routine see the same instance.
constexpr std::array<std::uint8_t, 32> kCipherInput = {
    'O', 'x', 'y', 'c', 'h', 'r', 'o', 'm',
    'a', 't', 'i', 'c', 'B', 'l', 'o', 'w',
    'f', 'i', 's', 'h', 'S', 'w', 'a', 't',
    'D', 'y', 'n', 'a', 'm', 'i', 't', 'e'};

// Local stream-to-word that mirrors upstream `stream2word` against the cipher
// input buffer. `j` is updated in place. Used to load `cdata` from
// `kCipherInput` while reproducing upstream BLF_J advancement semantics.
static std::uint32_t cipherStreamToWord(std::size_t& j) noexcept {
    std::uint32_t temp = 0;
    for (int i = 0; i < 4; ++i) {
        if (j >= kCipherInput.size()) {
            j = 0;
        }
        temp = (temp << 8) | kCipherInput[j];
        ++j;
    }
    return temp;
}

void bcryptHashRaw(const std::uint8_t* sha2pass,
                   const std::uint8_t* sha2salt,
                   std::uint8_t* out) noexcept {
    Blowfish state;

    state.expandstate(sha2salt, 64, sha2pass, 64);
    for (int i = 0; i < 64; ++i) {
        state.expand0state(sha2salt, 64);
        state.expand0state(sha2pass, 64);
    }

    // Build cdata. Upstream `expand0state` resets `BLF_J` to 0 mid-routine
    // and never advances it again before returning, so by the time we reach
    // here `BLF_J` is 0. We mirror that with our own counter `j` starting at
    // 0; cipherStreamToWord wraps as needed against the 32-byte cipher input.
    std::size_t j = 0;
    std::array<std::uint32_t, BLOCKS> cdata{};
    for (std::size_t i = 0; i < BLOCKS; ++i) {
        cdata[i] = cipherStreamToWord(j);
    }

    // Encrypt cdata 64 times. Upstream:
    //   for (i = 0; i < 64; i++) state.enc(cdata, cdata.byteLength / 8);
    // cdata is BLOCKS uint32_t = 32 bytes, / 8 = BLOCKS/2 = 4 64-bit blocks.
    for (int i = 0; i < 64; ++i) {
        state.enc(cdata.data(), BLOCKS / 2);
    }

    // Output is little-endian per word, matching upstream's byte assignments
    //   out[4*i + 0] = cdata[i]
    //   out[4*i + 1] = cdata[i] >> 8
    //   out[4*i + 2] = cdata[i] >> 16
    //   out[4*i + 3] = cdata[i] >> 24
    for (std::size_t i = 0; i < BLOCKS; ++i) {
        out[4 * i + 0] = static_cast<std::uint8_t>(cdata[i]);
        out[4 * i + 1] = static_cast<std::uint8_t>(cdata[i] >> 8);
        out[4 * i + 2] = static_cast<std::uint8_t>(cdata[i] >> 16);
        out[4 * i + 3] = static_cast<std::uint8_t>(cdata[i] >> 24);
    }
}

}  // namespace polycpp::bcrypt_pbkdf::detail

namespace polycpp::bcrypt_pbkdf {

namespace {

// SHA-512 of `data` returned as a 64-byte byte array. Uses
// `polycpp::crypto::createHash("sha512")` (OpenSSL-backed). Byte-identical to
// upstream's `tweetnacl.lowlevel.crypto_hash` for the inputs this KDF
// produces.
std::array<std::uint8_t, 64> sha512Bytes(const std::uint8_t* data, std::size_t len) {
    auto hasher = polycpp::crypto::createHash("sha512");
    hasher.update(polycpp::Buffer::from(data, len));
    auto digest = hasher.digestBuffer();
    std::array<std::uint8_t, 64> out{};
    if (digest.length() != 64) {
        // Defense in depth: SHA-512 always produces 64 bytes. If anything
        // goes wrong, throw a typed error rather than producing a silently
        // wrong key.
        throw PbkdfError(PbkdfErrorCode::InvalidHashInput,
                         "SHA-512 returned an unexpected digest length");
    }
    std::memcpy(out.data(), digest.data(), 64);
    return out;
}

}  // namespace

const char* pbkdfErrorCodeName(PbkdfErrorCode code) noexcept {
    switch (code) {
        case PbkdfErrorCode::InvalidRounds:    return "InvalidRounds";
        case PbkdfErrorCode::EmptyPassword:    return "EmptyPassword";
        case PbkdfErrorCode::EmptySalt:        return "EmptySalt";
        case PbkdfErrorCode::EmptyKeylen:      return "EmptyKeylen";
        case PbkdfErrorCode::KeylenTooLarge:   return "KeylenTooLarge";
        case PbkdfErrorCode::SaltTooLarge:     return "SaltTooLarge";
        case PbkdfErrorCode::InvalidHashInput: return "InvalidHashInput";
    }
    return "Unknown";
}

PbkdfError::PbkdfError(PbkdfErrorCode code, const std::string& message)
    : polycpp::Error(message), code_(code) {
    name = "PbkdfError";
}

polycpp::Buffer bcryptHash(const polycpp::Buffer& sha2pass,
                           const polycpp::Buffer& sha2salt) {
    if (sha2pass.length() != 64 || sha2salt.length() != 64) {
        throw PbkdfError(PbkdfErrorCode::InvalidHashInput,
                         "bcryptHash inputs must each be exactly 64 bytes");
    }
    polycpp::Buffer out = polycpp::Buffer::alloc(HASHSIZE);
    detail::bcryptHashRaw(sha2pass.data(), sha2salt.data(), out.data());
    return out;
}

polycpp::Buffer pbkdf(const polycpp::Buffer& password,
                      const polycpp::Buffer& salt,
                      std::uint32_t rounds,
                      std::uint32_t keylen) {
    if (rounds < 1) {
        throw PbkdfError(PbkdfErrorCode::InvalidRounds,
                         "rounds must be >= 1");
    }
    if (password.length() == 0) {
        throw PbkdfError(PbkdfErrorCode::EmptyPassword,
                         "password must not be empty");
    }
    if (salt.length() == 0) {
        throw PbkdfError(PbkdfErrorCode::EmptySalt,
                         "salt must not be empty");
    }
    if (keylen == 0) {
        throw PbkdfError(PbkdfErrorCode::EmptyKeylen,
                         "keylen must be >= 1");
    }
    if (keylen > MAX_KEYLEN) {
        throw PbkdfError(PbkdfErrorCode::KeylenTooLarge,
                         "keylen exceeds maximum (HASHSIZE * HASHSIZE = 1024)");
    }
    if (salt.length() > MAX_SALTLEN) {
        throw PbkdfError(PbkdfErrorCode::SaltTooLarge,
                         "salt length exceeds maximum (1 << 20 = 1048576)");
    }

    polycpp::Buffer key = polycpp::Buffer::alloc(keylen);

    // sha2pass = SHA-512(password)
    auto sha2pass = sha512Bytes(password.data(), password.length());

    // countsalt = salt || 4-byte big-endian counter
    std::vector<std::uint8_t> countsalt(salt.length() + 4);
    std::memcpy(countsalt.data(), salt.data(), salt.length());

    // Upstream:
    //   stride = floor((keylen + HASHSIZE - 1) / HASHSIZE)
    //   amt    = floor((keylen + stride - 1) / stride)
    const std::uint32_t kHashSize32 = static_cast<std::uint32_t>(HASHSIZE);
    const std::uint32_t stride = (keylen + kHashSize32 - 1) / kHashSize32;
    std::uint32_t amt = (keylen + stride - 1) / stride;
    const std::uint32_t origkeylen = keylen;

    std::array<std::uint8_t, HASHSIZE> out_block{};
    std::array<std::uint8_t, HASHSIZE> tmpout{};

    std::uint32_t remaining = keylen;
    for (std::uint32_t count = 1; remaining > 0; ++count) {
        countsalt[salt.length() + 0] = static_cast<std::uint8_t>(count >> 24);
        countsalt[salt.length() + 1] = static_cast<std::uint8_t>(count >> 16);
        countsalt[salt.length() + 2] = static_cast<std::uint8_t>(count >> 8);
        countsalt[salt.length() + 3] = static_cast<std::uint8_t>(count);

        auto sha2salt = sha512Bytes(countsalt.data(), countsalt.size());
        detail::bcryptHashRaw(sha2pass.data(), sha2salt.data(), tmpout.data());
        out_block = tmpout;

        for (std::uint32_t i = 1; i < rounds; ++i) {
            sha2salt = sha512Bytes(tmpout.data(), tmpout.size());
            detail::bcryptHashRaw(sha2pass.data(), sha2salt.data(), tmpout.data());
            for (std::size_t j = 0; j < HASHSIZE; ++j) {
                out_block[j] ^= tmpout[j];
            }
        }

        const std::uint32_t this_amt = std::min(amt, remaining);
        std::uint32_t i = 0;
        for (; i < this_amt; ++i) {
            const std::uint32_t dest = i * stride + (count - 1);
            if (dest >= origkeylen) {
                break;
            }
            key.data()[dest] = out_block[i];
        }
        remaining -= i;
    }

    // Scrub scratch buffers that held SHA-512(password) and intermediate
    // round outputs. polycpp::ssl::secureZero delegates to OPENSSL_cleanse,
    // which is documented not to be optimized away.
    polycpp::ssl::secureZero(out_block.data(), out_block.size());
    polycpp::ssl::secureZero(tmpout.data(), tmpout.size());
    polycpp::ssl::secureZero(sha2pass.data(), sha2pass.size());

    return key;
}

polycpp::Buffer pbkdf(const std::string& password,
                      const polycpp::Buffer& salt,
                      std::uint32_t rounds,
                      std::uint32_t keylen) {
    return pbkdf(polycpp::Buffer::from(reinterpret_cast<const std::uint8_t*>(password.data()),
                                       password.size()),
                 salt, rounds, keylen);
}

}  // namespace polycpp::bcrypt_pbkdf
