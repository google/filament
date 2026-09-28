# APIGen: Generator Special Cases & Un-Hardcoding Roadmap

This document catalogs class-specific special cases, hardcoded checks, and manual JNI snippets currently present in the generator, explains why each exists, and proposes declarative C++ annotations or generalized AST pattern-matching rules to eliminate them.

> [!NOTE]
> For a comprehensive, line-by-line audit of all current special-case checks in the codebase (including `View`, `PickingQueryResult`, and POJO option structs), see **[hardcoding_audit.md](hardcoding_audit.md)**.

---

## 1. Context & Motivation

During the initial phase of replacing handwritten bindings in `android/filament-android/` with APIGen, various class-specific behaviors were accommodated using string checks such as `if self.name == "Material":` or `if parent_name == "RenderableManager":`. These hardcoded paths preserved 1:1 behavioral compatibility with legacy handwritten JNI code (e.g. memory management across multiple builder calls, packed struct buffers, and reflection).

To transition APIGen into a fully scalable, generic binding generator across all Filament components and new backends, these special cases should be replaced by:
1. **Declarative C++ Macros / Attributes** (e.g. `UTILS_APIGEN_*`) that capture semantic intent directly in public headers.
2. **Generic Pattern Detection** in the Clang AST extractor and generator.

---

## 2. Inventory of Remaining Special Cases to Un-Hardcode

### 2.1. Material Parameter Reflection (`getParameters()`)

* **Locations in Generator**:
  * [javagen.py:L4029-L4045](javagen.py#L4029-L4045): Hardcoded Java implementation of `ParameterInfo[] getParameters()`.
  * [javagen.py:L6544-L6614](javagen.py#L6544-L6614): 70 lines of handwritten JNI allocating `std::vector<Material::ParameterInfo>`, invoking `that->getParameters(...)`, unpacking anonymous union fields (`type`, `samplerType`, `subpassType`), and allocating Java `ParameterInfo` objects.

* **Reason**:
  C++ uses an output buffer pattern: `size_t getParameters(ParameterInfo* parameters, size_t count) const noexcept`. Java callers expect an allocated `ParameterInfo[]`. Furthermore, `Material::ParameterInfo` contains an anonymous union whose active variant is determined by discriminant booleans (`isSampler`, `isSubpass`).

* **Proposed Solution**:
  * **C++ Output Array Annotation**:
    ```cpp
    UTILS_APIGEN_OUT_ARRAY(getParameterCount)
    size_t getParameters(ParameterInfo* UTILS_NONNULL parameters, size_t count) const noexcept;
    ```
  * **Struct Union Discriminant Annotation**:
    ```cpp
    struct ParameterInfo {
        const char* UTILS_NONNULL name;
        bool isSampler;
        bool isSubpass;
        union {
            UTILS_APIGEN_DISCRIMINANT("!isSampler && !isSubpass")
            ParameterType type;
            UTILS_APIGEN_DISCRIMINANT("isSampler")
            SamplerType samplerType;
            UTILS_APIGEN_DISCRIMINANT("isSubpass")
            SubpassType subpassType;
        };
        uint8_t count;
        Precision precision;
    };
    ```
  * **Generic Generator Implementation**:
    * Java caller invokes the count method (`getParameterCount()`), allocates the array `ParameterInfo[count]`, and passes it to JNI.
    * JNI unpacks union fields according to their boolean discriminant condition expressions without custom class matching.

---

### 2.2. MaterialInstance Duplication & Nullable Parent Retention

* **Locations in Generator**:
  * [`tools/apigen/javagen/java_emitter.py`](java_emitter.py#L3791): Special case for `duplicate()` passing `other.getMaterial()` to `new MaterialInstance(...)`.
  * [`tools/apigen/javagen/java_emitter.py`](java_emitter.py#L1078): Special case synthesizing package-private fallback constructor `MaterialInstance(long) { this(long, null); }`.

* **Reason**:
  `MaterialInstance` has a retained parent reference to `Material`, but can also be instantiated without a Java `Material` wrapper (e.g. from `gltfio` native loaders via `wrap(long)`), requiring nullable parent retention and custom constructor chaining.

* **Proposed Solution**:
  * Generalize fallback 1-arg constructor synthesis for any handle class carrying a nullable retained parent reference (`retained_references`).
  * Add declarative attribute `UTILS_APIGEN_FACTORY(parent = "other.getMaterial()")` for duplication methods.

---

### 2.3. ToneMapper Polymorphic Hierarchy & Standalone Deletion

* **Locations in Generator**:
  * [`tools/apigen/javagen/java_emitter.py`](java_emitter.py#L796): Emits nested static classes `ToneMapper.Linear`, `ToneMapper.ACES`, etc.
  * [`tools/apigen/javagen/jni_emitter.py`](jni_emitter.py#L1059): Custom JNI `nDestroyToneMapper` calling `delete toneMapper;`.

* **Reason**:
  `ToneMapper` is a polymorphic hierarchy allocated with `new` and freed with `delete`, rather than created/destroyed via `Engine`. In Java, concrete mappers were exposed as nested static classes (`new ToneMapper.Linear()`) for discoverability.

* **Proposed Solution**:
  * Detect `virtual ~T() = default;` without `Engine` destruction -> emit standard `destroy()` / `delete that;` JNI wrapper generically.
  * Emit nested static subclass aliases automatically when annotated with `UTILS_APIGEN_NESTED_CHILDREN`.

---

### 2.4. View Legacy Enums & Custom Async Pick

* **Locations in Generator**:
  * [`tools/apigen/javagen/java_emitter.py`](java_emitter.py#L875): Emits deprecated `ToneMapping` and `TargetBufferFlags` enums, deprecated stubs `setToneMapping`/`getToneMapping`, and async `pick(...)` runnable trampoline.
  * [`tools/apigen/javagen/jni_emitter.py`](jni_emitter.py#L1094): Injects custom JNI `Java_com_google_android_filament_View_nPick` with reflection field caching for `InternalOnPickCallback`.

* **Reason**:
  * `ToneMapping` and `TargetBufferFlags` were removed from modern C++ `View.h` (superseded by `ColorGrading` and `RenderTarget`) but remain in `View.java` for Android client backward compatibility.
  * `pick()` requires a custom callback struct (`PickingQueryResult`) unpacking native results.

* **Proposed Solution**:
  * Decorate deprecated legacy methods with an explicit deprecated compatibility attribute or generate them from an auxiliary compatibility definition.
  * Map `View::pick` through the generic async callback pipeline with struct unrolling.

---

### 2.5. Engine Lifecycle (`destroy`) & SwapChain Creation (`createSwapChain`)

* **Locations in Generator**:
  * [`tools/apigen/javagen/java_emitter.py`](java_emitter.py#L2337) & [`tools/apigen/javagen/jni_emitter.py`](jni_emitter.py#L1172): `"destroy"` and `"createSwapChain"` in `intercepted_engine_methods`.
  * [`tools/apigen/javagen/java_emitter.py`](java_emitter.py#L5042): Handwritten `Engine.destroy()`, `createSwapChain` overloads (surface, headless, raw pointer).
  * [`tools/apigen/javagen/jni_emitter.py`](jni_emitter.py#L2526): Handwritten `nDestroyEngine`, `nCreateSwapChain`, `nCreateSwapChainHeadless`, `nCreateSwapChainFromRawPointer`.

* **Reason**:
  * `Engine::destroy(&engine)` is static in C++ and takes a pointer-to-pointer to clear the handle, while Java exposes instance method `engine.destroy()`.
  * `createSwapChain` has two overloads in C++: `createSwapChain(void*, uint64_t)` (windowed) and `createSwapChain(uint32_t, uint32_t, uint64_t)` (headless). On Android, the windowed version requires platform `Surface` unwrapping and retention, while the headless version only needs generic parameters and a 1-arg constructor on `SwapChain`.

* **Proposed Solution**:
  * Decorate `createSwapChain(void*)` with `UTILS_NOAPIGEN`, generate the headless overload generically, and synthesize a fallback constructor `SwapChain(long) { this(long, null); }`.
  * Support static pointer-to-pointer destroy functions as instance `destroy()` in handle classes.

---

## 3. Remaining Implementation Roadmap

| Priority | Feature / Special Case | Primary Mechanism | Files to Modify | Status |
| :--- | :--- | :--- | :--- | :--- |
| **P1** | **Engine SwapChain Generalization (2.5)** | `UTILS_NOAPIGEN` on `void*` overload + fallback constructor `SwapChain(long)` | `Engine.h`, `java_emitter.py`, `jni_emitter.py` | Planned |
| **P1** | **Engine Instance Destruction (2.5)** | Handle instance `destroy()` pattern for `T::destroy(T**)` | `java_emitter.py`, `jni_emitter.py` | Planned |
| **P2** | **MaterialInstance Duplication & Fallback Ctor (2.2)** | Generic fallback constructor for nullable retained references | `java_emitter.py` | Planned |
| **P2** | **Polymorphic Hierarchies (2.3)** | Generic `delete that;` for standalone polymorphic classes | `extractor.py`, `java_emitter.py`, `jni_emitter.py` | Planned |
| **P3** | **Output Array Pattern (2.1)** | `UTILS_APIGEN_OUT_ARRAY(countMethod)` macro + union discriminant | `compiler.h`, `extractor.py`, `javagen/` | Planned |
| **P3** | **View Async Pick Generalization (2.4)** | Generic async callback pipeline with struct unrolling | `java_emitter.py`, `jni_emitter.py` | Planned |


