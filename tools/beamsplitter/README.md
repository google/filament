# beamsplitter

- [Description](#description)
- [Instructions](#instructions)
- [Input Limitations](#input-limitations)
- [Magic Strings](#magic-strings)
- [Input Files](#input-files)
- [Output Files](#output-files)

### Description

This Go program consumes C++ header file(s) and generates Java bindings, JavaScript bindings, and
C++ code that performs JSON serialization.

### Instructions

To install the Go compiler on macOS, just do:

    brew install go

To build and invoke the code generator, do:

    cd tools/beamsplitter ; go run .

### Input Limitations

The source files must have very simple C++ syntax. Some of the limitations include:

- Only `enum class` is supported; no old-style enums.
- Opening braces for `enum` and `struct` must live at the end of a codeline.
- Enum values must be sequential and cannot have custom values.
- There are no namespaces other than the top-level namespace.
- Every struct field must supply a default value on a single codeline using the = operator.
- If the default value of a field is a vector, it must be in the form: `{ x, y, z }`.
- There must be no string literals that contain keywords.

### Magic Strings

If the comment for a struct field contains the string `%codegen_skip_json%`, then the field is
skipped when generating JSON serialization code.

If the comment for a struct field contains the string `%codegen_skip_javascript%`, then the field is
skipped when generating JavaScript and TypeScript bindings.

### Input Files

- `filament/include/filament/Options.h`

### Output Files

 The following files are created:

- `libs/viewer/src/Settings_generated.h`
- `libs/viewer/src/Settings_generated.cpp`
- `web/filament-js/jsbindings_generated.cpp`
- `web/filament-js/jsenums_generated.cpp`
- `web/filament-js/extensions_generated.js`
- `android/filament-android/src/main/cpp/View_generated.cpp` (TBD)

Additionally, in-place edits are made to the following files:

- `web/filament-js/filament.d.ts`
- `android/filament-android/src/main/java/.../View.java` (TBD)
