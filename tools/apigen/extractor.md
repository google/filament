# APIGen Extractor (`extractor.py`)

## 1. Overview & Architectural Role

`extractor.py` is the front-end AST extractor of the Filament API binding generation toolchain. It parses C++ headers using `libclang` (via the LLVM Python bindings) and produces a language-agnostic **JSON Intermediate Representation (IR)** conformant to [`ir_def.ts`](ir_def.ts).

### Core Design Principles
* **Source-Fidelity over ABI Explosion**: Generic constructs (e.g. SFINAE constraints, templates) are captured semantically in a compressed format rather than being prematurely expanded at the extraction phase.
* **Dual Type Naming**: Captures both source-level sugar (`cpp_name`) and canonical desugared form (`qualified_name`) to enable precise generator-side type matching while retaining human-readable error messages.
* **Structured Documentation**: Normalizes Doxygen comments into CommonMark data structures rather than passing raw strings.
* **Strict Determinism**: Produces sorted, predictable JSON outputs for robust testing and caching.

---

## 2. Type Extraction & Desugaring Pipeline

The type extraction engine is centered around `get_type_info()` and `get_fully_qualified_name()`:

```
                  +---------------------------+
                  |    libclang Type / Cursor  |
                  +-------------+-------------+
                                |
             +------------------+------------------+
             |                                     |
    [Source Representation]             [Canonical FQN & Desugaring]
      t.spelling -> cpp_name               get_fully_qualified_name(t)
             |                                     |
             |                         +-----------v-----------+
             |                         | Is pointer / ref / []? |
             |                         |   -> Recurse pointee  |
             |                         +-----------+-----------+
             |                                     |
             |                         +-----------v-----------+
             |                         | Typedef / Alias Decl? |
             |                         +-----------+-----------+
             |                                    / \
             |                      In SIZED_TYPES   Not in SIZED_TYPES
             |                            |                 |
             |                     Return Sized Name  Recurse Underlying
             |                     (e.g. uint32_t)    (e.g. utils::Entity)
             |                            \                 /
             +-----------------+                   |
                               |                   |
                     +---------v-------------------v---------+
                     |         Construct TypeInfo Dict        |
                     |  - cpp_name / qualified_name          |
                     |  - category / is_const / is_pointer   |
                     |  - nullability (nonnull/nullable)     |
                     +---------------------------------------+
```

### Sized Types Preservation
Standard fixed-width integer typedefs (`int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `size_t`, `ssize_t`, `intptr_t`, `uintptr_t`, `ptrdiff_t`) are explicitly protected from decaying into compiler-dependent primitives (e.g. `signed char` or `unsigned long long`). When a typedef matches `SIZED_TYPES`, its canonical name is retained as the fixed-width identifier.

### Recursive Typedef Desugaring
Non-sized aliases (e.g. `using Thing = utils::Entity;` or `using Instance = utils::EntityInstance<Foo>;`) are recursively resolved to their underlying canonical tag types. This guarantees that back-end generators can link against known entity/object patterns regardless of local alias sugar.

### Nullability Annotations
Pointers, references, and return types inspect both Clang attributes and token sequences for Filament nullability macros:
* `UTILS_NONNULL` / `_Nonnull` $\rightarrow$ `nullability: "nonnull"`
* `UTILS_NULLABLE` / `_Nullable` $\rightarrow$ `nullability: "nullable"`
* Both parameter definitions (`arg["type"]["nullability"]`) and function/method return values (`func["return_type"]["nullability"]`) are annotated, allowing downstream generators to enforce strict `@NonNull` / `@Nullable` contracts and handle nullable return types (e.g. returning `nullptr` / `null` for missing optional parameters).

### Clang Annotations (`[[clang::annotate]]`)
* `UTILS_NOAPIGEN` / `[[clang::annotate("filament:apigen:skip")]]`: Extracted from `ANNOTATE_ATTR` cursors on classes, structs, methods, fields, constants, and enums into `attributes: ["filament:apigen:skip"]`. Downstream generators completely exclude annotated symbols from language binding generation.
* `UTILS_APIGEN_RETAINED` / `[[clang::annotate("filament:apigen:retained")]]`: Extracted from `ANNOTATE_ATTR` cursors on methods. Informs downstream generators that the returned object or passed parameter is retained on the Java wrapper class across its lifetime.
* `UTILS_APIGEN_FLAGS` / `[[clang::annotate("filament:apigen:flags")]]`: Extracted from `ANNOTATE_ATTR` cursors on enums into `attributes: ["filament:apigen:flags"]`, marking `enum_obj["is_flags"] = True`.
* `UTILS_APIGEN_ALTERNATE_NAME(<name>)` / `[[clang::annotate("filament:apigen:alternate_name:<name>")]]`: Extracted from `ANNOTATE_ATTR` cursors on methods into `attributes: ["filament:apigen:alternate_name:<name>"]`. Downstream generators use this alternate identifier for public API methods and native bridge symbols to resolve target-language collisions (e.g. reserved keywords or overloaded methods like `setBonesAsQuaternions` / `setBonesAsMatrices`).
* `UTILS_APIGEN_TAGGED_ARRAY` / `[[clang::annotate("filament:apigen:tagged_array")]]`: Extracted from `ANNOTATE_ATTR` cursors on template array parameters. Downstream generators collapse multiple template specializations into unified `<Family>Element` enum-tagged array methods.

### Bitfield Auto-Deduction & Sizing (`archetype: "bitfield"`)
Classes and structs are automatically analyzed for bitfield semantics without requiring manual annotations:
* **Direct bitfield struct**: All non-static member fields are bitfield declarations (`is_bitfield == True`).
* **Wrapper class**: The class contains a single non-static data member whose underlying type is a struct comprised solely of bitfields (e.g. `TextureSampler` encapsulating `backend::SamplerParams`).
* **Dynamic Primitive Sizing**:
  * `sizeof <= 2` bytes (≤ 16 bits): `bitfield_primitive = "short"`, JNI `jshort`.
  * `sizeof <= 4` bytes (17..32 bits): `bitfield_primitive = "int"`, JNI `jint`.
  * `sizeof <= 8` bytes (33..64 bits): `bitfield_primitive = "long"`, JNI `jlong`.
  * `sizeof > 8` bytes (> 64 bits): Cannot be backed by a primitive scalar integer in Java; bitfield deduction is skipped and generation proceeds *as-if* it were a standard class or aggregate struct.

### POJO Options Struct Detection (`archetype: "pojo_struct"`)
Options structs (such as `DynamicResolutionOptions`, `AmbientOcclusionOptions`, `BloomOptions`, etc. declared in `filament/Options.h`) feature public mutable fields, flattened sub-structs, and symmetrical property caching on consumer classes like `View`:
* **Origin Header Detection**: Any struct declared in `filament/Options.h` (or matching `*Options.h`), or nested within an options struct (e.g. `Ssct`, `Gtao`), is automatically marked `cls["is_pojo_struct"] = True` and assigned `cls["archetype"] = "pojo_struct"`.
* **Nested Result POD Structs**: Result records declared inside consumers (e.g. `View::PickingQueryResult`) are also classified as `pojo_struct` to preserve public mutable field access in target languages.
* This automated deduction completely eliminates hardcoded struct name whitelists (`POJO_STRUCT_NAMES`), allowing new options structs to be added to `Options.h` and instantly bound across language targets without manual generator configuration.

### Aggregate Struct Detection (`is_aggregate`)
In `handle_class`, classes and structs are analyzed for aggregate value semantics:
* Criteria:
  1. Non-polymorphic (`not cursor.type.is_polymorphic()`).
  2. No base classes (`len(bases) == 0`).
  3. Non-empty field set (`len(cls["fields"]) > 0`).
  4. All member fields are `public` (`AccessSpecifier.PUBLIC`).
* Outputs:
  - `cls["is_aggregate"] = true`
  - `cls["type"]["category"] = "struct"`
* Nested Struct Hierarchies:
  - Nested structs declared inside classes or outer structs (e.g. `LightManager::ShadowOptions`, `ShadowOptions::Vsm`) capture `parent_class: "<Parent>"`, `is_nested: true`, and inherit aggregate classifications when their fields meet the aggregate criteria.
* Fixed-Size Array Field Extraction:
  - Fixed-size array fields (e.g. `float cascadeSplitPositions[3]`) are extracted with their full array type spelling `float[3]` in `field["type"]["cpp_name"]`.

This automatic classification differentiates pure value types (e.g. `Box`, `Aabb`, `ShadowOptions`, `Vsm`) from classes encapsulating private state (e.g. `Frustum`), eliminating manual type whitelists.

> **Note on Value Classes & Archetypes**: Types with private/encapsulated state (such as `Frustum` storing `private: math::float4 mPlanes[6];` or `TextureSampler` storing `SamplerParams`) do not qualify as pure public aggregates and are marked `"is_aggregate": false`. In APIGen, they are classified under the generic Class Archetype system (`INLINE_BUFFER` and `BITFIELD`), which dynamically handles constructor extraction, storage fields, and zero-allocation JNI bridging without class-name hardcoding.

### Utility Class Detection (`is_utility`)
In `handle_class`, classes providing pure namespaces of static functions (such as `Colors` and `Exposure`) are automatically classified:
* Criteria:
  1. Non-polymorphic (`not cursor.type.is_polymorphic()`).
  2. No base classes (`len(bases) == 0`).
  3. No member fields (`len(cls["fields"]) == 0`).
  4. Non-empty method set (`len(cls["methods"]) > 0`).
  5. All public methods are static (`all(m.get("is_static", False) for m in cls["methods"])`).
* Outputs:
  - `cls["is_utility"] = true`
  - `cls["category"] = "utility"`

### Builder Class Detection (`is_builder`)
In `handle_class`, nested or base-derived builder classes (such as `IndirectLight::Builder`, `Skybox::Builder`, `LightManager::Builder`) are automatically classified without requiring annotations or hardcoded class name lists:
* Criteria:
  1. Class is named `Builder` OR derives from `BuilderBase<T>`.
* Outputs:
  - `cls["is_builder"] = true`
  - `cls["archetype"] = "builder"`
  - `cls["category"] = "builder"`
  - `cls["parent_class"] = "<ParentName>"` (if nested via `cursor.semantic_parent`)
  - `cls["is_nested"] = true` (if nested)
* Constructor & Method Filtering:
  - Copy and move constructors (`Builder(const Builder&)`, `Builder(Builder&&)`) and copy/move assignment operators (`operator=`) are automatically filtered from the IR.

### Template Trait Discovery & SFINAE Specializations
In `extractor.py`, methods constrained by SFINAE traits (`template <typename T, typename = is_supported_parameter_t<T>>`) are generically discovered and tagged for generator expansion:
* **Pre-Scan Header Trait Discovery**:
  - Traverses tokens across all included non-system headers to discover SFINAE type traits (`template <typename T> using Name = std::enable_if_t<...>;` and `using Name = std::enable_if_t<...>;`).
  - Automatically parses disjunctions (`std::is_same_v<Type, T> || ...`) and stores concrete allowed types in `self.traits`.
* **Cross-Class & Aliased Trait Resolution**:
  - Resolves type aliases referencing traits across classes or namespaces (e.g. `using alias_t = OtherClass::source_trait_t<T>;` or `using alias_t = source_trait_t<T>;`).
* **Template Parameter Tagging**:
  - Arguments referencing template type parameters are tagged with `"is_template_param": true` and `"template_param_name": "T"`.
  - Methods returning template type parameters (e.g. `T getValue(const char* name) const;`) tag `return_type` with `"is_template_param": true` and `"template_param_name": "T"`.
* **Specialization Cartesian Product**:
  - Emits the computed list of specializations in `method["specializations"] = [{"T": "float"}, {"T": "int32_t"}, ...]` in the JSON IR, enabling downstream generators to emit type-safe overloads and explicit C++ template calls.

---

## 3. Exception Specification Extraction (`is_noexcept`)

Each method cursor is inspected via `cursor.exception_specification_kind`:
* `BASIC_NOEXCEPT` (`noexcept`, `noexcept(true)`) and `DYNAMIC_NONE` (`throw()`) $\rightarrow$ `is_noexcept: true`
* `COMPUTED_NOEXCEPT`: Token scanned to differentiate `noexcept(false)` (`false`) from `noexcept(true)` (`true`)
* `NONE` / unannotated $\rightarrow$ `is_noexcept: false`

This flag instructs the back-end code generator whether a JNI call must be enclosed in `wrapJni()`.

---

## 4. Documentation Normalization

Doxygen comments (`/** ... */`) are parsed and structured into:
* `brief`: The leading sentence / summary.
* `details`: Full body description normalized into CommonMark (converting Doxygen commands like `\c`, `\p` / `@p`, `\b`, `\li` into Markdown).
* `params`: Key-value map of parameter names to descriptions.
* `returns`: Return value description.
* `meta`: Extracted metadata tags, including `see` (supporting `@see`, `\see`, `@sa`, `\sa` with comma-separated targets and scope resolution) and block tags (`@warning`, `\warning`, `@note`, `\note`, `@deprecated`, `\deprecated`, `@since`, `\since`, `@todo`, `\todo`).

### Enum Type Aliases
When a type alias (`TYPEDEF_DECL` or `TYPE_ALIAS_DECL`) references an `ENUM_DECL` (e.g. `using FenceStatus = backend::FenceStatus;`), `handle_alias` traverses the underlying enum's declaration and registers its `ENUM_CONSTANT_DECL` children, values, and doc comments directly into `cls["enums"]`. This allows downstream generators to emit complete Java `public enum` definitions for aliased backend enums.

### Struct Type Aliases & Options Struct Promotion
When a type alias (`TYPEDEF_DECL` or `TYPE_ALIAS_DECL`) inside a class references a `STRUCT_DECL` defined outside the class (e.g. `using DynamicResolutionOptions = filament::DynamicResolutionOptions;` in `View.h` referencing structs in `Options.h`), `handle_alias` resolves the underlying struct declaration cursor. It recursively parses the struct's member fields, methods, default value tokens, comment directives, and Doxygen documentation, and registers the struct into `cls["nested_records"]` with `parent_class = cls["name"]` and `is_nested = True`. This enables downstream generators to seamlessly nest external options structs as inner classes within their consumers (`View.DynamicResolutionOptions`).

### Comment Directives (`%codegen_*`) & Field Attributes
Comment directives in struct fields (from headers like `Options.h`) are extracted into standardized `attributes`:
* `%codegen_java_float%` $\rightarrow$ `attributes: ["apigen:java_type:float"]`: Informs downstream generators that vector fields (e.g. `math::float2 minScale = {0.5f, 0.5f}`) should be exposed as scalar `float` in Java.
* `%codegen_java_flatten%` $\rightarrow$ `attributes: ["apigen:flatten"]`: Informs downstream generators that nested sub-structs (e.g. `Ssct ssct;`, `Gtao gtao;`) should have their fields flattened directly into the enclosing class with the parent field name as a prefix (`ssctLightConeRad`).
* `%codegen_skip_javascript%` $\rightarrow$ `attributes: ["apigen:skip_javascript"]`.
* **Field Default Values**: Raw token sequences on member variable initializers (`math::float2 minScale = {0.5f, 0.5f}`, `float quality = 0.5f`, `QualityLevel quality = QualityLevel::LOW`) are extracted and stored into `field["default_value"]`.

### Static Class Constants (`VAR_DECL`)
Public `VAR_DECL` members inside classes (e.g. `static constexpr uint64_t FENCE_WAIT_FOR_EVER = backend::FENCE_WAIT_FOR_EVER;`) are extracted into `cls["constants"]`. If the RHS references another `VAR_DECL` across headers, the extractor resolves `child.referenced` to capture the underlying canonical value expression (e.g. `uint64_t(-1)`).

### Type Aliases (`TYPEDEF_DECL` / `TYPE_ALIAS_DECL`)
Public type aliases (such as `using LinearColor = math::float3;` or `using LinearColorA = math::float4;`) are extracted into `cls["aliases"]` with their underlying type info and doc comments, allowing code generators to synthesize dedicated Java `@interface` type annotations (e.g. `@LinearColor`).

---

## 5. Usage & Invocation

```bash
python3 tools/apigen/extractor.py \
  -I . \
  -I filament/include \
  -I libs/utils/include \
  -I libs/math/include \
  path/to/Header.h > Output.json
```

---

## 6. Feature Working Log & Changelog

### 2026-09-09
* **Struct Type Alias Promotion & Directive Extraction**:
  * Added `handle_alias` struct resolution: when a `TYPEDEF_DECL` or `TYPE_ALIAS_DECL` references a `STRUCT_DECL` (e.g. `using DynamicResolutionOptions = filament::DynamicResolutionOptions;` in `View.h`), resolves and extracts the full struct definition from `Options.h`, registering it into `cls["nested_records"]` with `is_nested = True`.
  * Added field comment directive parsing: `%codegen_java_float%` $\rightarrow$ `apigen:java_type:float`, `%codegen_java_flatten%` $\rightarrow$ `apigen:flatten`, `%codegen_skip_javascript%` $\rightarrow$ `apigen:skip_javascript`.
  * Added field default value token extraction, preserving raw expressions (`math::float2 minScale = {0.5f, 0.5f}`, `float quality = 0.5f`, `QualityLevel::LOW`) into `field["default_value"]`.
  * Added source line text caching for reliable directive and comment extraction across headers.

### 2026-09-08
* **Return Type Nullability Extraction**: Added token inspection before method name / opening parenthesis to extract `UTILS_NULLABLE` / `_Nullable` and `UTILS_NONNULL` / `_Nonnull` on function and method return types (`func["return_type"]["nullability"]`), enabling downstream generators to handle nullable pointers and C-strings (e.g. `Material::getParameterTransformName`).

### 2026-09-07
* **Retained Objects Attribute Extraction**: Extracted `UTILS_APIGEN_RETAINED` (`filament:apigen:retained`) into symbol attributes to support data-driven handle retention in downstream generators.

### 2026-09-04
* **Generic SFINAE Trait Resolution & Template Specializations**: Added header pre-scanning and AST token parsing for `std::enable_if_t` disjunction traits (`is_same_v<T, ...>`), cross-class trait alias forwarding (`using Alias = Source<T>`), template parameter tagging on method arguments and return types (`is_template_param`, `template_param_name`), and Cartesian product `specializations` generation in the JSON IR.
* **Universal Doxygen Tag Normalization (`\see`, `@see`, `\sa`, `@sa`, `\tag`)**: Expanded Doxygen pre-pass parsing to support both `@` and `\` prefixes across `@see`, `\see`, `@sa`, `\sa`, and block tags (`\warning`, `\note`, `\deprecated`, `\todo`, `\since`), as well as `@p` and `\p` parameter references. Previously, `\see` lines were silently omitted from extracted `meta["see"]` and discarded by downstream generators.

### 2026-09-03
* **Referenced Classes & Lazy Header Discovery**: Implemented automated discovery and resolution for external and forward-declared types (`Viewport`, `Box`, `PixelBufferDescriptor`) referenced across method parameters, return types, fields, and bases. Definitions are resolved either from existing TU declarations or lazily parsed from header files discovered across include search paths into `output["referenced_classes"]`.
* **Zero-Coupling External Aggregate Inference**: Enables downstream generators (`javagen.py`) to unroll struct parameters (e.g. `Renderer::copyFrame` unrolling `Viewport` coordinates) without hardcoded schemas or C++ forward-declaration annotations.

### 2026-09-02
* **Nested Aggregate Struct Extraction**: Extracted multi-level nested structs within classes and enclosing records (`parent_class`, `is_nested: true`), automatically classifying nested data-only records as `is_aggregate: true`.
* **Fixed-Size Array Field Extraction**: Extracted fixed-size C array record fields (`float[3]`, etc.) with explicit array type spellings in `field["type"]["cpp_name"]`.
* **Class Archetype Model Alignment**: Documented AST alignment for downstream archetypes (`INLINE_BUFFER`, `BITFIELD`, `AGGREGATE`, `UTILITY`, `HANDLE`).
* **Utility Class Classification**: Added automated AST classification for utility classes with all-static methods and no fields (`is_utility: true`, `category: "utility"`), used for `Colors` and `Exposure`.
* **Type Alias Extraction for Annotations**: Extracted class-level type aliases (`cls["aliases"]`) to support downstream `@LinearColor` annotation synthesis.

### 2026-09-01
* **Generic Constant Extraction**: Added AST traversal for `CursorKind.VAR_DECL` in classes, extracting `static constexpr` constants, documentation, and following referenced AST declarations (`child.referenced`) to resolve cross-header expressions into `cls["constants"]`.
* **Enum Alias Extraction**: Enhanced `handle_alias` to inspect `underlying_typedef_type.get_declaration()` for `ENUM_DECL`, automatically promoting aliased backend enums into fully populated class `enums`.

### 2026-08-31
* **Repository Include Auto-Discovery**: Automatically discovers and passes Filament include paths (`filament/include`, `filament/backend/include`, `libs/utils/include`, `libs/math/include`, `libs/filabridge/include`, `libs/filaflat/include`) when parsing files within the Filament repository.
* **Enhanced String, Chrono, and Primitive Category Detection**: Categorizes `std::string_view`, `utils::CString`, `utils::StaticString`, `utils::ImmutableCString` as `"string"`, `std::chrono` types as `"chrono"`, `utils::tribool` as `"tribool"`, and `std::optional` as `"optional"`.
* **AST Aggregate Struct Heuristic**: Implemented automated AST-level aggregate struct classification in `handle_class`, identifying non-polymorphic records with all-public fields as `"is_aggregate": true` and `"category": "struct"` (e.g. `Box`, `Aabb`).
* **Nested Class Enum Extraction**: Added AST traversal for `CursorKind.ENUM_DECL` inside classes and structs in `handle_class`, extracting nested enums with their entries, integer values, and Doxygen comments into `cls["enums"]`.
* **Static Method Extraction**: Extracted `is_static` flag on method definitions using `cursor.is_static_method()`.
* **Parameter Annotation Extraction**: Extraction of `[[clang::annotate]]` attributes into `attributes: [...]` on function and method parameters (such as `size_param:<count>`).
* **C++ Exception Specification Extraction**: Added extraction of `is_noexcept` for methods and functions via `cursor.exception_specification_kind` and token validation.
* **FQN & Typedef Desugaring**: Implemented recursive typedef desugaring in `get_fully_qualified_name()` with the `SIZED_TYPES` exception whitelist.
* **Template Argument FQN**: Added recursive fully-qualified template argument formatting for template types (e.g. `filament::math::details::TVec3<float>`).
* **Nullability Extraction**: Added token inspection for `UTILS_NONNULL` and `UTILS_NULLABLE` macros on parameter declarations.

### Earlier
* **Initial Extraction Engine**: libclang visitor for classes, structs, enums, fields, and member methods.
* **Doxygen Normalization**: Parser converting raw C++ docstrings into structured CommonMark IR.
* **SFINAE / Trait Constraint Detection**: Scans AST and tokens for `std::enable_if_t` and `is_supported_parameter_t` patterns.
