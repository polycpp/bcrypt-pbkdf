// Minimal example demonstrating polycpp::bcrypt_pbkdf::pbkdf for an
// OpenSSH-style key derivation (32-byte AES-256 key plus 16-byte AES-CTR IV
// = 48 bytes total, derived from a 16-byte salt with 16 rounds).

#include <polycpp/bcrypt_pbkdf.hpp>
#include <polycpp/buffer.hpp>

#include <cstdint>
#include <iostream>
#include <string>

int main() {
    // OpenSSH-style invocation. In practice the salt is 16 random bytes
    // produced by ssh-keygen; the password is what the user typed.
    const polycpp::Buffer password = polycpp::Buffer::from("correct horse battery staple");
    const polycpp::Buffer salt     = polycpp::Buffer::from("0123456789abcdef");

    const polycpp::Buffer derived =
        polycpp::bcrypt_pbkdf::pbkdf(password, salt, /*rounds=*/16, /*keylen=*/48);

    std::cout << "derived key length: " << derived.length() << " bytes\n";
    std::cout << "derived key (hex): ";
    static constexpr char kHex[] = "0123456789abcdef";
    for (std::size_t i = 0; i < derived.length(); ++i) {
        std::cout << kHex[(derived.data()[i] >> 4) & 0xf]
                  << kHex[derived.data()[i] & 0xf];
    }
    std::cout << "\n";

    return 0;
}
