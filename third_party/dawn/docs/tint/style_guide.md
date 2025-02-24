# Tint style guide

* Generally, follow the [Chromium style guide for C++](https://chromium.googlesource.com/chromium/src/+/HEAD/styleguide/c++/c++.md)
  which itself is built on the [Google C++ style guide](https://google.github.io/styleguide/cppguide.html).

* Overall try to use the same style and convention as code around your change.

* Code must be formatted. Use `clang-format` with the provided [.clang-format](../.clang-format)
  file.  The `tools/format` script runs the formatter.

* Code should not have linting errors.
    The `tools/lint` script runs the linter. So does `git cl upload`.

* Do not use C++ exceptions

* Do not use C++ RTTI.
   Instead, use `tint::Castable::As<T>()` from
   [src/castable.h](../src/castable.h)

* Generally, avoid `assert`.  Instead, issue a [diagnostic](../src/diagnostic.h)
  and fail gracefully, possibly by returning an error sentinel value.
  Code that should not be reachable should call `TINT_UNREACHABLE` macro
  and other internal error conditions should call the `TINT_ICE` macro.
  See [src/debug.h](../src/debug.h)

* Use `type` as part of a name only when the name refers to a type
  in WGSL or another shader language processed by Tint.  If the concept you are
  trying to name is about distinguishing between alternatives, use `kind` instead.

* Forward declarations:
  * Use forward declarations where possible, instead of using `#include`'s.
  * Place forward declarations in their own **un-nested** namespace declarations. \
    Example: \
    to forward-declare `struct X` in namespace `A` and `struct Y`
    in namespace `A::B`, you'd write:
    ```c++
    // Forward declarations
    namespace A {
      struct X;
    }  // namespace A
    namespace A::B {
      struct Y;
    }  // namespace A::B

    // rest of the header code is declared below ...
    ```

## Compiler support

Tint requires C++17.

Tint uses the Chromium build system and will stay synchronized with that system.
Compiler configurations beyond that baseline is on a best-effort basis.
We strive to support recent GCC and MSVC compilers.

## Test code

We might relax the above rules rules for test code, since test code
shouldn't ship to users.

However, test code should still be readable and maintainable.

For test code, the tradeoff between readability and maintainability
and other factors is weighted even more strongly toward readability
and maintainability.
