# Dangling Pointer Detector

A pointer is dangling when it references freed memory. Typical examples can be found
[here](https://docs.google.com/document/d/11YYsyPF9rQv_QFf982Khie3YuNPXV0NdhzJPojpZfco/edit?resourcekey=0-h1dr1uDzZGU7YWHth5TRAQ#heading=h.wxt96wl0k0sq).

Dangling pointers are not a problem unless they are subsequently dereferenced and/or used for other
purposes. Proving that dangling pointers are unused has turned out to be difficult in general,
especially in face of future modifications to the code. Hence, they are a source of UaF bugs and
highly discouraged unless you are able to ensure that they can never be used after the pointed-to
objects are freed.

Dawn tests are configured to detect dangling pointers.

## Level of support

The dangling pointer detector is an optional dependency of Dawn. It is enabled only when running
tests in the dawn_standalone configuration. It is also enforced as part of Chrome's tests.

It is currently enforced under this configuration:

| System      | Compiler | Build system | Directory   | Final embedder       | Misc                         |
| ----------- | -------- | ------------ | ----------- | -------------------- | ---------------------------- |
| ✅ Android  | ✅ Clang | ✅ GN        | ✅ src/dawn | ✅ Dawn (standalone) | ❌ windows + debug           |
| ✅ Windows  | ✅ GCC   | ❌ CMake     | ❌ src/tint | ❌ Skia              | ❌ windows + component_build |
| ✅ Mac      | ❌ MSVC  | ❌ Bazel     |             | ✅ Chrome            | ❌ sanitizers                |
| ✅ Linux    | ❌ _     | ❌ _         |             | ❌ _                 | ✅ _                         |
| ✅ ChromeOS |          |              |             |                      |                              |
| ✅ Fuchsia  |          |              |             |                      |                              |
| ❌ iOS      |          |              |             |                      |                              |
| ❌ _        |          |              |             |                      |                              |

## raw_ptr<>

A `raw_ptr<T>` is a non-owning pointer. When using this kind of pointer, the severity of UAFs can be
reduced, because they are protected by
[MiraclePtr/BackupRefPtr](https://security.googleblog.com/2022/09/use-after-freedom-miracleptr.html)

A `raw_ptr<T>` is transparently usable as a `T*`. It should be used in class and struct member
variable.

In general, it shouldn't be used elsewhere: local variable, function arguments, etc...

## raw_ptr flavors

`raw_ptr<T>` comes in 3 main flavors:
```cpp
raw_ptr<T> ptr_never_dangling;
raw_ptr<T, DisableDanglingPtrDetection> ptr_allowed_to_dangle;
raw_ptr<T, DanglingUntriaged> ptr_dangling_to_investigate;
```

The `DisableDanglingPtrDetection` option can be used to annotate “intentional-and-safe” dangling
pointers. It is meant to be used as a last resort, only if there is no better way to re-architecture
the code.

The `DanglingUntriaged` means the pointer was dangling at the time of the
initial rewrite. We should investigate why. No new occurences should be added.

## Pointer arithmetic

The use of pointer arithmetic with `raw_ptr<T>` is discouraged and disabled by default. Usually a
container like `std::span<>` should be used instead of the `raw_ptr`.

The `AllowPtrArithmetic` option can be used to enable pointer arithmetic. For instance:
```
raw_ptr<T, AllowPtrArithmetic> artihmetic_ptr.
```

## Chrome's documentation

The dangling pointer detector is part of Chrome too. You can use the main
documentation:
- [Dangling pointers detector](https://chromium.googlesource.com/chromium/src/+/main/docs/dangling_ptr.md)
- [Fixing dangling pointers guide](https://chromium.googlesource.com/chromium/src/+/main/docs/dangling_ptr_guide.md)

## Dawn's specificities.

Contrary to chrome, Dawn is not yet configured to display useful StackTraces and TaskTraces. It displays the error:
```
DanglingPointerDetector: A pointer was dangling!
                         Documentation: https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/docs/dangling-pointer-detector.md
```

To debug you can:
- **remotely**: Find a failing bot on the CQ running tests inside Chrome. You
  will find useful debugging informations.
- **locally**: Use a debugger to display the StackTrace. Dawn is configured to crash when the
  dangling raw_ptr is released. If needed, you can also understand where the memory was freed by
  replacing `SetDanglingRawPtrReleasedFn` by `SetDanglingRawPtrDetectedFn` in
  src/dawn/tests/PartitionAllocSupport.cpp`.

## Other resources
- [MiraclePtr in Dawn design doc](https://docs.google.com/document/d/1wz45t0alQthsIU9P7_rQcfQyqnrBMXzrOjSzdQo-V-A/edit#heading=h.vn4i6wy373x7)
- [MiraclePtr in chrome](https://chromium.googlesource.com/chromium/src/+/ddc017f9569973a731a574be4199d8400616f5a5/base/memory/raw_ptr.md)
- [Use-after-freedom: MiraclePtr](https://security.googleblog.com/2022/09/use-after-freedom-miracleptr.html)
- [MiraclePtr: protecting users from use-after-free vulnerabilities on more platforms](https://security.googleblog.com/2024/01/miracleptr-protecting-users-from-use.html)
