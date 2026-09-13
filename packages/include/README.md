# packages/include — Vendored Headers

Dependency vendoring for hermetic builds of the ASFDK-C++ packages in
environments without vcpkg/CMake. Add `-I packages/include` to the compiler
invocation (after the package's own `-I packages/<pkg>/include`).

This directory is a build-time fallback only. The canonical dependency
source remains vcpkg (`vcpkg.json` manifests per package); when a full
toolchain is available, builds should prefer the vcpkg-provided versions
and may ignore this directory entirely.

| Header | Upstream | Version | License | Provenance |
|---|---|---|---|---|
| `nlohmann/json.hpp` | https://github.com/nlohmann/json | 3.11.3 | MIT | Official single-header release (`json.hpp` from the `v3.11.3` release) |
| `tl/expected.hpp` | https://github.com/TartanLlama/expected | 1.3.1 | CC0 1.0 | Upstream `include/tl/expected.hpp` |
| `spdlog/spdlog.h` | https://github.com/gabime/spdlog | shim | MIT (project) | Local compatibility shim — see below |

## spdlog shim

`spdlog/spdlog.h` is **not** upstream spdlog. It is a minimal, dependency-free
compatibility shim implementing the small spdlog API surface used by the NLT
C++ ports (`spdlog::set_level`/`get_level`, `info`/`warn`/`error` with `{}`
placeholder formatting, output to stderr, `SPDLOG_SHIM_QUIET` to silence).
Replace with the real spdlog dependency when the full toolchain is available.

## Used by

| Package | Headers required |
|---|---|
| `packages/toi` | `tl/expected.hpp`, `nlohmann/json.hpp`, spdlog shim |
| `packages/otoi` | `nlohmann/json.hpp`, spdlog shim |
| `packages/sleepwalker` | `nlohmann/json.hpp`, spdlog shim |
| `packages/rrt-advocate` | spdlog shim (test runner) |

## Local build/verification example

```sh
# TOI
g++ -std=c++20 -I packages/toi/include -I packages/include \
    packages/toi/tests/standalone_test.cpp packages/toi/src/*.cpp -o /tmp/toi_test

# OTOI
g++ -std=c++23 -I packages/otoi/include -I packages/include \
    packages/otoi/tests/standalone_test.cpp packages/otoi/src/*.cpp -o /tmp/otoi_test

# Sleepwalker
g++ -std=c++20 -I packages/sleepwalker/include -I packages/include \
    packages/sleepwalker/tests/standalone_test.cpp packages/sleepwalker/src/*.cpp -o /tmp/sw_test
```

Governed by: ORG-DEV-OTOI-1.0.3
