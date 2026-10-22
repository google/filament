# Filament APIGen: Master Technical Guide & AI Agent Entry Point

This document is the **primary technical reference and operational entry point** for AI coding assistants, agents, and engineers working on the Filament API binding generation toolchain (`tools/apigen/`).

Whenever you begin a new chat or session working on APIGen, language bindings, or Android JNI generation in Filament, consult and adhere to this document.

---

## 1. Executive Directive for AI Agents

You are acting as a **Principal C++ and Systems Software Engineer** specializing in zero-cost abstractions, compiler tooling, and high-performance cross-language FFI (Foreign Function Interface) systems.

### Core Mission
Your objective is to transition Filament's language bindings from brittle, handwritten JNI/Java code to an automated, compiler-verified binding pipeline driven by Clang AST reflection. The generated code must achieve:
1. **Zero Runtime Overhead**: Direct register passing, stack-allocated structs, zero heap allocations for value classes, and inline native calls.
2. **Type Safety & Idiomatic APIs**: Java APIs must follow Android/Java idioms (`@NonNull`, `@Nullable`, `@IntRange`, `@Size`, fluent builders, enum type safety), while native JNI code must strictly obey C++17/20 contracts.
3. **Data-Driven Generality**: Never hardcode class-specific or parameter-specific special cases in the generator unless strictly necessary. If a special case is unavoidable, you **must explicitly notify the user and document it in your plan**.

---

## 2. Architecture & Pipeline Overview

APIGen is built as a strictly decoupled, two-stage compiler pipeline:

```
  +-------------------------------------------------------+
  |               Filament Public C++ Headers             |
  |             (filament/include/filament/*.h)           |
  +---------------------------+---------------------------+
                              |
                              v  libclang (AST visitor, SFINAE parser, Doxygen)
  +-------------------------------------------------------+
  |                extractor.py (Frontend)                |
  +---------------------------+---------------------------+
                              |
                              v  JSON Intermediate Representation (ir_def.ts)
  +-------------------------------------------------------+
  |              Intermediate Representation (IR)         |
  +---------------------------+---------------------------+
                              |
                              v  Modular AST emitters (javagen/ package)
  +-------------------------------------------------------+
  |             javagen (Backend Code Generator)          |
  +--------------+--------------------------+-------------+
                 |                          |
                 v                          v
    +-------------------------+  +--------------------------+
    |   Java Wrapper Class    |  |     Zero-Cost JNI C++    |
    |         (*.java)        |  |          (*.cpp)         |
    | com.google.android.     |  | wrapJni, AutoBuffer,     |
    | filament.*              |  | register struct passing  |
    +-------------------------+  +--------------------------+
```

### Stage 1: Frontend Extractor (`extractor.py`)
- Traverses public C++ headers using `libclang` Python bindings.
- Resolves fully qualified types, unwinds typedef aliases (while preserving sized integer types like `uint32_t` and `size_t`), and inspects pointers/references.
- Normalizes Doxygen comments into structured CommonMark IR (`brief`, `details`, `params`, `returns`, `meta`).
- Scans AST tokens across headers to discover `std::enable_if_t` SFINAE traits (`is_supported_parameter_t<T>`) and emits Cartesian product `specializations`.
- Detects declarative `UTILS_APIGEN_*` Clang attributes and marks AST entities accordingly.
- Classifies classes into high-level behavioral **Archetypes**.

### Stage 2: Intermediate Representation (`ir_def.ts`)
- A strictly typed TypeScript interface definition serving as the contract between frontends and backends.
- Encapsulates `API`, `Class`, `Function`, `Argument`, `Field`, `Constant`, `Enum`, `Alias`, and `Type`.
- Language-agnostic: designed so alternative backends (e.g. WebAssembly/TypeScript, Rust, Swift) can consume the same JSON IR.

### Stage 3: Backend Generator (`javagen/` package & `javagen.py`)
- Modular Python package organized into focused subsystems:
  - `orchestrator.py`: Multi-file driver and CLI argument parser.
  - `context.py`: `ClassContext` data model, SFINAE template specialization unrolling, and method signature tracking.
  - `type_resolver.py`: Type resolution engine, aggregate struct leaf traversal, buffer classification, and JNI stack reconstruction.
  - `java_emitter.py`: Emits Java wrapper classes, nested builders, aggregate structs, convenience overloads, and JNI declarations.
  - `jni_emitter.py`: Emits zero-cost JNI C++ bridges, buffer pointer unwrapping, stack struct reconstruction, and exception-safe `wrapJni` blocks.
  - `validator.py`: Static parity, signature, and reflection descriptor verification engine.
  - `config.py`: Type mapping tables (`TYPE_MAP`, `MATH_TYPES`, `JAVA_KEYWORDS`), dynamic class registries (`KNOWN_CLASSES`).
  - `utils.py`: Attribute extractors, identifier sanitizers, and naming helpers.
  - `doc.py`: CommonMark AST parsing and Markdown-to-Javadoc translation.
  - `errors.py`: Custom typed exception hierarchy (`TypeResolutionError`, `TemplateExpansionError`, etc.).
- Standalone backward-compatible CLI entry point: `javagen.py`.

### Stage 4: Static Verification Engine (`javagen/validator.py` & `verify_bindings.py`)
- Statically audits 100% of Java wrapper classes and JNI C++ translation units:
  - **Bidirectional Symbol Parity**: Guarantees every Java `native` method has an exported C++ JNI bridge and vice-versa, preventing runtime `UnsatisfiedLinkError`.
  - **Parameter & Signature Consistency**: Verifies parameter counts and maps Java types against JNI ABI types across all overloads.
  - **Reflection Integrity**: Verifies all `env->GetFieldID`, `env->GetMethodID`, and `env->FindClass` lookups match existing Java fields and methods with compatible type descriptors, preventing `NoSuchFieldError`.
- Callable standalone via `python3 tools/apigen/verify_bindings.py` or integrated into code generation via `python3 tools/apigen/generate_android.py --validate`.

---

## 3. Class Archetype System

APIGen avoids class-specific branching by grouping all C++ types into 6 behavioral **Archetypes**:

| Archetype | Characteristics | Java Representation | JNI Bridge Pattern | Examples |
| :--- | :--- | :--- | :--- | :--- |
| **`HANDLE`** | Managed engine object wrapping a native pointer. | Holds `private long mNativeObject;`. Methods invoke `getNativeObject()` to assert non-zero handle. | Native methods accept `jlong native<Class>` and cast to `Class*`. | `Camera`, `Scene`, `Texture`, `Material`, `SwapChain`, `Stream` |
| **`BITFIELD`** | Packed scalar bitfield value object (auto-deduced from bitfield struct or wrapper class, $\le$ 64 bits). | Holds primitive scalar (`short`, `int`, or `long`) with zero heap allocations. Mutators return new bitfield values. | Marshaled via `filament::JniUtils::to_short`/`from_short`, `to_int`/`from_int`, or `to_long`/`from_long`. | `TextureSampler` |
| **`INLINE_BUFFER`** | Fixed-size array buffer value class. | Holds inline primitive array (`final float[] mPlanes = new float[24]`). No native handle. | JNI reinterprets array memory directly (`reinterpret_cast<Frustum*>(planes)`). | `Frustum` |
| **`AGGREGATE`** | Pure data-only POD struct (non-polymorphic, all public fields). | Exploded primitive fields (`mCenterX`, `mCenterY`, etc.) or public fields. Zero auxiliary allocations. | Leaves unrolled to JNI scalar parameters; reconstructed on C++ stack. | `Box`, `Aabb`, `Viewport`, `ShadowOptions` |
| **`UTILITY`** | Pure static utility namespace (no fields, all static methods). | `private <Class>() {}` constructor. All methods `public static`. Omits `mNativeObject`. | Native methods omit `native<Class>` and call `Class::method(...)` directly. | `Colors`, `Exposure` |
| **`BUILDER`** | Fluent nested builder class. | Lifecycle via `BuilderFinalizer`. Fluent setters return `this`. Terminal `build(Engine)`. | Creates native builder via `nCreateBuilder`. Direct buffers retained via `BuilderWrapper`. | `IndirectLight.Builder`, `Skybox.Builder`, `Stream.Builder` |

---

## 4. Documentation Navigational Map

Consult the following deep-dive specifications depending on your task:

```
tools/apigen/
├── README.md                 -> Executive project summary and quickstart
├── apigen.md                 -> Master agent guide, operational guardrails, and recipes (YOU ARE HERE)
├── verify_bindings.py        -> Standalone CLI static parity & reflection validator
├── javagen.md                -> Comprehensive Java & JNI generator specification
│                                (Type mapping, JNI mangling, buffer unpacking, Javadoc)
├── extractor.md              -> Comprehensive libclang AST extractor specification
│                                (Clang visiting, FQN desugaring, SFINAE discovery, attributes)
├── ir_def.ts                 -> Canonical TypeScript schema for the JSON Intermediate Representation
├── gap_analysis.md           -> Production status scorecard and 100% core coverage roadmap
├── hardcoded_features.md     -> Detailed audit of remaining special cases to un-hardcode into generic attributes
└── beamsplitter_migration.md -> Roadmap for replacing Go beamsplitter with APIGen struct serialization
```

### When to Consult Which Document:
- **Adding or modifying a Java/JNI binding feature**: Read [javagen.md](javagen.md).
- **Adding or modifying C++ AST parsing, annotations, or type extraction**: Read [extractor.md](extractor.md).
- **Modifying the JSON IR schema**: Read and update [ir_def.ts](ir_def.ts).
- **Checking progress or choosing the next target to automate**: Read [gap_analysis.md](gap_analysis.md).
- **Refactoring a hardcoded class name check**: Read [hardcoded_features.md](hardcoded_features.md).
- **Working on `View` or `Options` post-processing structs**: Read [beamsplitter_migration.md](beamsplitter_migration.md).

---

## 5. Agent Operational Guardrails & Invariants

AI agents modifying APIGen or Filament bindings **must strictly observe** the following rules:

### Rule 1: Never Manually Edit Generated Files
Any file that begins with:
```java
// THIS FILE IS GENERATED BY APIGEN AND MUST NOT BE HAND-MODIFIED.
```
must **never** be manually edited. If a generated file has a bug, incorrect type, missing method, or compilation failure:
1. Fix the generator (`javagen/` package),
2. Fix the extractor (`extractor.py`), or
3. Annotate the C++ header (`filament/include/filament/*.h`).

### Rule 2: Strict Platform Separation
APIGen binds core, cross-platform Filament C++ APIs. Platform-specific hooks and OS subsystems (e.g. Android `SurfaceTexture`, `AHardwareBuffer`, `ANativeWindow`, EGL contexts) **must not** be mixed into core generated bindings.
- Exclude platform-specific methods from APIGen using `UTILS_NOAPIGEN`.
- Provide dedicated platform adapters under `com.google.android.filament.android.*Helper` (e.g. `TextureHelper`, `StreamHelper`, `UiHelper`).

### Rule 3: Data-Driven Attributes & Explicit Notification for Hardcoding
- **Prefer Declarative Macros**: Use `UTILS_APIGEN_*` attributes rather than matching against class or method names.
- **Mandatory Notification**: If you *must* hardcode class names, method names, or parameter names (i.e. do anything non-generic), you **must explicitly inform the user and document the rationale in your plan**.

### Rule 4: Mandatory Verification Pipeline
Before concluding any task that touches APIGen or generated bindings, you must execute the verification pipeline (see Section 6, Recipe D).

### Rule 5: Commit Message Conventions
Commit messages must adhere to the **50/72 rule**:
- Subject line: Maximum 50 characters, imperative mood, lowercase prefix (e.g. `apigen: migrate Stream, add StreamHelper`).
- Body: Wrapped at 72 characters, explaining the problem, architectural rationale, and verified tests.

---

## 6. Step-by-Step Development Recipes

### Recipe A: Migrating a New Class to APIGen

1. **Header Audit**:
   - Inspect the public header (`filament/include/filament/<Class>.h`).
   - Identify any methods accepting `void*` platform objects, OS handles, or private structs.
   - Annotate platform-dependent methods with `UTILS_NOAPIGEN`.
   - Accept contiguous bounded buffers using `utils::Slice<T>` or `utils::Slice<const T>`.
   - If the class retains references to dependencies (e.g. Engine, Material), annotate the corresponding getter with `UTILS_APIGEN_RETAINED`.
2. **Register Target**:
   - Add `("<Class>", REPO_ROOT / "filament/include/filament/<Class>.h")` to `TARGETS` in `tools/apigen/generate_android.py`.
3. **Execute Parallel Generation**:
   ```bash
   python3 tools/apigen/generate_android.py -j
   ```
4. **Inspect Diff**:
   - Run `git diff android/` to verify that generated `.java` and `.cpp` files are clean and match the handwritten behavior.
5. **Compile & Verify**:
   - Run unit tests and compilation (Recipe D).
6. **Update Scorecard**:
   - Update `tools/apigen/gap_analysis.md` (increment active target count, remove from gap list).

---

### Recipe B: C++ Header Annotation Reference

All annotations are defined in [`libs/utils/include/utils/compiler.h`](../../libs/utils/include/utils/compiler.h):

| Macro | Underlying Clang Attribute | Usage & Effect |
| :--- | :--- | :--- |
| `UTILS_NOAPIGEN` | `[[clang::annotate("filament:apigen:skip")]]` | Excludes a class, method, field, constant, or enum from binding generation. |
| `UTILS_APIGEN_RETAINED` | `[[clang::annotate("filament:apigen:retained")]]` | Informs generator that the returned object or parameter is retained across the Java wrapper lifetime (synthesizes Java field, constructor parameter, wrap parameter, and direct getter; skips JNI getter). |
| `UTILS_APIGEN_FLAGS` | `[[clang::annotate("filament:apigen:flags")]]` | Marks an enum as a bitmask/flags type (`is_flags = true`). |
| `UTILS_APIGEN_ALTERNATE_NAME(n)` | `[[clang::annotate("filament:apigen:alternate_name:" #n)]]` | Renames a method in Java and JNI export symbol to avoid keyword or signature collisions. |
| `UTILS_APIGEN_TAGGED_ARRAY` | `[[clang::annotate("filament:apigen:tagged_array")]]` | Collapses template array specializations into unified `<Family>Element` enum-tagged array methods. |
| `UTILS_APIGEN_USED_BY_NATIVE` | `[[clang::annotate("filament:apigen:used_by_native")]]` | Marks classes, methods, or fields accessed via native JNI or reflection, emitting `@UsedByNative` to protect against dead-code stripping and obfuscation. |

---

### Recipe C: Un-hardcoding Generator Special Cases

When eliminating a special case from `javagen/`:
1. Check `tools/apigen/hardcoded_features.md` for existing analysis and proposed solutions.
2. If the behavior can be inferred from C++ AST semantics (e.g. non-polymorphic struct with public fields $\rightarrow$ aggregate POD), implement a generic detector in `extractor.py` or `javagen/type_resolver.py`.
3. If semantic intent cannot be deduced from standard C++ syntax, define a new `UTILS_APIGEN_*` macro in `libs/utils/include/utils/compiler.h`, extract it in `extractor.py`, and branch on the attribute in `javagen/`.
4. Run `python3 tools/apigen/generate_android.py -j` and verify that `git diff android/` produces zero unintended behavioral regressions across all 40 production targets.
5. Update `tools/apigen/hardcoded_features.md`.

---

### Recipe D: Standard Verification Pipeline

Always run the following commands before completing a task:

#### 1. APIGen Unit Tests
```bash
python3 -m unittest discover tools/apigen/tests
```
*(All 96+ tests must pass).*

#### 2. Static Binding & Reflection Validation
```bash
python3 tools/apigen/verify_bindings.py
```
*(Validates bidirectional symbol parity, JNI parameter types, and GetFieldID/GetMethodID reflection descriptors).*

#### 3. Code Generation Parity Check
```bash
python3 tools/apigen/generate_android.py -j --validate
git diff android/
```
*(Verify changes are intended and no unexpected modifications occurred).*

#### 4. Core Engine Desktop Compilation
```bash
./build.sh -ip desktop debug
```

#### 5. Android Compilation (Debug & Release)
```bash
# Debug
./build.sh -q arm64-v8a -Pip desktop,android debug

# Release
./build.sh -q arm64-v8a -Pip desktop,android release
```

#### 6. Android Samples (If affected)
```bash
./android/gradlew -p android/samples sample-hello-camera:assembleDebug sample-stream-test:assembleDebug
```

---

## 7. Common Gotchas & Troubleshooting

1. **Negative Enum Values**:
   - C++ enums with negative values (e.g. `FrameStatus::ERROR = -2`) cannot use 0-indexed array lookup tables.
   - `javagen` detects this automatically via `is_custom_enum()` and generates a switch-based `public static Foo from(int value)` lookup method instead of `EnumCache.sValues[idx]`.

2. **Retained Direct NIO Buffers in Builders**:
   - If a C++ builder stores raw pointers to direct NIO buffer memory (e.g. `BufferObject::Builder`), passing an `AutoBuffer` scoped to the JNI setter function will cause a use-after-free when `build()` is called.
   - `javagen` detects this via `has_retained_buffers` and generates a heap-allocated `BuilderWrapper` (`std::vector<std::unique_ptr<AutoBuffer>> retainedBuffers`) that keeps buffers alive until the builder is finalized or destroyed.

3. **Package-Private Class Visibility Across Subpackages**:
   - `com.google.android.filament.*` package-private utilities (like `Asserts`) cannot be accessed by classes in `com.google.android.filament.android.*` (such as `StreamHelper` or `TextureHelper`).
   - Platform helpers must perform their own argument validation or access public APIs only.

4. **Name Collisions with Final `java.lang.Object` Methods**:
   - C++ methods with default parameters can expand into 0-argument Java overloads (e.g. `wait()`).
   - Zero-argument methods matching `JAVA_OBJECT_FINAL_NOARG_METHODS` (`wait`, `notify`, `notifyAll`, `getClass`) are suppressed to avoid colliding with `final` methods in `java.lang.Object`.

5. **JNI Signature Mangling for Overloaded Methods**:
   - Overloaded native methods in JNI require standard signature mangling (e.g. `__JFF`, `__3F`).
   - `javagen` tracks method name frequencies per class and automatically switches from short JNI names (`Java_com_..._nMethod`) to fully mangled names (`Java_com_..._nMethod__<Signature>`) when overloads exist.
