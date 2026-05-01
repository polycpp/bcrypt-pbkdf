# API Mapping

| Upstream symbol | C++ symbol | Status | Notes |
|---|---|---|---|
| `bcrypt-pbkdf.pbkdf(pass, passlen, salt, saltlen, key, keylen, rounds)` | `polycpp::bcrypt_pbkdf::pbkdf(const polycpp::Buffer& password, const polycpp::Buffer& salt, std::uint32_t rounds, std::uint32_t keylen) -> polycpp::Buffer` | adapted | Synchronous in both upstream and C++. The C++ form removes the in/out `key` argument and the explicit length arguments — `polycpp::Buffer` already carries length, and the derived key is returned. Upstream's integer return code (`-1` on bad parameters, `0` on success) is replaced by typed `PbkdfError` exceptions. The byte-stride output layout is byte-identical to upstream for every (password, salt, rounds, keylen) tuple in the test plan. |
| `bcrypt-pbkdf.hash(sha2pass, sha2salt, out)` | `polycpp::bcrypt_pbkdf::bcryptHash(const polycpp::Buffer& sha2pass, const polycpp::Buffer& sha2salt) -> polycpp::Buffer` | adapted | Same inner Blowfish-based hash as upstream. Inputs must each be exactly 64 bytes (the size of a SHA-512 digest); a `PbkdfError(InvalidHashInput)` is thrown otherwise. The output is always a 32-byte (`HASHSIZE`) `polycpp::Buffer`. The function is exposed primarily for parity testing and for downstream callers that have already pre-computed SHA-512 of password and salt. |
| `bcrypt-pbkdf.BLOCKS` (= 8) | `polycpp::bcrypt_pbkdf::BLOCKS` (`inline constexpr std::size_t BLOCKS = 8`) | direct | Same value, same meaning: number of 64-bit Blowfish blocks in the inner cipher state. |
| `bcrypt-pbkdf.HASHSIZE` (= 32) | `polycpp::bcrypt_pbkdf::HASHSIZE` (`inline constexpr std::size_t HASHSIZE = 32`) | direct | Same value, same meaning: byte length of the inner `bcryptHash` output. |
| upstream integer error sentinel `-1` returned by `pbkdf` | `polycpp::bcrypt_pbkdf::PbkdfError` exception with `polycpp::bcrypt_pbkdf::PbkdfErrorCode` | adapted | Replaces a numeric sentinel with a typed exception. `PbkdfErrorCode` enumerates `InvalidRounds`, `EmptyPassword`, `EmptySalt`, `EmptyKeylen`, `KeylenTooLarge`, `SaltTooLarge`, `InvalidHashInput`. |
| async/callback variant of `pbkdf` mentioned in some documentation | (none) | omitted | Upstream `bcrypt-pbkdf@1.0.2` is purely synchronous in `index.js`; there is no callback variant to map. The C++ port mirrors the upstream synchronous shape because the work is CPU-local and short for the parameter ranges OpenSSH actually uses (16-32 rounds, 32-64-byte keys). Documented under "Sync-only API" in `docs/divergences.md`. |

Status values:

- `direct`: same behavior with an idiomatic C++ spelling.
- `compatibility layer`: same user-facing behavior through a different
  implementation shape.
- `adapted`: preserves the upstream intent with a typed C++ API.
- `deferred`: planned future work that is intentionally not implemented yet.
- `omitted`: deliberately not part of this port.

## TypeScript Declaration Review

- Declaration source used: none; upstream ships no `.d.ts` and `@types/bcrypt-pbkdf` is not maintained on DefinitelyTyped.
- Public APIs, overloads, options, callbacks, streams, or literal unions found only or most clearly in declarations: none — the upstream public surface is only what `module.exports` in `index.js` declares (`pbkdf`, `hash`, `BLOCKS`, `HASHSIZE`).
- Declaration-only globals, caches, deprecated fields, or runtime-specific surfaces mapped as unsupported/not-applicable: none.

## Framework object boundary review

- Upstream reads or mutates framework/request/response/context objects: no
- Upstream fields or methods read: none
- Upstream fields or methods written: none
- C++ adapter boundary: not applicable. The package consumes byte buffers and integers and produces a byte buffer. There is no `req`, `res`, `ctx`, header bag, socket, or framework-shaped object touched by the upstream code.
- Partial mutation risk on validation failure: not applicable. The C++ API never receives or mutates a framework-shaped object. `pbkdf` validates all parameters before allocating any output, so no caller-visible state is changed when a validation failure occurs.

If the boundary overlaps with existing `polycpp` request, response, header,
stream, URL, or companion-library types, name the reused type here. Do not leave
the boundary as a custom local abstraction without a recorded justification in
`docs/research.md`.

## Node parity surface review

- Callback APIs: not present in upstream; no adaptation needed. The C++ API is synchronous.
- Promise APIs: not present in upstream; no adaptation needed.
- EventEmitter APIs: not present in upstream; no adaptation needed.
- Server/listener APIs: not present in upstream; no adaptation needed. No listen/accept/connect surface.
- Diagnostic/tracing APIs: not present in upstream; no adaptation needed.
- Stream APIs: not present in upstream; no adaptation needed. The algorithm is whole-buffer.
- Buffer and binary APIs: upstream uses `Uint8Array` / `Uint32Array`. The C++ public API uses `polycpp::Buffer` for byte inputs/outputs and internal `std::array<uint32_t, N>` for the Blowfish state.
- URL, timer, process, and filesystem APIs: not used by upstream; no adaptation needed.
- Crypto, compression, TLS, network, and HTTP APIs: SHA-512 is the only crypto primitive. C++ port uses `polycpp::crypto::createHash("sha512")` (OpenSSL). No other crypto, compression, TLS, network, or HTTP surface is touched.
- Unsupported or non-meaningful Node-specific APIs and audit reason: none. The upstream package contains no Node-specific runtime APIs (no `process`, `Buffer`, `EventEmitter`, fs, etc.). The only runtime concept that does not survive translation is the integer sentinel return value, which is intentionally replaced by typed exceptions.
