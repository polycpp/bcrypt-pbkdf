# Divergences From Upstream

## Deferred Features

- None. The C++ port covers every callable in upstream `index.js` and both numeric constants. There is nothing intentionally postponed for a later release.

Use this section only for behavior intended for future implementation.

## Deliberate Behavior Changes

- Sync-only API: the C++ port exposes only synchronous `pbkdf` and `bcryptHash`. Upstream `bcrypt-pbkdf@1.0.2` is itself synchronous in `index.js` (it returns an integer status code rather than calling a callback or returning a Promise), so there is no upstream async surface to mirror. The work is CPU-local and short for the parameter ranges OpenSSH actually uses (16-32 rounds, 32-64-byte keys). If a caller needs to run the KDF off the main thread they can use `std::async` or any thread-pool primitive at the call site.
- SHA-512 source change: upstream uses `tweetnacl.lowlevel.crypto_hash` for SHA-512. The C++ port uses `polycpp::crypto::createHash("sha512")` (which delegates to OpenSSL). Both implementations produce byte-identical SHA-512 output for the same input; the change is purely internal. The `tweetnacl` notice is therefore not reproduced because no `tweetnacl` source is shipped.
- Typed exceptions in place of integer return codes: upstream returns `-1` from `pbkdf` for invalid parameters and `0` for success. The C++ port throws `polycpp::bcrypt_pbkdf::PbkdfError` (a subclass of `polycpp::Error`) carrying a `PbkdfErrorCode` enum value (`InvalidRounds`, `EmptyPassword`, `EmptySalt`, `EmptyKeylen`, `KeylenTooLarge`, `SaltTooLarge`, `InvalidHashInput`). The C++ API never returns a sentinel error; every parameter validation failure is raised as an exception before any keying material is computed. Tested in `tests/test_errors.cpp`.
- Buffer-shaped public API: upstream takes seven positional arguments including explicit `passlen`, `saltlen`, an in/out `key`, and `keylen`. The C++ port collapses these into `pbkdf(password, salt, rounds, keylen) -> Buffer` because `polycpp::Buffer` already carries length and the derived key is a return value, not an in/out buffer. The byte-level output is identical to upstream for the same inputs.
- Private Blowfish: upstream's `Blowfish` class is exported as part of the module surface only because `module.exports` happens to capture it. Consumers in practice only ever call `pbkdf` and (rarely) `hash`. The C++ port keeps `Blowfish` private under `include/polycpp/bcrypt_pbkdf/detail/blowfish.hpp` and never exposes it as part of the public API. This avoids surprising callers with a Blowfish surface that would not be portable across OpenSSL versions (Blowfish is not in OpenSSL's default provider in modern releases) and that does not exercise the standard ECB/CBC modes.

Use this section for intentional C++ API or behavior choices, including
upstream APIs that are intentionally omitted rather than deferred.

## Unsupported Runtime-Specific Features

- None. The upstream package uses no Node-specific runtime APIs (no `process`, no `EventEmitter`, no `Buffer`, no fs, no URL, no timers). There is therefore no JS runtime surface to mark unsupported.
