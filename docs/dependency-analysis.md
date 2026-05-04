# Dependency and JavaScript API Analysis

- package: bcrypt-pbkdf
- package version: 1.0.2
- package root: `.tmp/upstream/node-bcrypt-pbkdf`
- analyzer json: `.tmp/dependency-analysis.json`
- published npm artifact path: `.tmp/npm-package`
- published npm artifact analyzed: yes — confirmed the published `index.js` and `package.json` are byte-identical to the Git source clone, no `dist/`, no `lib/`, no minification step
- include dev dependencies: no
- dependency source install used: `npm install --ignore-scripts` via the libgen analyzer with the package's lone runtime dependency (`tweetnacl@0.14.5`) installed in a temporary workspace
- companion root checked: parent directory containing the polycpp companion checkouts

## Package entry metadata

- main: `index.js`
- module: not declared
- types: not declared
- exports: not declared (legacy `main`-only package)
- bin: none
- missing declared entries in repo clone: none — `index.js` exists at the declared path
- TypeScript source files detected: 0
- TypeScript declarations reviewed: none ship with the package; `@types/bcrypt-pbkdf` is not maintained on DefinitelyTyped. Source-only review.
- declaration-source decision: not applicable; manual review of `index.js` and the upstream README is the API surface.
- source-vs-published artifact decision: source clone is the runtime source of truth because the published artifact is byte-identical to the source `index.js`.

## Direct dependencies

- `tweetnacl` ^0.14.3 (hard runtime dependency) — used solely for `tweetnacl.lowlevel.crypto_hash` (SHA-512). The upstream code only calls `crypto_hash(out, msg, msgLen)`; no other tweetnacl surface is touched.

## Dependency ownership decisions

| Package | Kind | Requested | Installed | License | License evidence | License impact | License strategy | Affects repo license | Deps | Source files | Node API calls | JS API calls | Recommendation | Rationale |
|---|---|---|---|---|---|---|---|---|---:|---:|---:|---:|---|---|
| tweetnacl | hard | ^0.14.3 | 0.14.5 | Unlicense | package.json license field declares Unlicense | permissive | clean-room replacement | no | 0 | 5 | 0 | 0 | implement private helper in this repo | tweetnacl is consumed only for SHA-512; polycpp already exposes SHA-512 through `polycpp::crypto::createHash("sha512")` (OpenSSL). Replace the entire dependency with that polycpp call. No tweetnacl source is shipped. |

## License impact summary

- upstream package license: BSD-3-Clause (with ISC notices for the bcrypt_pbkdf and Joyent performance-improvement portions, both reproduced in `THIRD_PARTY_LICENSES.md`)
- repo license decision: MIT for the polycpp companion repo. The clean-room C++ port carries the polycpp project's MIT license; the upstream BSD-3-Clause + ISC notices are preserved verbatim in `THIRD_PARTY_LICENSES.md` because the algorithm and the standard Blowfish constants ultimately originate there.
- GPL/AGPL dependencies: none
- LGPL/MPL dependencies: none
- permissive dependencies requiring notices: tweetnacl is replaced by polycpp's OpenSSL-backed SHA-512; no tweetnacl notice is shipped because no tweetnacl code is shipped.
- dev/test-only dependencies excluded from shipped artifacts: none — upstream has no devDependencies in `package.json`.
- dependency license notices to add to `THIRD_PARTY_LICENSES.md`: BSD-3-Clause notice for the Blowfish portion (Niels Provos, with David Mazieres as advisor), ISC notice for the bcrypt_pbkdf portion (Ted Unangst), ISC notice for the Joyent performance-improvement portion (Joyent / Alex Wilson). All three are reproduced verbatim from the upstream `LICENSE` file.

## Transitive dependency summary

- `tweetnacl@0.14.5` has zero further runtime dependencies (verified by analyzer `dependedBy` graph).
- The replacement path (polycpp's `crypto::createHash("sha512")`) introduces no new transitive dependency beyond OpenSSL, which is already a transitive dependency of polycpp.

## Runtime API usage

### Target package

- entry points analyzed: `index.js`
- source files analyzed by analyzer: 1
- source files manually inspected: 1 (`index.js`, in full)
- external imports seen from target: `tweetnacl` (only `tweetnacl.lowlevel.crypto_hash`)

### Analyzer porting gates

- polycpp reuse hints consumed: analyzer reported zero hints because the upstream code uses no Node built-in module identifiers and no obvious polycpp-shaped API names. Manual mapping: SHA-512 -> `polycpp::crypto::createHash("sha512")`; binary buffers -> `polycpp::Buffer`.
- Node parity hints consumed: analyzer reported zero hints. Manual review confirms the package has no callbacks, no Promises, no EventEmitters, no streams, no server/listener APIs, no URLs, no timers, no fs, no process. The audit is recorded in `docs/research.md ## Node parity surface audit`.
- security hints consumed: analyzer reported `securitySensitive: false` because `index.js` references no obvious crypto API name strings. This is a false negative — the package's stated purpose is a key derivation function, so by definition it is security-sensitive. We override the analyzer and treat the package as security-sensitive. The supported algorithm matrix, fail-closed policy, and misuse tests are recorded in `docs/research.md ## Security and fail-closed review` and `docs/test-plan.md`.
- security-sensitive package: yes (analyzer false negative; manual override)
- polycpp capability snapshot consumed: HEAD `7a8df099e2564ff55729a1d2121feb9a88501119` captured 2026-05-01; reuse decisions in `docs/research.md ## Polycpp ecosystem reuse analysis` are pinned to that snapshot.
- transport/listener capability hints consumed: not applicable — the package has no listener semantics; the polycpp transport/listener surfaces (TCP, Unix path, adopted handle, TLS) were searched and confirmed irrelevant.

### Node.js API usage

- none — `index.js` does not import any Node built-in module. The only `require` call is `require('tweetnacl')`.

### Node parity surface usage

- callbacks: not present
- Promise APIs: not present
- EventEmitter APIs: not present
- server/listener APIs: not present
- diagnostic/tracing APIs: not present
- streams: not present (algorithm is whole-buffer)
- Buffer and binary data: upstream uses `Uint8Array` / `Uint32Array`; C++ port uses `polycpp::Buffer` for the public surface and fixed-size `std::array` for internal state
- URL/timer/process/filesystem APIs: not present
- crypto/compression/TLS/network/HTTP APIs: SHA-512 only. C++ port uses `polycpp::crypto::createHash("sha512")` (OpenSSL).

### JavaScript API usage

- `Math.floor` (2 occurrences): used to compute `stride` and `amt`. C++ port uses integer division.
- `Math.min` (1 occurrence): clamps the per-pass byte count to remaining `keylen`. C++ port uses `std::min`.
- `Uint8Array` (9 occurrences): byte buffers. C++ port uses `polycpp::Buffer` (public) and `std::array<uint8_t, N>` (internal).
- `Uint32Array` (8 occurrences): 32-bit word arrays for Blowfish state and the cipher block. C++ port uses `std::array<uint32_t, N>`.

### Framework object boundary usage

- analyzer-reported target-package framework object accesses: 0
- analyzer-reported dependency framework object accesses: 0
- manual review decision: not applicable — the package neither reads nor mutates `req`, `res`, `ctx`, headers, sockets, or any other framework-shaped object. The public API takes byte buffers and integers and returns byte buffers.

## Porting decisions

- Replace `tweetnacl.lowlevel.crypto_hash` with `polycpp::crypto::createHash("sha512").update(...).digestBuffer()`. This eliminates the only runtime dependency. OpenSSL's SHA-512 is byte-identical to tweetnacl's, so output parity is guaranteed.
- Implement Blowfish privately in `src/blowfish.cpp`. OpenSSL deprecated and removed Blowfish from the default provider, and the bcrypt-style expansion schedule (`expand0state` / `expandstate`) is not exposed by the EVP API even where Blowfish is still available. Keeping the Blowfish state private (under `include/polycpp/bcrypt_pbkdf/detail/`) avoids creating a public Blowfish surface that callers might mistake for general-purpose cipher access.
- Keep all binary inputs/outputs byte-preserving. Public APIs accept and return `polycpp::Buffer`; an additional `std::string` overload accepts ASCII-style passwords without re-encoding.
- Replace upstream's integer return code (`-1` / `0`) with typed exceptions (`PbkdfError`) carrying a small `code` enum. The C++ API never returns a sentinel error value; every parameter validation failure throws before any keying material is computed.
- Each porting decision is consistent with the polycpp ecosystem reuse analysis recorded in `docs/research.md ## Polycpp ecosystem reuse analysis`.

## Analyzer warnings

- `tweetnacl: no entry points found for tweetnacl` — the analyzer cannot resolve a public entry point for `tweetnacl` because it consists of a single `nacl.js` file plus generated sub-builds. Not relevant to the port: `tweetnacl` is fully replaced by `polycpp::crypto::createHash("sha512")` and is not a transitive build dependency. The analyzer warning is informational only.
