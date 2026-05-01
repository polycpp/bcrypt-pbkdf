# API Mapping

| Upstream symbol | C++ symbol | Status | Notes |
|---|---|---|---|
| TODO | TODO | TODO | TODO |

Status values:

- `direct`: same behavior with an idiomatic C++ spelling.
- `compatibility layer`: same user-facing behavior through a different
  implementation shape.
- `adapted`: preserves the upstream intent with a typed C++ API.
- `deferred`: planned future work that is intentionally not implemented yet.
- `omitted`: deliberately not part of this port.

## TypeScript Declaration Review

- Declaration source used: TODO
- Public APIs, overloads, options, callbacks, streams, or literal unions found only or most clearly in declarations: TODO
- Declaration-only globals, caches, deprecated fields, or runtime-specific surfaces mapped as unsupported/not-applicable: TODO

## Framework object boundary review

- Upstream reads or mutates framework/request/response/context objects: TODO
- Upstream fields or methods read: TODO
- Upstream fields or methods written: TODO
- C++ adapter boundary: TODO
- Partial mutation risk on validation failure: TODO

If the boundary overlaps with existing `polycpp` request, response, header,
stream, URL, or companion-library types, name the reused type here. Do not leave
the boundary as a custom local abstraction without a recorded justification in
`docs/research.md`.

## Node parity surface review

- Callback APIs: TODO
- Promise APIs: TODO
- EventEmitter APIs: TODO
- Server/listener APIs: TODO (listen modes including TCP, Unix/IPC path and adopted handles when relevant, accepted connection type, lifecycle, and C++ adapter)
- Diagnostic/tracing APIs: TODO
- Stream APIs: TODO (object-mode chunk type and C++ adapter, if any)
- Buffer and binary APIs: TODO
- URL, timer, process, and filesystem APIs: TODO (separate connect, per-command, pool wait, and idle deadlines when relevant)
- Crypto, compression, TLS, network, and HTTP APIs: TODO (separate client transport, server/listener lifecycle, Unix/IPC path support, and TLS client/server modes when relevant)
- Unsupported or non-meaningful Node-specific APIs and audit reason: TODO
