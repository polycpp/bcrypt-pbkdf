# Research

Run `scripts/intake-upstream.py` before filling this file manually.

- package:
- npm url:
- source url:
- upstream version basis:
- upstream revision analyzed:
- upstream default branch:
- license:
- license evidence:
- category:

## Package purpose

- TODO

## Runtime assumptions

- browser: TODO
- node.js: TODO
- filesystem: TODO
- network: TODO
- crypto: TODO
- terminal: TODO

## Dependency summary

- package.json present: TODO
- hard dependencies: TODO
- peer dependencies: TODO
- optional dependencies: TODO
- package exports: TODO
- package types: TODO
- package bin: TODO
- dependency analysis report: `docs/dependency-analysis.md`

## Upstream repo layout summary

- Clone path used for analysis: TODO
- TODO

## Entry points used by consumers

- TODO
- TypeScript declarations inspected: TODO

## Important files and why they matter

- TODO

## Files likely irrelevant to the C++ port

- TODO

## Test directories worth mining first

- TODO

## Implementation risks discovered from the source layout

- TODO

## Companion repo alignment

- companion repos inspected: TODO
- CMake target and alias pattern: TODO
- public header layout: TODO
- detail/private header strategy: TODO
- aggregator header strategy: TODO
- examples strategy: TODO
- documentation site strategy: TODO
- deliberate deviations from existing companions: TODO

## Polycpp ecosystem reuse analysis

- polycpp core paths inspected: TODO
- polycpp capability snapshot: TODO (record `<polycpp checkout>` HEAD, date checked, and command output)
- transport/listener capability review: TODO (TCP, Unix/IPC path, adopted handles, TLS client, and TLS server primitives found or rejected)
- polycpp core types/functions selected: TODO
- polycpp core types/functions rejected: TODO
- public polycpp interop review: TODO
- string policy: TODO (default to `std::string`/UTF-8; use `polycpp::String` only for JavaScript UTF-16 code-unit parity)
- JsonValue/Object/Array policy: TODO
- Date/time interop policy: TODO
- diagnostic/config object policy: TODO
- toJSON/stringify policy: TODO
- companion libs inspected for reusable APIs: TODO
- companion libs selected for reuse: TODO
- companion libs rejected or deferred: TODO
- new local abstractions introduced: TODO
- reuse risks or integration gaps: TODO

## Node parity surface audit

- callback APIs: TODO
- Promise APIs: TODO
- EventEmitter APIs: TODO
- server/listener APIs: TODO (listen modes including TCP, Unix/IPC path and adopted handles when relevant, accepted connection type, lifecycle, and C++ adapter)
- stream APIs: TODO (record object-mode chunk type and concrete adapter such as typed iterator/callback or NDJSON byte stream)
- Buffer and binary APIs: TODO
- URL, timer, process, and filesystem APIs: TODO
- crypto, compression, TLS, network, and HTTP APIs: TODO (separate client transport, server/listener lifecycle, Unix/IPC path support, and TLS client/server modes when relevant)
- unsupported Node-specific APIs and audit reason: TODO

## External SDK and native driver strategy

- upstream external services/protocols: TODO
- native SDKs/client libraries to use: TODO
- SDKs/protocols explicitly not reimplemented: TODO
- adapter/linking strategy: TODO
- test environment needs: TODO

## Compatibility foundation review

- downstream dependency role: TODO
- native substitution risk: TODO
- upstream implementation data to preserve: TODO
- generated or vendored data plan: TODO (include source artifact, generator command, license notice, and whether large data is isolated in its own translation unit)
- compatibility fixture strategy: TODO

## Security and fail-closed review

- security-sensitive behavior: TODO
- trust boundary: TODO
- supported protocol or algorithm matrix: TODO
- unsupported behavior and fail-closed policy: TODO
- result-set/framing drain policy, if protocol client: TODO
- binary payload type-mapping policy, if protocol client: TODO
- stateful parser/session-state policy, if protocol client/server: TODO
- server/listener response writer matrix, if protocol server surface exists: TODO
- key, secret, credential, or user-controlled input handling: TODO
- misuse cases that must be tested: TODO

## Core use cases

- TODO

## Key features to port first

- TODO

## Features to defer

- TODO

## v0 scope

- port version: 0.1.0
- versioning note: port version is independent from upstream versioning
- supported APIs: TODO
- unsupported APIs: TODO
- dependency plan: TODO
- polycpp modules to use: TODO
- missing polycpp primitives: TODO (separate connect deadlines from per-operation deadlines when relevant, and record post-timeout resource state)
