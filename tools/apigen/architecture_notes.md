# APIGen Architecture Rationalization

## Helper Definitions
- **Extractor**: Front-end tool (Python/libclang) that parses C++ headers.
- **Generator**: Back-end tool that reads IR and outputs JS/Java/Rust.
- **IR**: Intermediate Representation (JSON).

## 1. The "Compressed" Template Strategy

### The Problem
C++ templates use compile-time polymorphism. 
```cpp
template<typename T> void setParameter(const char* name, T value);
```
Many bindings generators "explode" this at the Extractor level, producing:
- `setParameter_float`
- `setParameter_int`
- `setParameter_vec3`

### The Solution: Compressed Representation
We represent the template **generically** in the IR, capturing the semantic intent.

### Rationalization
1.  **Single Source of Truth**: The IR reflects the C++ *source*, not the compiled ABI. This keeps the IR stable.
2.  **Language Agnostic**:
    - **Rust**: Can bind to a generic function `fn set_parameter<T>(name: &str, value: T)`.
    - **Java**: Does not support primitives in generics easily. The Java Generator can read the `specializations` field and generate overloads (Unrolling) *at generation time*.
    - **JS**: Can generate a single function that inspects the runtime type or uses a DataView.
3.  **Configurable Specialization**: If we add a new math type (`half4`), we don't need to re-parse the C++ headers. We just add "half4" to the generator config, which injects it into the `specializations` list.

## 2. Backend Filtering

We introduce a mandatory `attributes` field to all entities.
This supports removing APIs that don't make sense for a specific target language (e.g., `getNativeHandle()` for WebGL/JS) without polluting the header files with `#ifdef __EMSCRIPTEN__` which might confuse the Extractor if not set up perfectly.

## 3. Internal Aliases (Re-exports)

Filament often exploits restricted visibility by defining types in a "backend" namespace and re-exporting them in public classes:
```cpp
// internal header
class BackendThing {};

// public header
class PublicClass {
    using Thing = filament::internal::BackendThing;
};
```
We capture these aliases so Generators can mimic this structure.
- **Java**: Needs to either expand the alias (if it's a value) or generate a nested static wrapper/shim to point to the internal type.
- **Rust**: Can use `pub type Thing = ...`.

## 4. Nullability Support

Pointers in C++ are assumed valid but can be null. Filament uses `UTILS_NULLABLE` (`_Nullable`) and `UTILS_NONNULL` (`_Nonnull`) to specify contracts.
- **Schema**: `Type.nullability` field.
    - `unspecified`: Default.
    - `nullable`: Explicitly allows null.
    - `nonnull`: Strictly forbidden.
- **Generators**:
    - **Java**: `@Nullable` / `@NonNull`.
    - **Rust**: Map `nullable` -> `Option<*mut T>`, `nonnull` -> `&mut T` (logic dependent).

## 5. Buffer Size Resolution & Dual-Mode Marshaling

C++ bounded array parameters are expressed type-safely using `utils::Slice`:
```cpp
UTILS_APIGEN_ALTERNATE_NAME(setBonesAsMatrices)
void setBones(Engine& engine,
        utils::Slice<const math::mat4f> transforms,
        size_t offset = 0);
```
- **Schema**:
  - `Method.attributes`: Contains `["filament:apigen:alternate_name:<newName>"]`.
  - `Argument.type`: Contains `Slice<...>` metadata (`category: "slice"`). Legacy IRs may also specify `attributes: ["filament:apigen:size_param:<countParam>"]`.
- **Generators**:
  - **Alternate Names**: Generates clean target language names (`setBonesAsMatrices`, `setBonesAsQuaternions`) to disambiguate overloaded methods or keyword collisions (`payload` for `package`).
  - **Dual-Mode Buffers**: For large contiguous data (e.g. bone transforms), generators emit both:
    1. Direct NIO `Buffer` overloads for zero-copy native memory transfers with capacity checks.
    2. Typed primitive arrays (`float[]` with `@Size(min = stride)`) with automatic element stride calculations (`Bone` = 8 floats, `mat4f` = 16 floats), length bounds checking, and convenience overloads omitting explicit counts.

## 6. Type Naming Strategy

We use a "Split" approach for Type names to balance precision and readability:
- **`cpp_name`**: The spelling from the source (e.g. `float3`). Used for documentation and error messages.
- **`qualified_name`**: The Fully Qualified Name (e.g. `filament::math::float3`). Used for strict type linking and imports.

## 7. Structured Documentation

We treat documentation as data, not strings.
- `params` map allows us to reconstruct `@param name description` in Javadoc or `* name - description` in Rustdoc.
- `brief` vs `details` separation is critical for IDE popups (IntelliJ/VSCode) which often show only the brief summary by default.
- **Markdown Contract**: The `details` field (and others) are GUARANTEED to be CommonMark. The Extractor normalizes Doxygen tags (`\bold`, `\c`, `\li`) into Markdown (`**bold**`, `` `code` ``, `- item`).
    - **Rust**: Uses Markdown natively.
    - **Java**: Generator converts Markdown to HTML.
    - **JS**: VSCode supports Markdown in JSDoc.

## 8. Value Objects & Aggregate Structs Strategy

Filament distinguishes between three categories of classes:
1. **Managed Engine Objects** (`mNativeObject` handle): Stateful C++ heap allocations (e.g. `Camera`, `Scene`, `LightManager`).
2. **Encapsulated Value Classes**: Self-contained value types with private invariant storage (e.g. `Frustum` storing 6 `float4` planes). In Java, they hold an inline backing buffer (`mPlanes = new float[24]`) and re-interpret memory in JNI without heap allocations.
3. **Pure Aggregate Structs**: Data-only composite structures with all-public fields (e.g. `Box`, `Aabb`).

### Zero-Allocation Java Aggregate Structs
* **Exploded Scalar Storage**: Rather than storing composite member objects or auxiliary arrays, aggregate structs decompose their fields into private primitive scalars in Java (e.g. `private float mCenterX, mCenterY, mCenterZ, mHalfExtentX, mHalfExtentY, mHalfExtentZ;`).
* **1-Object Allocation**: Instantiating `new Box()` allocates exactly 1 single object on the JVM heap with 0 auxiliary array objects.
* **Reusable Out-Getters**: Methods provide zero-allocation out-parameter signatures (`@NonNull float[] getCenter(@Nullable float[] out)`) alongside convenience allocations (`getCenter()`).
* **Register-Passed JNI Calls**: When an aggregate struct is passed to a native API (e.g. `Frustum.intersects(Box)`), Java unpacks the scalar leaves directly into the native method call. JNI receives them in CPU registers (AAPCS64 `s0-s5`) and constructs the C++ struct on the stack with 0 JNI array allocations, locking, or pinning.
* **Nested Aggregate Structs (`ShadowOptions.Vsm`)**: Aggregate structs defined within classes or outer structs emit as nested `public static class` structures within their enclosing parent classes in Java, avoiding standalone `.java`/`.cpp` files while supporting multi-level hierarchical leaf access chains (`options.getVsm().getElvsm()`).
* **Fixed-Size Struct Arrays**: Struct fields declared as fixed-size C arrays (e.g. `float cascadeSplitPositions[3]`) map to `@NonNull @Size(min = 3) float[]` with indexed primitive fields (`mCascadeSplitPositions0`, `1`, `2`) in Java and element-by-element assignment in C++.

### Note on `Frustum` Special-Casing
`Frustum` represents an encapsulated value class: it possesses private storage (`private: math::float4 mPlanes[6];`) rather than public fields, meaning it does not pass the AST pure-aggregate heuristic. It is currently handled via a dedicated value-class code path (`self.is_value_class = (self.name == "Frustum")`), which generates an inline `final float[] mPlanes = new float[24]` backing array and uses `reinterpret_cast<Frustum*>(planes)` in JNI. This special-case will be retained until additional opaque/encapsulated value classes are introduced, at which point a generalized `category: "value_class"` extraction and buffer-mapping pipeline will be formalized.

## 9. Include Mapping

We explicitly separate `user` and `system` includes to allow the Java Backend to perform "Package Resolution".
- `system: ["<vector>"]` -> `import java.util.List;`
- `user: ["math/mat4.h"]` -> `import com.google.android.filament.math.Mat4;`

This IR design allows the Frontend to be "dumb" (just parse what's there) and the Backend to be "smart" (map to target idioms).

## 10. Phase 1 Core Type System & Marshalling

### String & String-View Types
* **C++ Types**: `std::string_view`, `std::basic_string_view<char>`, `utils::CString`, `utils::StaticString`, `utils::ImmutableCString`, `std::string`, `const char*`.
* **Java Target**: `@NonNull String` (or `@Nullable String`).
* **JNI Bridge**:
  * Inputs: `char const * const val = env->GetStringUTFChars(val_, nullptr);` with bracketed `env->ReleaseStringUTFChars(val_, val);` forwarding `std::string_view(val)` or `utils::*String(val)` to C++.
  * Outputs: Returns `jstring` via `env->NewStringUTF(res.c_str())` or `env->NewStringUTF(std::string(res).c_str())`.

### Chrono & Time Types
* **C++ Types**: `std::chrono::duration<...>`, `std::chrono::time_point<...>`.
* **Java Target**: `@IntRange(from = 0) long` (nanoseconds) or `double` (seconds).
* **JNI Bridge**: Direct conversion via `std::chrono::nanoseconds(val)` and `(jlong)res.time_since_epoch().count()`.

### Tribool & Optional Primitives
* **C++ Types**: `utils::tribool`, `std::optional<bool>`.
* **Java Target**: `boolean`.
* **JNI Bridge**: Direct cast `(utils::tribool)val` and `std::make_optional((bool)val)`.

### Bitmask & Flag Sets
* **C++ Types**: `utils::bitset<uint32_t>`.
* **Java Target**: `@IntRange(from = 0) int`.
* **JNI Bridge**: `(jint)res.to_ulong()` and `{type}(val)`.

### Struct Slices & Memory Safety (`utils::Slice<const StructT>`)
* **C++ Types**: `utils::Slice<const HardwareTimeline>`, etc.
* **Java Target**: `@Nullable HardwareTimeline[]` (in high-level Java API) and `long[]` (in JNI native declaration).
* **JNI Bridge**:
  * Unpacks flat `long[]` to `jlong*` via `env->GetLongArrayElements`.
  * Re-interprets contiguous element structs via `reinterpret_cast<HardwareTimeline*>(elements)`.
  * Passes `utils::Slice<const HardwareTimeline>{ structs, count }` to C++.
  * Strictly releases the array via `env->ReleaseLongArrayElements(elements, JNI_ABORT)` post-call to avoid JVM buffer pinning and leaks.

## 11. Android Platform Extension Pattern (e.g. `FramePacer`)

Core Filament APIs remain decoupled from Android-specific SDK frameworks (e.g., `android.view.Choreographer` and `android.os.Build`), allowing pure C++ engine classes to be bound into `com.google.android.filament.*`.

When an engine class benefits from platform-specific integration:
1. **Pure Binding**: `com.google.android.filament.FramePacer` is generated 100% automatically by APIGen from `FramePacer.h` and bound to generated `FramePacer.cpp` in JNI.
2. **Platform Helper**: `com.google.android.filament.android.FramePacer` wraps the generated instance and provides Android SDK extensions (e.g., `@RequiresApi(33) setupFrame(Choreographer.FrameData, long)` and backwards-compatibility accessors), delegating all native interactions directly to the generated binding.
3. **Zero Handwritten JNI**: Handwritten JNI files (such as `FramePacerAndroidJni.cpp`) are entirely eliminated in favor of generated JNI bridges.

## 12. Modular Generator Architecture (JavaGen Subpackage)

The Java & JNI code generator is organized as a modular Python package under `tools/apigen/javagen/` with a backward-compatible CLI and module facade at `tools/apigen/javagen.py`. This decomposes the binding pipeline into distinct layers with strict separation of concerns, comprehensive PEP 484 typing, and zero-change functional parity across all Filament targets.

### Package Structure & Layered Responsibilities

```
tools/apigen/
├── javagen.py                      # Standalone CLI entrypoint & backward-compatible API facade
└── javagen/                        # Modular package
    ├── __init__.py                 # Public package interface re-exporting core classes & tables
    ├── errors.py                   # Fine-grained custom exception hierarchy
    ├── config.py                   # Type mapping tables, math/value registries, keywords, constants
    ├── utils.py                    # Pure AST attribute extractors, sanitizers, naming utilities
    ├── doc.py                      # CommonMark-to-Javadoc translation and tag formatting
    ├── type_resolver.py            # TypeResolver engine (type resolution, layouts, buffer classes)
    ├── context.py                  # ClassContext data model, IR parsing, SFINAE template expansion
    ├── java_emitter.py             # JavaEmitter (Java wrappers, builders, aggregate structs)
    ├── jni_emitter.py              # JniEmitter (zero-cost JNI C++ bridges, buffer & slice unpackers)
    └── orchestrator.py             # process_file multi-file IR pipeline and CLI argument parser
```

### Key Subsystems & Design Invariants

1. **Custom Exception Hierarchy (`errors.py`)**:
   * Rooted at `JavaGenError` to distinguish code generation failures from standard Python runtime errors.
   * Fine-grained subtypes for each lifecycle stage: `TypeResolutionError`, `TemplateExpansionError`, `EmissionError`, and `IRParseError`.

2. **Decoupled Type Resolution Engine (`type_resolver.py`)**:
   * `TypeResolver` operates independently of code emission, encapsulating:
     * Mapping C++ types to Java/JNI representations and annotation constraints (`resolve_type_info`).
     * Recursive aggregate struct leaf flattening and accessor traversal (`_get_struct_leaf_fields`).
     * Buffer classification: distinguishing packed buffers, retained buffers, and primitive arrays.
     * JNI stack reconstruction and memory-safe cleanup (`build_struct_from_slice`, `build_struct_cleanup`).

3. **Context Model & SFINAE Trait Expansion (`context.py`)**:
   * `ClassContext` serves as the single source of truth for extracted class metadata.
   * Expands template specializations dynamically (`_expand_method`) to generate unrolled overloads.
   * Tracks JNI signatures and detects overload collisions (`track_signature`).
   * Delegates code generation tasks directly to `JavaEmitter` and `JniEmitter`.

4. **Dedicated Java and JNI Emitters (`java_emitter.py`, `jni_emitter.py`)**:
   * Code generation logic is isolated into dedicated emitter classes instantiated with an active `ClassContext`.
   * Transparent context attribute forwarding (`__getattr__`) ensures clean class boundaries while avoiding redundant state duplication.
   * `JavaEmitter` synthesizes Java class bodies, builder inner classes, zero-allocation aggregate structs, vector array overloads, and JNI declarations.
   * `JniEmitter` emits zero-cost JNI C++ translation units, `wrapJni` exception safety blocks, struct stack reconstructions, and callback dispatches.

5. **Backward-Compatible Facade (`javagen.py`)**:
   * Preserves exact CLI invocation syntax (`python3 tools/apigen/javagen.py ...`).
   * Re-exports key APIs (`ClassContext`, `JavaEmitter`, `JniEmitter`, `TypeResolver`, `process_file`, `main`, `generate_javadoc`, `format_see_tag`, `TYPE_MAP`, etc.) so that existing tools (`generate_android.py`) and test suites (`test_javagen.py`) operate seamlessly without modification.

6. **Strict Bit-for-Bit Parity**:
   * Any architectural refactoring must produce zero changes in generated bindings across all 38 Android Java and JNI C++ targets (`git diff android/` must be completely empty).


