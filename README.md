# polycpp-bcrypt-pbkdf

C++ port of [bcrypt-pbkdf](https://www.npmjs.com/package/bcrypt-pbkdf) for [polycpp](https://github.com/enricohuang/polycpp).

`bcrypt-pbkdf` derives a cryptographic key from a password and salt using the
OpenBSD `bcrypt_pbkdf` construction (Blowfish + SHA-512). It is the key
derivation function OpenSSH `ssh-keygen` uses to encrypt new-format private
keys (`-----BEGIN OPENSSH PRIVATE KEY-----`).

## Status

Port version: `0.1.0`

Initial port based on upstream version: `1.0.2`

Compatibility note:

- This repo does not imply full parity with upstream `bcrypt-pbkdf`.
- Implemented and deferred behavior is tracked in `docs/research.md`, `docs/api-mapping.md`, and `docs/divergences.md`.

Implemented:

- `polycpp::bcrypt_pbkdf::pbkdf(password, salt, rounds, keylen) -> Buffer` — synchronous KDF; byte-identical output to upstream JS for every recorded test vector.
- `polycpp::bcrypt_pbkdf::bcryptHash(sha2pass, sha2salt) -> Buffer` — inner round function, exposed for parity testing and pre-hashed callers.
- `polycpp::bcrypt_pbkdf::PbkdfError` plus `PbkdfErrorCode` enum for fail-closed parameter validation.
- Constants `polycpp::bcrypt_pbkdf::BLOCKS` (= 8) and `polycpp::bcrypt_pbkdf::HASHSIZE` (= 32).

Deferred:

- None. The C++ port covers every callable in upstream `index.js`. See `docs/divergences.md`.

Known divergences:

- Sync-only API (upstream is itself synchronous; no async surface to mirror).
- SHA-512 is provided by `polycpp::crypto::createHash("sha512")` (OpenSSL) instead of `tweetnacl`. Byte-identical output.
- Typed `PbkdfError` exceptions replace upstream's integer return codes (`-1` / `0`).
- Full list in `docs/divergences.md`.

## Prerequisites

- C++20 compiler
- CMake 3.20+
- Ninja recommended

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

To build against a local polycpp checkout:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DPOLYCPP_SOURCE_DIR=<polycpp checkout>
```

## Usage

```cpp
#include <polycpp/bcrypt_pbkdf.hpp>
#include <polycpp/buffer.hpp>

#include <iostream>

int main() {
    // OpenSSH-style invocation: 16-byte salt, 16 rounds, 48-byte derived key
    // (32 bytes for AES-256 + 16 bytes for the AES-CTR IV).
    const polycpp::Buffer password = polycpp::Buffer::from("correct horse battery staple");
    const polycpp::Buffer salt     = polycpp::Buffer::from("0123456789abcdef");

    const polycpp::Buffer derived =
        polycpp::bcrypt_pbkdf::pbkdf(password, salt, /*rounds=*/16, /*keylen=*/48);

    std::cout << "derived key length: " << derived.length() << " bytes\n";
    return 0;
}
```

## License

MIT (see `LICENSE`). Upstream BSD-3-Clause and ISC notices for the bcrypt-pbkdf
algorithm portions are reproduced verbatim in `THIRD_PARTY_LICENSES.md`.
