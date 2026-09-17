# Apigen Emitter Audit: Hardcoded Symbols & Unhardcoding Roadmap

This document catalogs all hardcoded class, struct, and method names within the Filament `apigen` binding emitters ([`tools/apigen/javagen/java_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py) and [`tools/apigen/javagen/jni_emitter.py`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py)).

For each entry, this audit explains the technical rationale for its presence, its current implementation mechanism, and an assessment of its feasibility for removal (unhardcoding).

---

## 1. Comprehensive Audit Matrix

| Location | Hardcoded Symbol(s) | Category | Rationale & Context | Low-Hanging Fruit? | Unhardcoding Strategy |
| :--- | :--- | :--- | :--- | :---: | :--- |
| [`java_emitter.py:465`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L465) | ~~`self.name == "Texture"`~~ | Import Collection | **RESOLVED**: Replaced with semantic `has_pbd_alias` detection across `self.raw_aliases`. | **RESOLVED** | Checked in. |
| [`java_emitter.py:610`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L610) | ~~`self.name == "Colors"`~~ | Import Collection | **RESOLVED**: Replaced with `"LinearColor" in self.aliases` check. | **RESOLVED** | Checked in. |
| [`java_emitter.py:1770`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1770) | ~~`name == "wait"`~~ | Language Collision Guard | **RESOLVED**: Replaced with `JAVA_OBJECT_FINAL_NOARG_METHODS` language collision set in `config.py`. | **RESOLVED** | Checked in. |
| [`jni_emitter.py:755`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L755)<br>[`jni_emitter.py:1904`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L1904) | ~~`m_name == "name"`~~<br>~~`name == "name"`~~ | Parameter Expansion | **RESOLVED**: Deprecated `(char const*, size_t)` and `StaticString` marked `UTILS_NOAPIGEN`, and `ImmutableCString` builder overload added in C++, removing the special-case `strlen` parameter expansion. | **RESOLVED** | Checked in. |
| [`java_emitter.py:729`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L729)<br>[`java_emitter.py:879`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L879)<br>[`java_emitter.py:1848`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1848) | `self.name == "MaterialInstance"` | Retained Nullability | Emits `@Nullable Material` on `mMaterial`, constructor parameters, and `getMaterial()` because the legacy single-arg `MaterialInstance(long)` constructor creates instances without a Java `Material` wrapper. | **Yes** (Tier 1) | Support nullability directly in retained reference metadata (e.g. `UTILS_APIGEN_RETAINED(Material) UTILS_NULLABLE`), making nullability purely data-driven. |
| [`java_emitter.py:3122`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L3122) | `self.name == "MaterialInstance"` & `name == "duplicate"` | Retained Factory | Generates `new MaterialInstance(nativeInstance, other.getMaterial())` when duplicating a `MaterialInstance`. | **Medium** (Tier 2) | Generalized retained copy rule: if class `T` retains reference `R`, any method `T duplicate(const T&)` forwards `other.getR()`. |
| [`java_emitter.py:892`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L892) | `self.name == "MaterialInstance"` | Legacy Constructor | Emits deprecated single-argument fallback constructor `public MaterialInstance(long nativeMaterialInstance)`. | **Medium** (Tier 2) | Phase out once legacy internal callers migrate to two-arg constructor, or annotate with a specific legacy compatibility attribute. |
| [`java_emitter.py:924`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L924) | `alias_name == "PixelBufferDescriptor"` | Backward Compatibility | Emits deprecated static inner class `Texture.PixelBufferDescriptor extends com.google.android.filament.PixelBufferDescriptor`. | **Medium** (Tier 2) | Generalize via a type alias rule that synthesizes inheritance shims for exported type aliases (`using X = Y;`). |
| [`java_emitter.py:696`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L696) | `self.name == "ToneMapper"` | Backward Compatibility | Emits static subclasses `ToneMapper.Linear`, `ToneMapper.ACES`, etc. for backward compatibility from when tone mappers were inner classes. | **Medium** (Tier 2) | Retain as explicit backward-compat shims, or drive via explicit `@deprecated_alias` annotations. |
| [`java_emitter.py:1006`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1006)<br>[`jni_emitter.py:987`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L987) | `self.name == "ToneMapper"` | Lifecycle / Destructor | Generates `finalize()` that calls JNI `nDestroyToneMapper(long)` which calls `delete (ToneMapper*) toneMapper;` in C++. ToneMapper is heap-allocated via `new` rather than managed by `Engine`. | **Medium** (Tier 2) | Introduce `UTILS_APIGEN_DESTRUCTIBLE` (or `UTILS_APIGEN_MANAGED_HEAP`) macro to automatically synthesize `finalize()` and native `delete` for standalone heap types. |
| [`java_emitter.py:1585`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1585) | `struct_cls["name"] == "VsyncTick"` | Struct Unpacking Overloads | Unpacks `FramePacer::setupFrame(const VsyncTick&)` into three convenience overloads with a default 16.6ms vsync period. | **Medium** (Tier 2) | Struct unpacking could be driven by an annotation (e.g. `UTILS_APIGEN_UNPACK_STRUCT`), though the 16.6ms default period is FramePacer-specific. |
| [`java_emitter.py:732`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L732)<br>[`java_emitter.py:900`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L900)<br>[`java_emitter.py:1886`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1886) | `self.name == "Material"` (`mDefaultInstance`) | Singleton Child Caching | `Material` instantiates and caches a singleton `MaterialInstance mDefaultInstance` in its constructor and returns it in `getDefaultInstance()`. | **Medium / High** (Tier 3) | Annotate `getDefaultInstance` with a cached singleton child annotation, or treat as a specialized retained child pattern. |
| [`java_emitter.py:3126`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L3126) | `self.name == "Material"` & `name == "createInstance"` | Retained Factory | Generates `return new MaterialInstance(result, this);` so that created instances retain the parent `Material`. | **Medium / High** (Tier 3) | Generalize via the retained reference model: if return type `T` retains parent `P`, auto-pass `this` into `new T(result, this)`. |
| [`java_emitter.py:1903`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L1903)<br>[`java_emitter.py:3224`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L3224)<br>[`jni_emitter.py:958`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L958)<br>[`jni_emitter.py:1054`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L1054) | `self.name == "Material"` & `name == "getParameters"` | Struct Array Reflection | Custom reflection bridge: synthesizes allocating and caller-allocated `getParameters()` overloads, and custom JNI marshaling to populate Java `ParameterInfo` objects from C++ structs. | **High** (Tier 4) | Full struct array out-parameter reflection generator in JNI. High complexity; recommended to keep as a specialized bridge. |
| [`java_emitter.py:3480`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L3480)<br>[`java_emitter.py:3842`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L3842)<br>[`java_emitter.py:4198`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/java_emitter.py#L4198)<br>[`jni_emitter.py:516`](file:///Users/mathias/sources/git/filament/tools/apigen/javagen/jni_emitter.py#L516) | `name == "build"`<br>`m_name == "build"` | Architectural Invariant | Maps `build()` methods to `nBuilderBuild` and returns the newly instantiated parent entity. | **Keep** (Invariant) | Core architectural invariant of the Filament Builder pattern (`archetype == "builder"`). Not a one-off class hack. |

---

## 2. Priority Action Roadmap

### Tier 1: Low-Hanging Fruit (Immediate Wins)
These items can be refactored with minimal risk and zero API breakage:
1. **`Texture` (Line 465) & `Colors` (Line 610) Import Checks**:
   - In `java_emitter.py`, replace explicit class name checks with AST inspections of inner classes/aliases for `ByteBuffer` and `LinearColor`. (Done)
2. **`wait` Keyword Guard (Line 1770)**:
   - Replace `if name == "wait"` with a shared `JAVA_OBJECT_RESERVED` set containing `{"wait", "equals", "hashCode", "toString", "notify", "notifyAll", "getClass"}`. (Done)
3. **`name` Method String Pairing (JNI Lines 755 & 1904)**:
   - Deprecate `(char const*, size_t)` and mark with `UTILS_NOAPIGEN`, mark `StaticString` with `UTILS_NOAPIGEN`, and expose clean `utils::ImmutableCString` builder overload in C++, eliminating special-case `strlen` parameter expansion. (Done)
4. **`MaterialInstance` Retained Nullability (Lines 729, 879, 1848)**:
   - Make `@Nullable` directly driven by `UTILS_NULLABLE` metadata attached to the retained reference declaration.

### Tier 2: Medium Effort (Retained Lifecycles & Destructors)
1. **Generic Heap Destructor (`ToneMapper`)**:
   - Add `#define UTILS_APIGEN_DESTRUCTIBLE [[clang::annotate("filament:apigen:destructible")]]` to `compiler.h`.
   - When present on a class, automatically emit Java `finalize()` / `nDestroy<Class>` and C++ JNI `delete ptr;`.
2. **Generic Retained Factory (`Material.createInstance` & `MaterialInstance.duplicate`)**:
   - In `java_emitter.py`, when a method returns a type that retains the enclosing class, automatically emit `new Target(result, this)`.
   - When a method duplicates an instance of the same class, automatically forward the source's retained reference.

### Tier 3: Language Guards & Architectural Invariants (Safe to Keep)
1. **Builder `build()`**:
   - The Filament Builder pattern is an invariant across all builders in the codebase. Identifying `build()` as the terminal method is a core convention of the generator archetype.

### Tier 4: Complex Reflection (Keep Specialized)
1. **`Material.getParameters()`**:
   - Marshals an array of `ParameterInfo` structs across JNI by reflecting field names and types. Because `Material` is currently the only class requiring this struct-array reflection bridge, keeping it specialized avoids substantial generator complexity.
