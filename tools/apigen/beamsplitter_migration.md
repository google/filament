# Beamsplitter vs. APIGen: Gap Analysis & Java Migration Plan

This document details the architectural overlap between [`beamsplitter`](../beamsplitter) and [`apigen`](.), identifies the functional gaps preventing `apigen` from replacing the Java code generation currently performed by `beamsplitter`, and defines a phased plan to migrate all Java and JNI generation for [`View`](../../filament/include/filament/View.h) and [filament/Options.h](../../filament/include/filament/Options.h) into `apigen`.

---

## 1. Executive Summary & Tool Responsibilities

* **Current Division (Post-Migration — September 2026)**:
  * [`beamsplitter`](../beamsplitter):
    1. Generates C++ JSON serialization code for Filament settings (`libs/viewer/src/Settings_generated.h/.cpp`).
    2. Generates Web / JavaScript bindings (`web/filament-js/jsbindings_generated.cpp`, `jsenums_generated.cpp`, `extensions_generated.js`) and TypeScript declarations (`web/filament-js/filament.d.ts`).
    3. **Java Generation Decoupled**: `emitters/java.go` and `emitters/java.template` have been deleted. `beamsplitter` no longer modifies `View.java`.
  * [`apigen`](.):
    1. Complete AST-driven binding generator using `libclang` ([extractor.py](extractor.py)) and modular Python package ([javagen/](javagen/)).
    2. Emits 100% of standalone `.java` and zero-cost JNI `.cpp` files with handle lifecycle validation, zero-allocation enums, aggregate struct register passing, buffer overloads, and POJO option structs.
    3. Generates the complete [`View.java`](../../android/filament-android/src/main/java/com/google/android/filament/View.java) and [`View.cpp`](../../android/filament-android/src/main/cpp/View.cpp) directly from [`View.h`](../../filament/include/filament/View.h) and [`Options.h`](../../filament/include/filament/Options.h).
    4. Manages 41 production targets with bit-for-bit parity.

---

## 2. Detailed Gap Analysis & Resolution

| Capability | [`beamsplitter`](../beamsplitter) | [`apigen`](javagen/) | Status in `apigen` |
| :--- | :--- | :--- | :--- |
| **1. Re-exported Struct Nesting** | Ingests structs from [`Options.h`](../../filament/include/filament/Options.h) and directly nests them as `public static class` inside `View.java`. | [extractor.py](extractor.py) resolves aliased `STRUCT_DECL` cursors in [`View.h`](../../filament/include/filament/View.h) and registers them as nested records in `View` metadata. | **Completed** |
| **2. Public Mutable POJO Structs** | Generates structs with `public` mutable fields and direct field initializers (e.g. `public float minScale = 0.5f;`, `public boolean enabled = false;`). | Implemented **POJO struct archetype** in [javagen/java_emitter.py](javagen/java_emitter.py) emitting public mutable fields with default initializers, preserving Android client API contracts. | **Completed** |
| **3. Default Field Value Desugaring** | Translates C++ default value expressions to Java literals: `INFINITY` $\rightarrow$ `Float.POSITIVE_INFINITY`, `{0, -1, 0}` $\rightarrow$ `{0f, -1f, 0f}`, `nullptr` $\rightarrow$ `null`, `QualityLevel::LOW` $\rightarrow$ `QualityLevel.LOW`. | Implemented C++ $\rightarrow$ Java default value expression translator in [javagen/type_resolver.py](javagen/type_resolver.py). | **Completed** |
| **4. Emitter Directives (`%codegen_*`)** | Parses comment directives: `%codegen_java_float%` (converts `math::float2` to scalar `float` in Java) and `%codegen_java_flatten%` (flattens `Ssct` and `Gtao` into `AmbientOcclusionOptions` with `ssct*` / `gtao*` prefixes). | Parses `%codegen_java_float%` into `apigen:java_type:float` and `%codegen_java_flatten%` into `apigen:flatten`. Inlines sub-structs with prefixes. | **Completed** |
| **5. Option Caching & JNI Unrolling** | **None** (was handwritten in `View.java` and `View.cpp`). | Supports symmetrical option property caching: synthesizes `private <Option> m<Option>;`, lazy default getter in pure Java, unrolled scalar setter to JNI, and C++ struct reconstruction in JNI. | **Completed** |
| **6. Full File vs. In-Place Editing** | In-place edited an existing handwritten file past a marker comment. | Generates complete, independent `.java` and `.cpp` translation units. | **Completed** |

---

## 3. Inventory of Options Structs & Directives in `Options.h`

The 14 post-processing option structs defined in [`filament/include/filament/Options.h`](../../filament/include/filament/Options.h) and aliased in [`filament/include/filament/View.h`](../../filament/include/filament/View.h):

1. **`DynamicResolutionOptions`**:
   - `math::float2 minScale = {0.5f, 0.5f}` (`%codegen_java_float%` $\rightarrow$ `public float minScale = 0.5f;`)
   - `math::float2 maxScale = {1.0f, 1.0f}` (`%codegen_java_float%` $\rightarrow$ `public float maxScale = 1.0f;`)
   - `float sharpness = 0.9f;`
   - `bool enabled = false;`
   - `bool homogeneousScaling = false;`
   - `QualityLevel quality = QualityLevel::LOW;`
2. **`BloomOptions`**:
   - `Texture* dirt = nullptr;` (`%codegen_skip_json% %codegen_skip_javascript%` $\rightarrow$ `@Nullable public Texture dirt = null;`)
   - `float dirtStrength = 0.2f;`
   - `float strength = 0.10f;`
   - `float resolution = 384;`
   - `float threshold = 0.0f;`
   - `bool enabled = false;`
   - `bool blendMode = false;`
   - `bool threshold = false;`
   - `uint8_t levels = 6;`
   - Nested enum: `BloomOptions::BlendMode` (`ADD`, `INTERPOLATE`)
3. **`FogOptions`**:
   - `float distance = 0.0f;`
   - `float maximumOpacity = 1.0f;`
   - `float height = 0.0f;`
   - `float heightFalloff = 1.0f;`
   - `math::float3 color = {1.0f, 1.0f, 1.0f};`
   - `float density = 0.1f;`
   - `float inScatteringStart = 0.0f;`
   - `float inScatteringSize = -1.0f;`
   - `bool fogColorFromIbl = false;`
   - `Texture* skyColor = nullptr;`
   - `bool enabled = false;`
4. **`DepthOfFieldOptions`**:
   - `float cocScale = 1.0f;`
   - `float cocAspectRatio = 1.0f;`
   - `float maxForegroundCOC = 0.05f;`
   - `float maxBackgroundCOC = 0.05f;`
   - `Filter filter = Filter::MEDIAN;`
   - `bool nativeAspectRatio = false;`
   - `uint8_t foregroundRingCount = 0;`
   - `uint8_t backgroundRingCount = 0;`
   - `uint8_t fastGatherRingCount = 0;`
   - `uint8_t maxCoCBilateralFilterRadius = 0;`
   - `bool enabled = false;`
   - Nested enum: `DepthOfFieldOptions::Filter` (`NONE`, `MEDIAN`)
5. **`VignetteOptions`**:
   - `float midPoint = 0.5f;`
   - `float roundness = 0.5f;`
   - `float feather = 0.5f;`
   - `math::float4 color = {0.0f, 0.0f, 0.0f, 1.0f};`
   - `bool enabled = false;`
6. **`RenderQuality`**:
   - `QualityLevel hdrColorBuffer = QualityLevel::HIGH;`
7. **`AmbientOcclusionOptions`**:
   - `AmbientOcclusionType aoType = AmbientOcclusionType::SAO;`
   - `float radius = 0.3f;`
   - `float power = 1.0f;`
   - `float bias = 0.0005f;`
   - `float resolution = 0.5f;`
   - `float intensity = 1.0f;`
   - `float bilateralThreshold = 0.05f;`
   - `QualityLevel quality = QualityLevel::LOW;`
   - `QualityLevel lowPassFilter = QualityLevel::MEDIUM;`
   - `QualityLevel upsampling = QualityLevel::LOW;`
   - `bool enabled = false;`
   - `bool bentNormals = false;`
   - `float minHorizonAngleRad = 0.0f;`
   - Flattened sub-struct: `Ssct ssct;` (`%codegen_java_flatten%`)
     - `ssctLightConeRad`, `ssctShadowDistance`, `ssctContactDistanceMax`, `ssctIntensity`, `ssctLightDirection`, `ssctDepthBias`, `ssctDepthSlopeBias`, `ssctSampleCount`, `ssctRayCount`, `ssctEnabled`
   - Flattened sub-struct: `Gtao gtao;` (`%codegen_java_flatten%`)
     - `gtaoSampleSliceCount`, `gtaoSampleStepsPerSlice`, `gtaoThicknessHeuristic`, `gtaoUseVisibilityBitmasks`, `gtaoConstThickness`, `gtaoLinearThickness`
8. **`MultiSampleAntiAliasingOptions`**:
   - `bool enabled = false;`
   - `uint8_t sampleCount = 4;`
   - `bool customResolve = false;`
9. **`TemporalAntiAliasingOptions`**:
   - `float feedback = 0.95f;`
   - `float filterWidth = 1.0f;`
   - `float sharpness = 0.0f;`
   - `bool enabled = false;`
   - `bool useYCoCg = false;`
   - `BoxType boxType = BoxType::AABB;`
   - `BoxClipping boxClipping = BoxClipping::CLAMP;`
   - `JitterPattern jitterPattern = JitterPattern::UNIFORM_HELIX;`
   - `float varianceGamma = 1.0f;`
   - `bool preventFlickering = false;`
   - `bool historyReprojection = true;`
   - Nested enums: `BoxType`, `BoxClipping`, `JitterPattern`
10. **`ScreenSpaceReflectionsOptions`**:
    - `float thickness = 0.1f;`
    - `float bias = 0.01f;`
    - `float maxDistance = 3.0f;`
    - `float stride = 2.0f;`
    - `bool enabled = false;`
11. **`GuardBandOptions`**:
    - `bool enabled = false;`
12. **`VsmShadowOptions`**:
    - `bool highPrecision = false;`
    - `uint8_t anisotropy = 0;`
    - `float minVarianceScale = 0.5f;`
13. **`SoftShadowOptions`**:
    - `float penumbraScale = 1.0f;`
    - `float penumbraRatioScale = 1.0f;`
14. **`StereoscopicOptions`**:
    - `bool enabled = false;`

---

## 4. Phased Migration Plan & Status

### Phase 1: AST Extractor Enhancements ([extractor.py](extractor.py)) [COMPLETED]

1. **Struct Type Alias Promotion**:
   - In `handle_alias`, when a `TYPEDEF_DECL` or `TYPE_ALIAS_DECL` references a `STRUCT_DECL` (e.g. `using DynamicResolutionOptions = filament::DynamicResolutionOptions;`), extracts the referenced struct AST definition from `Options.h` and registers it in `cls["nested_records"]` (marking `parent_class = "View"` and `is_nested = True`).
2. **Comment Directives / Field Attributes**:
   - Detects `%codegen_java_float%` on struct fields and emits `attributes: ["apigen:java_type:float"]`.
   - Detects `%codegen_java_flatten%` on struct fields and emits `attributes: ["apigen:flatten"]`.
   - Extracts raw default value tokens into `field["default_value"]`.

### Phase 2: JavaGen POJO Struct & Option Pipeline ([javagen/](javagen/)) [COMPLETED]

1. **POJO Struct Archetype (`POJO_STRUCT`) in [javagen/java_emitter.py](javagen/java_emitter.py)**:
   - When emitting nested structs classified as `POJO_STRUCT`:
     - Emits `public static class <Name> { ... }`.
     - Emits public mutable fields: `public <Type> <name> = <default_value>;`.
     - Translates C++ default expressions:
       - `"{ 0.5f , 0.5f }"` with `apigen:java_type:float` $\rightarrow$ `0.5f`.
       - `"{ 0 , -1 , 0 }"` $\rightarrow$ `{0f, -1f, 0f}`.
       - `"QualityLevel :: LOW"` $\rightarrow$ `QualityLevel.LOW`.
       - `"nullptr"` $\rightarrow$ `null`.
       - `"INFINITY"` $\rightarrow$ `Float.POSITIVE_INFINITY`.
     - Recursively unrolls child fields when `apigen:flatten` is present on a nested struct field (`ssctLightConeRad`).
     - Emits `@NonNull`, `@Nullable`, and `@Size(min = N)` annotations matching legacy `View.java`.
2. **Symmetrical Option Property Caching**:
   - Detects symmetrical setter/getter pairs taking and returning `const <Option>Options&`.
   - Synthesizes `private <Option>Options m<Option>Options;`.
   - Emits zero-cost, lazy-initializing pure Java getter:
     ```java
     @NonNull
     public <Option>Options get<Option>Options() {
         if (m<Option>Options == null) {
             m<Option>Options = new <Option>Options();
         }
         return m<Option>Options;
     }
     ```
   - In setter: stores `m<Option>Options = options;` and calls JNI `nSet<Option>Options(getNativeObject(), ...)` passing unrolled scalar fields.
3. **JNI C++ Struct Reconstruction in [javagen/jni_emitter.py](javagen/jni_emitter.py)**:
   - In `nSet<Option>Options`:
     - Receives unrolled scalar primitives.
     - Reconstructs the C++ options struct on the stack (`View::<Option>Options optionsCpp;`).
     - For scalar-to-vector broadcasts (`minScale`, `maxScale`), constructs `math::float2{ minScale }`.
     - Reconstructs nested sub-structs (`optionsCpp.ssct`, `optionsCpp.gtao`).
     - Invokes `view->set<Option>Options(optionsCpp);`.
   - Omits native JNI getters.

### Phase 3: `View` Production Generation & Parity Validation [COMPLETED]

1. Added `("View", REPO_ROOT / "filament/include/filament/View.h")` to `TARGETS` in [generate_android.py](generate_android.py).
2. Generated `View.java` and `View.cpp`.
3. Validated parity and zero breaking changes against legacy `View.java`:
   - All 14 option structs, public mutable fields, and exact default values.
   - All nested enums preserved.
   - Deprecated legacy stubs and enums retained: `ToneMapping`, `TargetBufferFlags`, `setToneMapping`, `getToneMapping`, `setSampleCount`, `getSampleCount`, `setAmbientOcclusion`, `getAmbientOcclusion`, `getUserTime`, `resetUserTime`.
   - Asynchronous `pick` method, `OnPickCallback`, and `InternalOnPickCallback` generated and mapped via JNI reflection.
4. **Zero-Diff Invariant on All Existing 40 Targets**:
   - Regenerating all 41 targets via `python3 tools/apigen/generate_android.py` verified to produce zero diff across the entire `android/filament-android/` directory outside `View` (except trivial clang diagnostic warning suppression in `Renderer.cpp`).
5. **Compilation & Multi-Platform Verification**:
   - Desktop Debug: `./build.sh -ip desktop debug` compiled cleanly without warnings or errors.
   - Desktop Unit Tests: `./out/cmake-debug/filament/test/test_filament` and `./out/cmake-debug/libs/utils/test_utils` passed 100%.
   - Android Release Build: `./build.sh -q arm64-v8a -Pip android release` compiled cleanly.
6. **On-Device Sample Verification**:
   - Installed `android/samples` on connected Android device.
   - Verified via `adb logcat` that no JNI exceptions, runtime crashes, or rendering warnings occurred.
   - Verified rendering visually via device screenshot capture.

### Phase 4: Decouple `beamsplitter` [COMPLETED]

1. Removed `emitters.EditJava(definitions, "View", javafolder)` from [tools/beamsplitter/main.go](tools/beamsplitter/main.go).
2. Deleted `tools/beamsplitter/emitters/java.go` and `tools/beamsplitter/emitters/java.template`.
3. Removed the `// The remainder of this file is generated by beamsplitter` marker from `View.java`.
4. Verified `go run .` inside `tools/beamsplitter` runs cleanly and no longer touches `View.java`.
5. Updated documentation in [tools/beamsplitter/README.md](tools/beamsplitter/README.md) and [skills/bindings_synchronization/SKILL.md](../../skills/bindings_synchronization/SKILL.md).
6. Created [hardcoding_audit.md](hardcoding_audit.md) cataloging all generator special-case logic introduced during the migration.
