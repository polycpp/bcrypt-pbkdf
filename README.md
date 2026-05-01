# polycpp-bcrypt-pbkdf

C++ port of [bcrypt-pbkdf](https://www.npmjs.com/package/bcrypt-pbkdf) for [polycpp](https://github.com/enricohuang/polycpp).

## Status

Port version: `0.1.0`

Initial port based on upstream version: `TBD`

Compatibility note:

- This repo does not imply full parity with upstream `bcrypt-pbkdf`.
- Implemented and deferred behavior is tracked in `docs/research.md`, `docs/api-mapping.md`, and `docs/divergences.md`.

Implemented:

- Planning only. Implementation has not started.

Deferred:

- Full feature list is tracked in `docs/divergences.md`.

Known divergences:

- None recorded yet beyond the planned `v0` scope. See `docs/divergences.md`.

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

## Usage

```cpp
// Usage examples will be added after the first implemented API slice.
```

## License

MIT
