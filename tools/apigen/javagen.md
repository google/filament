# APIGen Generator (`javagen`)

## 1. Overview & Architectural Role

The APIGen generator is the Java & JNI C++ code generation subsystem of the Filament API binding generation toolchain. It consumes the JSON Intermediate Representation (IR) produced by `extractor.py` and emits:
1. **Idiomatic Java Wrapper Classes** (`.java`) targeting Android / Desktop JVM.
2. **Zero-Cost JNI C++ Bridges** (`.cpp`) establishing high-performance, type-safe JNI bindings.

### Modular Package Architecture (`tools/apigen/javagen/`)

The generator is organized as a modular Python package under `tools/apigen/javagen/` with a backward-compatible CLI facade script at `tools/apigen/javagen.py`:

```
tools/apigen/
├── javagen.py                      # Standalone CLI entrypoint & backward-compatible facade
└── javagen/                        # Modular package
    ├── __init__.py                 # Public package interface re-exporting core classes & tables
    ├── errors.py                   # Custom typed exception hierarchy (JavaGenError, TypeResolutionError, etc.)
    ├── config.py                   # Type mapping tables, math/value registries, keywords, constants
    ├── utils.py                    # AST attribute extractors, sanitizers, naming utilities
    ├── doc.py                      # CommonMark-to-Javadoc translation and tag formatting
    ├── type_resolver.py            # TypeResolver engine (type resolution, layouts, buffer classes)
    ├── context.py                  # ClassContext data model, IR parsing, SFINAE template expansion
    ├── java_emitter.py             # JavaEmitter (Java wrapper classes, builders, aggregate structs)
    ├── jni_emitter.py              # JniEmitter (zero-cost JNI C++ bridges, buffer & slice unpackers)
    └── orchestrator.py             # process_file multi-file IR pipeline and CLI argument parser
```

---

## 2. Type Mapping & Resolution Subsystem

Type resolution is driven by `resolve_type_info()` which matches against `TYPE_MAP`, `MATH_TYPES`, and `TYPE_PATTERNS`:

```
                 +---------------------------------+
                 |       Input IR Type Object      |
                 |  (qualified_name / cpp_name)    |
                 +----------------+----------------+
                                  |
                +-----------------+-----------------+
                |                                   |
       [Exact / FQN Match]                 [Pattern / Substring]
     TYPE_MAP / MATH_TYPES                    TYPE_PATTERNS
                |                                   |
                +-----------------+-----------------+
                                  |
               +------------------v------------------+
               |          Resolved Type Info         |
               |  - java type (e.g. float[], int)    |
               |  - jni type (e.g. jfloatArray)      |
               |  - annotation (e.g. @Size, @NonNull)|
               |  - marshaling expressions           |
               +-------------------------------------+
```

### Type Categories Handled
* **Fixed-Width & Sized Integers**:
  * All sub-32-bit scalar integer types (`int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `short`, `unsigned short`, `char`, `unsigned char`, `signed char`) $\rightarrow$ Java `int` / JNI `jint` (annotated with `@IntRange(from = 0)` when unsigned, marshaled as `(type)val` in JNI C++).
  * `uint32_t` / `int32_t` $\rightarrow$ `int` (annotated with `@IntRange(from = 0)` when unsigned).
  * `uint64_t` / `int64_t` $\rightarrow$ `long` (annotated with `@IntRange(from = 0)` when unsigned).
  * `size_t` / `ssize_t` $\rightarrow$ `int` / `jint` (annotated with `@IntRange(from = 0)` for non-negative sizes and collection counts).
* **Fixed-Size Struct Array Fields** (`T[N]`):
  * C-style array fields (e.g. `float cascadeSplitPositions[3]`) map to `@NonNull @Size(min = N) float[]` in Java.
  * Decomposed into indexed primitive fields (`mCascadeSplitPositions0`, `mCascadeSplitPositions1`, etc.) in aggregate structs.
  * Cross-method marshalling unrolls indexed scalar leaves to JNI (`var0`, `var1`, ...) and assigns elements individually (`that.field[i] = ...`) in C++.
* **Math Vector, Matrix & Quaternion Types** (`MATH_TYPES`):
  * Vectors (`float2`..`float4`, `double2`..`double4`) and matrices (`mat2f`..`mat4f`, `mat2`..`mat4`).
  * Quaternions (`math::quatf`, `math::quat`, `filament::math::details::TQuaternion<float>`) mapping to `@NonNull @Size(min = 4) float[]` with `X, Y, Z, W` component expansion.
  * Supported in both short form (`math::float3`) and canonical FQN form (`filament::math::details::TVec3<float>`).
* **Enums & Dynamic Zero-Allocation Caching (`EnumCache`)**:
  * C++ `enum class Foo : int` $\rightarrow$ nested Java `public enum Foo { ... }`.
  * **Standard (0-indexed sequential) Enums**:
    * Emits `public int toFilamentNative() { return ordinal(); }` in every enum to decouple Java ordinal from C++ native values.
    * Uses `EnumCache.sFooValues[idx]` for zero-allocation $O(1)$ reverse lookup on queries.
  * **Custom / Negative / Explicitly-Valued Enums** (e.g. `FrameStatus` with `-2, -1, 0`, `PacingStatus` with `0, -1, 1`, `AttachmentPoint` with aliases `COLOR0 = 0`, `COLOR = 0`):
    * Detected automatically via `is_custom_enum()`.
    * Generates `private final int mValue;` constructor and `public int toFilamentNative() { return mValue; }`.
    * Synthesizes a high-performance, switch-based `public static Foo from(int value)` lookup table with automatic deduplication for value aliases (avoiding duplicate case labels).
    * Bypasses array indexing in `EnumCache` to prevent `ArrayIndexOutOfBoundsException` on negative native indices.
  * Method parameters accept `@NonNull Foo foo` and forward `foo.toFilamentNative()` to JNI as `jint`.
  * JNI bridge casts `jint` to `(Class::Foo)foo` when invoking C++.
  * Any class whose methods return a standard enum dynamically synthesizes a private static inner cache:
    ```java
    static final class EnumCache {
        static final Foo[] sFooValues = Foo.values();
    }
    ```
  * Native getters return `EnumCache.sFooValues[nMethod(...)]`, completely eliminating hidden Dalvik/ART array clone allocations during high-frequency queries.
* **Struct Slices & Flat Array Packing (`utils::Slice<const StructT>`)**:
  * Extracted and resolved via `is_struct_slice`.
  * High-level Java API represents the slice as `@Nullable StructT[]`.
  * Flattened JNI calls pack the structs into a flat `long[]` primitive array buffer and pass `(long[] rawBuffer, int count)`.
  * JNI C++ zero-copy unpacks the buffer via `reinterpret_cast<StructT*>(elements)` and constructs `utils::Slice<const StructT>{ structs, count }`.
  * Memory safety is guaranteed via `env->ReleaseLongArrayElements(rawBuffer, elements, JNI_ABORT)` cleanup after native invocation.
* **Static Methods** (`is_static`):
  * Methods marked `is_static: true` in the IR emit `public static` Java methods calling `nMethod(...)` without `getNativeObject()`.
  * JNI methods omit `jlong native<Class>` parameters and invoke static C++ methods directly (`Class::method(...)`).
* **Utility Classes** (`is_utility`, e.g. `Colors`, `Exposure`):
  * Classes classified as utility classes generate a `private <Class>() {}` constructor to prevent instantiation.
  * Omit `mNativeObject`, `getNativeObject()`, `clearNativeObject()`, and `IllegalStateException`.
  * All public methods are generated as `public static`.
* **Color & LinearColor Type Annotations** (`@LinearColor`):
  * Emits `@Retention(SOURCE)` and `@Target({PARAMETER, METHOD, LOCAL_VARIABLE, FIELD}) public @interface LinearColor {}` when `LinearColor` alias or `Colors` class is encountered.
  * Methods accepting or returning linear color vectors (`float3`, `float4`, `float[]`) are annotated with `@LinearColor`.
* **Entity Types** (`TYPE_PATTERNS`):
  * `utils::Entity` $\rightarrow$ `int` annotated with `@Entity`, smuggled via `Entity::import` / `Entity::smuggle`.
  * `utils::EntityInstance<T>` $\rightarrow$ `int` annotated with `@EntityInstance`.
* **Class Archetypes**:
  * **Handle Archetype (`HANDLE`, e.g. `Camera`, `Scene`, `Fence`)**:
    * Java holds `private long mNativeObject;` initialized via package-private constructor `Camera(long nativeObject)`.
    * Methods validate non-zero handle through `getNativeObject()`.
    * Destruction hook clears handle via `clearNativeObject()`.
  * **Bitfield Value Object Archetype (`BITFIELD`, e.g. `TextureSampler`)**:
    * Encapsulates packed scalar bitfields backed by dynamic primitive storage (`short mSampler`, `int mSampler`, or `long mSampler` based on `sizeof <= 2`, `<= 4`, or `<= 8` bytes) with zero hidden heap allocations. If `sizeof > 8` bytes (> 64 bits), APIGen falls back to standard class/struct generation.
    * Constructors expand dynamically from Clang AST with Doxygen and telescope default arguments (`this(minMag, WrapMode.CLAMP_TO_EDGE);`).
    * Value mutators return updated packed bitfields (`mSampler = nSet...`).
    * Const getters query native bitfields and return cached enum instances (`EnumCache.s...Values[nGet...]`).
    * JNI bridges marshal bitfields via dynamically selected `filament::JniUtils::to_short`/`from_short`, `to_int`/`from_int`, or `to_long`/`from_long`.
  * **Inline Buffer Value Object Archetype (`INLINE_BUFFER`, e.g. `Frustum`)**:
    * Holds an inline primitive array backing buffer (`/* package */ final float[] mPlanes = new float[24];`).
    * Does not allocate or hold `mNativeObject`.
    * Methods pass buffer reference to native declarations (`nIntersects(mPlanes, ...)`).
    * JNI methods receive `jfloatArray planes_` and re-interpret buffer memory directly (`reinterpret_cast<Frustum const *>(planes)`).
    * Buffer getters (e.g. `getNormalizedPlanes`) perform pure Java copies (`System.arraycopy(mPlanes, 0, out, 0, 24)`).
  * **Pure Aggregate Struct Archetype (`AGGREGATE`, e.g. `Viewport`, `Box`, `Aabb`)**:
    * Automatically detected from AST when non-polymorphic and all member fields are `public`, or registered via `DEFAULT_KNOWN_CLASSES` / sibling JSON IR files.
    * Generates as a zero-allocation flat Java class holding private primitive scalar fields (`private float mCenterX, mCenterY, mCenterZ, mHalfExtentX, mHalfExtentY, mHalfExtentZ;`) or public fields and getters (`Viewport.left, bottom, width, height`, `getLeft()`, etc.).
    * **Cross-Method Parameter Unrolling**:
      * When an aggregate struct is passed to a method (by value, reference, or const reference, such as `Renderer::copyFrame(SwapChain*, const Viewport&, const Viewport&, uint32_t)` or `Frustum::intersects(const Box&)`), `resolve_type_info` recognizes the struct via `is_aggregate_struct` and prevents misclassification as an opaque `is_filament_type` handle.
      * **Java Wrapper**: Instead of generating non-existent `dstViewport.getNativeObject()` calls, Java unrolls the struct's scalar fields into primitive arguments via getter chains (`dstViewport.getLeft(), dstViewport.getBottom(), dstViewport.getWidth(), dstViewport.getHeight()`, `box.getCenterX(), box.getCenterY(), ...`).
      * **Native Declaration**: Emits primitive scalar types (`int dstViewportLeft, int dstViewportBottom, int dstViewportWidth, int dstViewportHeight`, `float boxCenterX, ...`).
      * **JNI C++ Bridge**: Accepts the scalar primitives (`jint dstViewportLeft, ...`), reconstructs the C++ aggregate struct on the stack (`Viewport dstViewport; dstViewport.left = dstViewportLeft; ...` or `Box box; box.center = { boxCenterX, boxCenterY, boxCenterZ }; ...`), and passes the struct directly to the native C++ invocation (`that->copyFrame(dstSwapChain, dstViewport, srcViewport, (uint32_t)flags)`).
      * **Dynamic Header Inclusion**: Referenced struct headers (e.g. `<filament/Viewport.h>`, `<filament/Box.h>`) are automatically tracked and included in the generated `.cpp` file.
  * **Builder Archetype (`BUILDER`, e.g. `IndirectLight.Builder`, `Skybox.Builder`, `LightManager.Builder`)**:
    * Automatically extracted and nested within its enclosing parent class (`public static class Builder`).
    * Lifecycle management via `BuilderFinalizer` tracking `mNativeBuilder` and invoking `nDestroyBuilder(mNativeObject)` upon GC.
    * Constructors instantiate the native C++ builder via `nCreateBuilder(...)`.
    * Fluent chained setter methods return `@NonNull Builder` (`return this;`) and invoke corresponding `nBuilder<MethodName>` native functions.
    * Terminal `build(...)` methods:
      * Object builders: `build(@NonNull Engine engine)` calls `nBuilderBuild(mNativeBuilder, engine.getNativeObject())` and returns `new TargetClass(nativeHandle)`.
      * Component builders: `build(@NonNull Engine engine, @Entity int entity)` calls `nBuilderBuild(mNativeBuilder, engine.getNativeObject(), entity)` and validates component creation.
    * JNI C++ bridges emit `Java_<package>_<Parent>_nCreateBuilder`, `nDestroyBuilder`, `nBuilderBuild`, and `nBuilder<Method>` with automatic JNI signature mangling for overloaded setters.
  * **PixelBufferDescriptor**:
    * Annotated with `UTILS_NOAPIGEN` in `PixelBufferDescriptor.h` and skipped as a standalone generated class by APIGen.
    * Maintained as a clean handwritten class (`com.google.android.filament.PixelBufferDescriptor`) wrapping CPU-side `Buffer storage`, coordinates (`left`, `top`, `stride`), alignment, format (`Texture.Format`, `Texture.Type`, `Texture.CompressedFormat`), compressed size, and asynchronous completion callback references (`@Nullable Object handler`, `@Nullable Runnable callback`).
    * Completely decoupled from the native C++ `backend::BufferDescriptor` inheritance hierarchy: does not wrap a native heap pointer (`mNativeObject`), preventing GC finalizer overhead.
    * Delegates `computeDataSize(@NonNull Texture.Format format, @NonNull Texture.Type type, int stride, int height, @IntRange(from = 1, to = 8) int alignment)` directly to native C++ `backend::PixelBufferDescriptor::computeDataSize(...)` via JNI `nComputeDataSize`, ensuring formatting, stride, and alignment calculations remain synchronized.
    * Enables 100% backward compatibility with legacy code via an inner subclass in `Texture.java`:
      ```java
      @Deprecated
      public static class PixelBufferDescriptor extends com.google.android.filament.PixelBufferDescriptor {
          // forwards all constructors to super(...)
      }
      ```
    * **Method Unrolling & JNI Bridge (`backend::PixelBufferDescriptor&&`)**:
      * Methods accepting `backend::PixelBufferDescriptor&&` (such as `Renderer::readPixels` and `Texture::setImage`) are recognized by `javagen.py` via `is_pixel_buffer_descriptor_method()`.
      * **Java Generation**: Emits `@NonNull PixelBufferDescriptor buffer`. Read operations conditionally validate `if (buffer.storage.isReadOnly()) throw new ReadOnlyBufferException();`. Unrolls `storage`, `remaining`, `left`, `top`, `type`, `alignment`, `stride`, `format` (supporting compressed textures), `handler`, and `callback` into the private native method call. Throws `BufferOverflowException` if native call returns `< 0`.
      * **JNI C++ Bridge**: Emits JNI C++ function definitions including `<backend/PixelBufferDescriptor.h>`, `"common/CallbackUtils.h"`, and `"common/NioUtils.h"`. Resolves overloaded native method names (e.g. `nReadPixels`) via standard JNI signature mangling (`Ljava_nio_Buffer_2`, `Ljava_lang_Runnable_2`). In the body, dynamically computes required size via `PixelBufferDescriptor::computeDataSize` (or compressed size), validates capacity with `AutoBuffer nioBuffer(env, storage, 0)`, instantiates `JniBufferCallback::make`, constructs `backend::PixelBufferDescriptor`, and dispatches to `that->method(..., std::move(desc))` inside `wrapJni<jint>`.

### Template Specializations & SFINAE Trait Expansion
Methods containing SFINAE trait constraints in the JSON IR (`"specializations": [{"T": "float"}, ...]`) are expanded dynamically into type-safe overloads across both Java and JNI C++:
* **Method Unrolling**:
  - In `ClassContext._expand_method`, every method entry with `specializations` is unrolled into concrete method definitions per type variant.
  - Template parameter placeholders on arguments and return types (`"is_template_param": true`) are substituted with the concrete specialization type.
* **Math Vector Unrolling**:
  - Vector types (`math::float2`..`float4`, `math::int2`..`int4`, `math::bool2`..`bool4`) are unrolled into both exploded scalar overloads (e.g. `float valuex, float valuey, float valuez`) and convenience array overloads (`@NonNull @Size(min = 3) float[] value`).
* **Return-Type Template Name Unmangling**:
  - For methods where the return type is a template parameter (e.g. `T getConstant(const char* name)`), the generator unmangles the Java method name by appending the capitalized Java type suffix (e.g. `getConstantFloat`, `getConstantInt`, `getConstantBoolean`, `getConstantFloat3`), preventing ambiguous type-erased return signatures in Java.
* **Signature Deduplication**:
  - SFINAE trait constraints frequently admit C++ types that map to identical Java types (such as `int32_t` and `uint32_t`, which both map to `int`). The generator deduplicates generated signatures by `(method_name, java_param_types, java_return_type)` to eliminate duplicate method errors in Java and linker collisions in JNI C++.
* **Explicit JNI C++ Template Dispatch**:
  - Emits explicit template instantiation calls in JNI C++ (`that->method<spec_type>(...)` and `builder->method<spec_type>(...)`), ensuring zero ambiguity and zero-cost template invocation in native code.
* **Nested Builder Support**:
  - Template methods on nested Builder classes (e.g. `Builder::constant<T>(...)` or `Builder::parameter<T>(...)`) are expanded into the corresponding Builder Java method overloads and JNI bridge calls.

---

## 3. Java Wrapper Generation (`.java`)

### File Structure & Imports
Every generated `.java` file contains:
* Standard AOSP license header.
* Generator warning header: `// THIS FILE IS GENERATED BY APIGEN AND MUST NOT BE HAND-MODIFIED.`
* Package declaration (`package com.google.android.filament;`).
* `import java.lang.IllegalStateException;` (omitted for utility classes).
* Alphabetized, conditional `androidx.annotation.*` imports (`@IntRange`, `@NonNull`, `@Nullable`, `@Size`).
* Suffix warning comment at file end.

### Native Object Lifecycle & Handle Validation
* Internal handle: `private long mNativeObject;` (omitted for utility and value classes).
* Accessor: `public long getNativeObject()` validating non-zero handle (`if (mNativeObject == 0) throw new IllegalStateException("Calling method on destroyed " + name);`).
* Call sites: Every generated instance Java method delegates through `getNativeObject()` (e.g. `nMethod(getNativeObject(), ...)`) to enforce destroyed object checks before entering JNI.
* Destructor hook: `void clearNativeObject() { mNativeObject = 0; }`

### Math Methods
* **Return Values**:
  * Emits out-parameter signatures: `@NonNull @Size(min = N) public float[] getVector(@Nullable @Size(min = N) float[] out)`.
  * Performs in-place validation: `out = Asserts.assertFloatN(out);`.
  * Passes `out` to native call `nGetVector(getNativeObject(), ..., out)` and returns `out`.
* **Input Parameters**:
  * Unrolled vectors (`float3 v` $\rightarrow$ `float vx, float vy, float vz`).
  * Array inputs (`mat4 const*`, `float4 const*`) validated with `@NonNull @Size(min = N)`.

### Documentation Generation
Structured CommonMark details are converted into standard HTML Javadoc (`<p>`, `<pre>`, `<code>`, `{@link ...}`, `@param`, `@return`, `@see`).

### Retained Objects & Static Handle Factories (`UTILS_APIGEN_RETAINED`, `@RestrictTo wrap(...)`)
* **Retained Object References (`UTILS_APIGEN_RETAINED`)**:
  * Methods annotated with `UTILS_APIGEN_RETAINED(type)` (e.g. `Renderer.getEngine()`, `MaterialInstance.getMaterial()`, `MaterialInstance.getEngine()`, `SwapChain.getNativeWindow()`) represent parent or peer objects passed during construction and retained in Java fields.
  * Standardized constructor parameter order: `(long nativeObject, Object ref)`.
  * The Java emitter synthesizes private fields (`private final Engine mEngine;`) and generates zero-cost Java-side getter methods (`public Engine getEngine() { return mEngine; }`) that return the retained reference directly without crossing the JNI boundary or creating redundant wrapper instances.
  * Prunes redundant native JNI getter functions from both Java native declarations and C++ JNI bridge files.
* **Retained Builder Direct NIO Buffers (`has_retained_buffers`)**:
  * If a builder method accepts direct NIO buffers retained by native code across builder configuration, the JNI emitter generates a heap-allocated `BuilderWrapper` (`struct <Class>BuilderWrapper`) holding `std::vector<std::unique_ptr<AutoBuffer>> retainedBuffers;`.
  * In the builder methods, `wrapper->retainedBuffers.push_back(std::make_unique<AutoBuffer>(env, buffer, size))` keeps native direct buffer allocations alive until `build(...)` or builder destruction.
* **Static Handle Factories (`@RestrictTo(LIBRARY_GROUP) wrap(...)`)**:
  * All handle classes generate `@NonNull @RestrictTo(RestrictTo.Scope.LIBRARY_GROUP) public static <Class> wrap(...)` static factory methods.
  * Handle constructors are package-private across all classes, preventing raw pointer constructor pollution from autocomplete and external misuse.
  * For classes with retained references, `wrap(...)` requires the retained dependencies. If all retained references are nullable (e.g. `MaterialInstance`, `SwapChain`), a convenience single-argument `wrap(long nativeObject)` overload is also generated.

### Generic Asynchronous Callbacks
* When C++ methods accept asynchronous completion callbacks with an optional callback handler (e.g. `backend::CallbackHandler* handler, Callback callback, void* user` in `SwapChain::setFrameScheduledCallback`):
  * **SAM Interface Synthesis**: Generates a `@FunctionalInterface public interface <CallbackName> { void on<Event>(...); }` inner interface.
  * **Convenience Overloads**: Emits convenience overloads for trailing default parameters (e.g. defaulting `handler = null`, `user = null`).
  * **JNI Dispatch Trampoline**: Dispatches through a native JNI callback handler (`JniCallback::make` or `JniImageCallback::make`) that posts back to the provided Java handler or executor.

### Nullable Return Handling
* When native methods return pointers or strings (`const char*`, `utils::CString`) annotated with `UTILS_NULLABLE` (or return `nullptr`):
  * In JNI C++, null pointers explicitly return `nullptr` / `NULL`.
  * In Java, methods receive `null` instead of empty string `""` fallback, ensuring complete consistency with the native C++ contract (e.g. `Material.getParameterTransformName`).

### Final `Object` Method Collision Avoidance
* Methods defined in C++ with default parameters that can expand into 0-argument overloads (e.g. `wait(uint64_t duration = 0)`) check `JAVA_OBJECT_FINAL_NOARG_METHODS` (`wait`, `notify`, `notifyAll`, `getClass`).
* Zero-argument variants colliding with final `java.lang.Object` methods are automatically suppressed to prevent compilation failures.

### Java-Side POJO Struct & Option Caching (`_detect_cached_fields`)
* **Matching Setter/Getter Pairs**:
  * Classes exposing paired options methods (e.g. `setDynamicResolutionOptions` / `getDynamicResolutionOptions` in `View`, `setClearOptions` / `getClearOptions` in `Renderer`) automatically synthesize a private field (`private @Nullable DynamicResolutionOptions mDynamicResolutionOptions;`).
  * Setters update the cached Java field and dispatch unrolled scalars or parameters across JNI.
* **Generic Standalone Zero-Argument POJO Getters**:
  * Any zero-argument method returning a POJO struct (`is_pojo_struct`, e.g. `Engine::getConfig()`) is systematically recognized as a cached property even in the absence of a member setter.
  * Emits lazy default initialization:
    ```java
    @NonNull
    public Config getConfig() {
        if (mConfig == null) {
            mConfig = new Config();
        }
        return mConfig;
    }
    ```
  * Extracts C++ Doxygen comments into JavaDoc.
  * Discards redundant native JNI getter functions (`nGetConfig`) to avoid JNI serialization overhead and GC pressure.
* **POJO vs. Aggregate Distinction**:
  * POJO options/config structs (which represent client configuration) use Java-side instance caching.
  * Aggregate POD math structs (e.g. `Box`, `Viewport`, `Frustum`) which represent live geometric queries retain the `@Nullable out` parameter pattern with JNI dispatch.

### Generic Sub-Component Handle Reference Caching (`T& getFoo()`)
* **AST Reference Inference (`is_handle_reference`)**:
  * In C++, an object method returning a reference to a Filament handle (`T& getFoo() noexcept`, e.g. `TransformManager& getTransformManager()`) indicates that `T` is an internal sub-component whose lifetime is bound to the parent object.
  * `ClassContext._detect_cached_fields()` automatically identifies zero-argument getters returning a reference to a Filament handle (`is_filament_type: True`, `is_reference: True`, `not is_pointer`) without an existing setter, registering them into `self.cached_fields` with `"is_handle_reference": True`.
* **Lazy Java Instance Caching**:
  * Emits a private cached handle field: `private @Nullable <Type> m<Prop>;`.
  * Generates a lazy, null-safe getter that validates `getNativeObject()`, throws on a null native pointer, and caches the wrapper:
    ```java
    @NonNull
    public TransformManager getTransformManager() {
        if (mTransformManager == null) {
            long nativeTransformManager = nGetTransformManager(getNativeObject());
            if (nativeTransformManager == 0) throw new IllegalStateException("Couldn't get TransformManager");
            mTransformManager = new TransformManager(nativeTransformManager);
        }
        return mTransformManager;
    }
    ```
  * Declares `private static native long nGet<Prop>(long native<Class>);`.
* **Zero-Cost JNI Address-Of Translation**:
  * The generic JNI emitter processes `nGet<Prop>` naturally: calling `that->get<Prop>()` returns `T&`, and `type_resolver` converts it to `(jlong)&(that->get<Prop>())`.
* **Removal of Hardcoded Manager Interceptions**:
  * Eliminates class-specific checks (`if self.name == "Engine"`) and manual JNI snippets for `getTransformManager`, `getLightManager`, `getRenderableManager`, and `getEntityManager`.

### Generic Factory Methods, Parent Retention & Nullability Guards (`retains_parent_handle`)
* **Data-Driven Parent Retention (`retains_parent_handle`)**:
  * When a method returns a Filament handle type (such as `Engine.createRenderer()` returning `Renderer` or `Material.createInstance()` returning `MaterialInstance`), `JavaEmitter.retains_parent_handle(target_class)` inspects `KNOWN_CLASSES` for the target return type.
  * If the target class defines any method annotated with `filament:apigen:retained` (such as `Renderer.getEngine()` or `MaterialInstance.getMaterial()`), the target class retains its parent handle in Java and requires `(long nativeHandle, ParentClass parent)` in its package-private constructor.
  * The emitter automatically appends `, this` to the constructor invocation without needing hardcoded class or method name checks.
* **Uniform Nullability Enforcement**:
  * **Non-Null Return (`@NonNull` / `UTILS_NONNULL` / default)**:
    ```java
    long nativeRenderer = nCreateRenderer(getNativeObject());
    if (nativeRenderer == 0) throw new IllegalStateException("Couldn't create Renderer");
    return new Renderer(nativeRenderer, this);
    ```
    Guarantees that null native handles immediately throw a descriptive `IllegalStateException("Couldn't create <Type>")` rather than constructing invalid Java wrapper instances.
  * **Nullable Return (`@Nullable` / `UTILS_NULLABLE`)**:
    ```java
    long nativeMaterialInstance = nCreateInstance(getNativeObject(), name);
    return nativeMaterialInstance == 0 ? null : new MaterialInstance(nativeMaterialInstance, this);
    ```
    Returns clean Java `null` when native handle creation returns zero.
* **Elimination of Generator Interceptions**:
  * Removed `createRenderer` from `intercepted_engine_methods` in both `java_emitter.py` and `jni_emitter.py`.
  * Removed hardcoded `Material.createInstance` special case in `java_emitter.py`.
  * Unifies handle creation across `createView`, `createScene`, `createCamera`, `createFence`, `createRenderer`, and `createInstance` with full Doxygen JavaDoc translation and C++ `wrapJni` exception safety.

---

## 4. JNI C++ Bridge Generation (`.cpp`)

### Standard Headers & Namespaces
```cpp
// THIS FILE IS GENERATED BY APIGEN AND MUST NOT BE HAND-MODIFIED.

#include <jni.h>
#include <common/JniUtils.h>
#include <{source_header}>

using namespace filament;
using namespace filament::android;
using namespace utils;
```
* Conditional math namespace: `using namespace filament::math;` is only emitted when math types or headers are referenced by the class.

### Exception Safety & `wrapJni` Invariants

JNI functions branch on `is_noexcept`:

#### When `is_noexcept == true` (Direct Invocation)
* **Void**: `that->method(args...);`
* **Scalar Return**: `return ({jni_ret}) that->method(args...);`
* **Math Return**:
  ```cpp
  jfloat *out = env->GetFloatArrayElements(out_, nullptr);
  *reinterpret_cast<float3 *>(out) = that->method(args...);
  env->ReleaseFloatArrayElements(out_, out, 0);
  ```

#### When `is_noexcept == false` (`wrapJni` Guarded)
* **Void Method**:
  ```cpp
  wrapJni(env, [=]() {
      that->method(args...);
  });
  ```
* **Scalar / Handle Return**:
  ```cpp
  return wrapJni<{jni_ret}>(env, [=]() {
      return ({jni_ret}) that->method(args...);
  });
  ```
* **Math Return (Output Arrays)**:
  `Get*ArrayElements` and `Release*ArrayElements` bracket the `wrapJni` call. If an exception occurs, no dummy values are assigned and the array buffer is safely released without leaking references:
  ```cpp
  jfloat *out = env->GetFloatArrayElements(out_, nullptr);
  wrapJni(env, [=]() {
      *reinterpret_cast<float3 *>(out) = that->method(args...);
  });
  env->ReleaseFloatArrayElements(out_, out, 0);
  ```
* **Input Pointer / Array Arguments**:
  Array elements acquired via `Get*ArrayElements` are passed into `wrapJni` and unconditionally released in `body_post`:
  ```cpp
  jfloat const * const v = env->GetFloatArrayElements(v_, nullptr);
  wrapJni(env, [=]() {
      that->setVector(*reinterpret_cast<float3 const *>(v));
  });
  env->ReleaseFloatArrayElements(v_, const_cast<jfloat *>(v), 0);
  ```

### Slice and Contiguous Buffer Parameters (`utils::Slice`)

C++ API headers express bounded array parameters using `utils::Slice`:
```cpp
void addEntities(utils::Slice<const utils::Entity> entities);
```
Legacy pointer + size overloads are preserved as zero-overhead inline forwarders marked `UTILS_NOAPIGEN`:
```cpp
UTILS_NOAPIGEN
inline void addEntities(const utils::Entity* UTILS_NONNULL entities, size_t count) {
    addEntities({ entities, count });
}
```

The generator handles slice parameters through:
1. **Public Java Signature Simplification**:
   * Slices map directly to native Java arrays (`T[]`) or direct NIO `Buffer` instances without requiring an explicit size parameter in Java.
   * In the Java method body, `p` is passed directly to the native JNI call, which constructs the corresponding `utils::Slice<T>` from `(elements, length)` or `(nioBuffer.getData(), count)`.
   * For backward compatibility with legacy intermediate representations (IR), pointer + size parameter pairs annotated with `filament:apigen:size_param:<count>` are also supported and mapped cleanly to Java array overloads.
2. **Type Mapping for Slices & Pointer Arrays**:
   * `utils::Slice<const utils::Entity>` $\rightarrow$ `@NonNull @Entity int[]` (Java), `jintArray` (JNI).
   * `utils::Slice<const uint32_t>` $\rightarrow$ `@NonNull @IntRange(from = 0) int[]` (Java), `jintArray` (JNI).
   * `utils::Slice<const float>` $\rightarrow$ `@NonNull float[]` (Java), `jfloatArray` (JNI).
3. **JNI C++ Buffer Lifetime**:
   * Input pointers acquire elements via `env->Get<Type>ArrayElements(...)` and construct a stack-scoped `utils::Slice<T>` passed to the C++ API.
   * Constant slices release using `JNI_ABORT` (`env->ReleaseIntArrayElements(entities_, const_cast<jint *>(entities), JNI_ABORT);`) to prevent redundant JVM write-backs.
   * Mutable slices release with mode `0` to write back modified contents.

### Bone Transforms Buffers & Dual-Mode Marshalling (`SkinningBuffer`, `RenderableManager`)

Bone transforms upload methods (`setBones`) take slices of dual-quaternion bones (`utils::Slice<const RenderableManager::Bone>`) or 4x4 transformation matrices (`utils::Slice<const math::mat4f>`). The generator provides:

1. **Struct Float Stride Calculation (`get_struct_float_stride`)**:
   * Inspects member fields of aggregate structs. For `RenderableManager::Bone` (`quatf unitQuaternion` [4 floats] + `float3 translation` [3 floats] + `float reserved` [1 float] = 8 floats / 32 bytes), it computes a stride of 8 floats.
   * Maps `Bone const*` targets to `float[]` with `@Size(min = 8)` rather than treating them as unrolled scalar coordinates or unsafe 64-bit integer arrays (`long[]`), completely avoiding heap memory corruption.
2. **Dual-Mode Java Overloads**:
   * **Zero-Copy NIO `Buffer`**:
     * `void setBonesAs*(..., @NonNull Buffer transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset)`
     * Emits `BufferOverflowException` validation if remaining buffer capacity is insufficient.
     * Convenience overload defaulting `offset = 0`.
   * **Typed Float Array**:
     * `void setBonesAs*(..., @NonNull @Size(min = stride) float[] transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset)`
     * Bounds validation: throws `ArrayIndexOutOfBoundsException` if `transforms.length < count * stride`.
     * Convenience overload defaulting `offset = 0`.
     * Convenience overload defaulting `count = transforms.length / stride, offset = 0`.
3. **JNI C++ Bridge & Name Mangling**:
   * `Buffer` variants dispatch via `AutoBuffer nioBuffer(env, transforms_, count * stride)` and `static_cast<... const *>(data)` wrapped in `utils::Slice`.
   * `float[]` variants acquire elements with `env->GetFloatArrayElements(...)`, call `reinterpret_cast<... const *>(transforms)` wrapped in `utils::Slice`, and release with `JNI_ABORT`.
   * Disambiguates overloaded native static declarations via standard JNI signature mangling (`__JJLjava_nio_Buffer_2III` and `__JJ_3FII`).

### Alternate Method Names (`UTILS_APIGEN_ALTERNATE_NAME`)

C++ allows function overloading on parameter types, but in target languages (or when signatures collide after primitive mapping), explicit alternate names are needed.
* Annotated in C++ via `UTILS_APIGEN_ALTERNATE_NAME(newName)` (`[[clang::annotate("apigen:alternate_name:" #newName)]]`).
* Handled universally by `get_effective_method_name()`, applying both to keywords (e.g. `package` $\rightarrow$ `payload`) and overloaded methods (e.g. `setBones` $\rightarrow$ `setBonesAsQuaternions` and `setBonesAsMatrices`).
* Javadoc parameter maps automatically associate documentation from original C++ parameter names (`transforms`) with renamed Java parameters (`quaternions`, `matrices`).

### Tagged Array Buffers (`UTILS_APIGEN_TAGGED_ARRAY`)

When a C++ API parameterizes uniform or attribute array uploads across template specializations (e.g. `template<typename T> void setParameter(std::string_view name, UTILS_APIGEN_TAGGED_ARRAY utils::Slice<const T> values);` where `T` is constrained by SFINAE traits to scalars, vectors, or matrices), annotating the slice parameter with `UTILS_APIGEN_TAGGED_ARRAY` instructs the generator to collapse multiple template specializations into unified, type-tagged array methods:

1. **Family Grouping & Specialization Collapse**:
   * Specializations are classified by scalar family: `float` (`float`, `float2`, `float3`, `float4`, `mat3f`, `mat4f`), `int` (`int32_t`, `int4`), and `bool` (`bool`, `bool4`).
   * Instead of generating separate primitive array overloads that collide in Java (e.g. `float[]` for `float`, `float2`, `mat4f`), the generator emits a single method family per scalar base type.

2. **Element Type Enums (`<Family>Element`)**:
   * For each scalar family, an enum is provided or synthesized (`FloatElement`, `IntElement`, `BooleanElement`), with constants corresponding to the rank of each specialization (`FLOAT`, `FLOAT2`, `FLOAT3`, `FLOAT4`, `MAT3`, `MAT4`).
   * If the class already defines the enum (e.g. `MaterialInstance`), the generator reuses it. Otherwise, it generates the enum with `public int toFilamentNative() { return ordinal(); }`.

3. **Java Overloads**:
   * **Full 5-argument method**:
     ```java
     public void setParameter(@NonNull String name, @NonNull FloatElement type, @NonNull float[] values, @IntRange(from = 0) int offset, @IntRange(from = 1) int count)
     ```
   * **Convenience 4-argument overload** (defaulting `offset = 0`):
     ```java
     public void setParameter(@NonNull String name, @NonNull FloatElement type, @NonNull float[] values, @IntRange(from = 1) int count) {
         setParameter(name, type, values, 0, count);
     }
     ```

4. **JNI C++ Bridge & Typed Pointer Arithmetic**:
   * The JNI bridge receives `jint element, <scalar>Array values_, jint offset, jint count`.
   * Dispatches via `switch (element)` with typed element pointer arithmetic:
     ```cpp
     switch (element) {
         case 0: // FLOAT
             that->setParameter(name, ((const float*) values) + offset, count);
             break;
         case 1: // FLOAT2
             that->setParameter(name, ((const math::float2*) values) + offset, count);
             break;
         case 2: // FLOAT3
             that->setParameter(name, ((const math::float3*) values) + offset, count);
             break;
         case 3: // FLOAT4
             that->setParameter(name, ((const math::float4*) values) + offset, count);
             break;
         case 4: // MAT3
             that->setParameter(name, ((const math::mat3f*) values) + offset, count);
             break;
         case 5: // MAT4
             that->setParameter(name, ((const math::mat4f*) values) + offset, count);
             break;
     }
     ```
   * Memory safety is guaranteed using `Release<Scalar>ArrayElements(values_, values, JNI_ABORT)` and `ReleaseStringUTFChars`.
   * Automatically injects `<math/mat4.h>` and `<math/vec4.h>` headers if vector/matrix types are used.

### Managed Field Caching (GC Reachability & Symmetrical Accessors)

When a class defines symmetrical setter and getter pairs for managed Filament handles (e.g. `setSkybox(Skybox*)` and `getSkybox() -> Skybox*`), the generator:
1. **Synthesizes Java Member Fields**: Emits `private @Nullable <Type> m<Prop>;` on the class.
2. **Maintains GC Reachability**: In `set<Prop>(@Nullable <Type> obj)`, assigns `m<Prop> = obj;` prior to invoking the native setter `nSet<Prop>(...)`, ensuring the JVM garbage collector retains the Java wrapper.
3. **Zero-Cost Java Reads**: Emits `@Nullable public <Type> get<Prop>() { return m<Prop>; }` in pure Java, eliminating the JNI boundary crossing and avoiding wrapper re-allocation.
4. **JNI / Native Pruning**: Omits `nGet<Prop>()` from both the private Java `native` declarations and the generated JNI C++ translation unit.

---

### Functor / Callback Marshalling (`utils::Invocable`) & Exception Invariants

When C++ methods accept functional functors via `utils::Invocable<R(Args...)>&&`:
1. **Java SAM Interface Synthesis**: Generates a `@FunctionalInterface public interface <Method>Callback { R accept(Args...); }` inner interface on the Java class.
2. **Java Method Signature**: Emits `public <Ret> <method>(@NonNull <Method>Callback callback)` allowing direct Java lambda expressions (e.g. `scene.forEach(entity -> { ... })`).
3. **JNI Reflection & Dispatch**: Obtains the callback's `jclass` and `jmethodID` once per method call, then instantiates a C++ lambda forwarding arguments into `env->Call<Type>Method(...)`.
4. **JNI Exception Guarding**: Inside the C++ forwarding lambda, `env->ExceptionCheck()` is verified on every iteration. If the Java lambda threw an exception:
   - Further JNI invocations are immediately aborted (`return` / `return false`), avoiding undefined JVM behavior.
   - The loop unwinds cleanly.
   - When the JNI method returns to the JVM, the JVM transparently unwinds and throws the original Java exception (with full stack trace and type intact) to the caller.
5. **Noexcept Combinations**: Supports both `noexcept` and non-`noexcept` (wrapped in `wrapJni`) methods, with `void` and non-`void` return types and multi-parameter callbacks.

---

### Default Arguments & Java Telescoping Overloads

When C++ methods declare trailing arguments with default values (e.g. `far = INFINITY`, `direction = Fov::VERTICAL`, `scale = 1.0f`), the generator:
1. **Translates C++ Default Values to Java Expressions**:
   * `INFINITY` / `-INFINITY` $\rightarrow$ `Double.POSITIVE_INFINITY` / `Double.NEGATIVE_INFINITY` (or `Float.*`).
   * Enum constants (`Fov :: VERTICAL`) $\rightarrow$ `Fov.VERTICAL`.
   * Primitives and booleans (`0.0f`, `true`, `false`) $\rightarrow$ clean Java literals.
   * Unrolled vector default structs (`math::double3{0, 1, 0}`) $\rightarrow$ component arguments (`0.0, 1.0, 0.0`).
2. **Generates Telescoping Java Overloads**:
   * Generates public convenience overloads for each prefix of parameters omitting trailing default arguments.
   * Convenience overloads delegate directly to the fuller Java method, forwarding default values.
   * Generates filtered Javadoc on convenience overloads omitting `@param` tags for unexposed parameters.
   * Only the full method declares a `private static native` bridge, keeping the JNI surface area minimal.

### Standard JNI Overload Name Mangling

When a class defines overloaded native methods in Java (same native method name with different parameter types):
1. **Overload Detection**: Frequencies of native method names (`nMethod`) are tallied during class initialization.
2. **Official JNI Mangling**:
   * Non-overloaded native functions use standard short names: `Java_<package>_<Class>_<nativeMethod>`.
   * Overloaded native functions append a double underscore (`__`) followed by standard JNI type mangling of all native parameters: `Java_<package>_<Class>_<nativeMethod>__<MangledParams>`.
3. **Encoding Rules**:
   * `Z` (boolean), `B` (byte), `C` (char), `S` (short), `I` (int), `J` (long), `F` (float), `D` (double).
   * Arrays `T[]` $\rightarrow$ `_3` + encoded element type (e.g. `_3F` for `float[]`, `_3D` for `double[]`).
   * Reference types $\rightarrow$ `L<escaped_name>_2`.
4. **Examples**:
   * `nSetIntensity(long, float)` $\rightarrow$ `Java_com_google_android_filament_LightManager_nSetIntensity__JF`
   * `nSetIntensity(long, float, float)` $\rightarrow$ `Java_com_google_android_filament_LightManager_nSetIntensity__JFF`
   * `nInverse(double[], double[])` $\rightarrow$ `Java_com_google_android_filament_Camera_nInverseProjection___3D_3D`
   * `nInverse(float[], float[])` $\rightarrow$ `Java_com_google_android_filament_Camera_nInverseProjection___3F_3F`

---

### Aggregate Structs & Value Objects Architecture

#### 1. Zero-Allocation Java Structs (`Box`, `Aabb`)
When a type is marked `"is_aggregate": true`, `javagen.py` generates a dedicated value class:
* **Private Exploded Field Storage**: All member vectors (e.g. `float3 center`, `float3 halfExtent`) are stored as private scalar primitives (`mCenterX, mCenterY, mCenterZ, mHalfExtentX, mHalfExtentY, mHalfExtentZ`). Calling `new Box()` allocates exactly 1 single heap object with 0 auxiliary array objects.
* **Constructors**:
  - `public Box()`
  - `public Box(float centerX, float centerY, float centerZ, float halfExtentX, float halfExtentY, float halfExtentZ)`
  - `public Box(@NonNull @Size(min = 3) float[] center, @NonNull @Size(min = 3) float[] halfExtent)`
* **Ergonomic Accessors**:
  - Exploded setters: `setCenter(float x, float y, float z)`
  - Array setters: `setCenter(@NonNull @Size(min = 3) float[] center)`
  - Reusable out-getters: `@NonNull @Size(min = 3) public float[] getCenter(@Nullable @Size(min = 3) float[] out)`
  - Convenience getters: `public float[] getCenter() { return getCenter(null); }`
  - Scalar getters/setters: `public float getCenterX()`, `public void setCenterX(float val)`

#### 2. Cross-Class Struct Marshalling in JNI
* **Dynamic Class Registry (`KNOWN_CLASSES`)**: `javagen.py` scans sibling JSON IR files in the workspace to resolve known struct schemas.
* **Leaf Flattening (`get_flattened_leaves`)**: Struct parameters in methods (such as `Frustum.intersects(Box)`) recursively unroll their primitive leaves.
* **Register-Passed JNI Calls**: Java passes primitive scalars directly to JNI (`box.getCenterX()`, etc.) where they are received in CPU floating-point registers (AAPCS64 `s0-s5`).
* **Stack Construction**: In the JNI C++ bridge, stack objects are initialized directly from scalars:
  ```cpp
  Box box;
  box.center = { boxCenterX, boxCenterY, boxCenterZ };
  box.halfExtent = { boxHalfExtentX, boxHalfExtentY, boxHalfExtentZ };
  ```
* **Dynamic Header Resolution**: Generated JNI C++ files dynamically include referenced struct headers (e.g. `#include <filament/Box.h>`).

#### 3. Value Object Archetypes (`BITFIELD` and `INLINE_BUFFER`)
Unlike aggregate structs with exploded scalar fields (`Box`), value types with encapsulated internal state are handled via dedicated archetypes:
* **`BITFIELD` Archetype (`TextureSampler`)**: Packed bitfield values stored as a dynamically sized primitive (`short`, `int`, or `long` based on byte width $\le 2$, $\le 4$, or $\le 8$, falling back to standard struct when $> 8$ bytes). Mutators return updated bitfield values (`mSampler = nSet...`), const getters query native bitfields, and JNI bridges marshal via `filament::JniUtils::to_short`/`from_short`, `to_int`/`from_int`, or `to_long`/`from_long`.
* **`INLINE_BUFFER` Archetype (`Frustum`)**: Fixed-size primitive buffers stored as inline arrays (`mPlanes = new float[24]`). Passed directly to native JNI calls where memory is re-interpreted (`reinterpret_cast<Frustum*>(planes)`), and buffer getters execute pure Java array copies (`System.arraycopy`).

#### 4. Nested Aggregate Structs & Scoped Hierarchies (`LightManager.ShadowOptions`, `ShadowOptions.Vsm`)
* **Inner Class Nesting**: Aggregate structs defined within classes or outer structs are emitted as nested `public static class` structures within their enclosing parent classes in Java rather than emitting standalone `.java` and `.cpp` files.
* **Hierarchical Access Chains**: Parameter flattening in method calls tracks getter chains across nesting levels (e.g. `options.getVsm().getElvsm()`) to pass scalar primitives directly to JNI.
* **Sub-Struct Constructor Instantiation**: Constructors of aggregate structs containing nested sub-structs construct inner objects directly (`setVsm(new Vsm(vsmElvsm, vsmBlurWidth));`).
* **Builder Struct Parameter Marshalling**: When builder methods accept aggregate structs (e.g. `Builder::shadowOptions(ShadowOptions const&)`), leaves are unrolled across JNI and reconstructed on the stack using fully qualified scoped C++ types (`LightManager::ShadowOptions optionsCpp;`).

#### 5. POJO Struct Archetype (`POJO_STRUCT`, e.g. `View.*Options`)
Unlike zero-allocation aggregate structs (`Box`) with private fields and getter/setter accessors, post-processing options structs (such as `DynamicResolutionOptions`, `AmbientOcclusionOptions`, `BloomOptions`, etc. declared in `filament/Options.h` and aliased in `View.h`) require a mutable POJO contract for backward compatibility with existing Android client applications.

These structs are **automatically deduced** during extraction based on their declaration origin (`*Options.h`), completely eliminating hardcoded struct name whitelists (`POJO_STRUCT_NAMES`):
* **Automatic Origin Deduction**: Any struct defined in `filament/Options.h` (or matching `*Options.h`), or nested within one, is automatically tagged as `is_pojo_struct = True` and assigned archetype `pojo_struct`.
* **Public Mutable Fields**: Emitted as nested `public static class <Name>` with `public` mutable fields and direct Java field initializers (e.g. `public float minScale = 0.5f;`, `public boolean enabled = false;`).
* **Default Value Expression Desugaring**:
  * C++ vector literals with scalar directives (`apigen:java_type:float`) desugar to scalar floats: `{ 0.5f, 0.5f }` $\rightarrow$ `0.5f`.
  * C++ vector literals desugar to Java arrays: `{ 0, -1, 0 }` $\rightarrow$ `new float[]{0f, -1f, 0f}`.
  * C++ scoped enum references desugar to Java enum accesses: `QualityLevel::LOW` $\rightarrow$ `QualityLevel.LOW`.
  * C++ pointers desugar to Java references: `nullptr` $\rightarrow$ `null`.
  * C++ math limits desugar to Java constants: `INFINITY` $\rightarrow$ `Float.POSITIVE_INFINITY`.
* **Flattened Sub-Struct Inlining (`apigen:flatten`)**:
  * When a struct field is annotated with `apigen:flatten` (such as `Ssct ssct;` and `Gtao gtao;` in `AmbientOcclusionOptions`), child fields are recursively inlined into the outer class with the parent field name as a prefix (`ssctLightConeRad`, `gtaoSampleSliceCount`).
* **Symmetrical Option Property Caching**:
  * Symmetrical setter/getter pairs taking and returning `const <Option>Options&` synthesize private instance fields (`private <Option>Options m<Option>Options;`).
  * Pure Java getters perform lazy default initialization:
    ```java
    @NonNull
    public <Option>Options get<Option>Options() {
        if (m<Option>Options == null) {
            m<Option>Options = new <Option>Options();
        }
        return m<Option>Options;
    }
    ```
  * In the setter, `m<Option>Options = options;` updates the cache and forwards all unrolled scalar fields into `nSet<Option>Options(...)`.
  * Redundant JNI native getters are pruned.
* **JNI Stack Reconstruction**:
  * JNI bridges accept unrolled scalar primitives, instantiate C++ options structs on the stack (`View::<Option>Options optionsCpp;`), map scalar parameters (broadcasting scalar floats to vectors where appropriate, such as `math::float2{ minScale }`), populate flattened sub-structs (`optionsCpp.ssct`, `optionsCpp.gtao`), and pass the reconstructed struct directly to `that->set<Option>Options(optionsCpp);`.
* **Custom Callbacks & Legacy Stubs**:
  * Emits custom wrappers for asynchronous query mechanisms like `View.pick` using `JniCallback::make` and reflection.
  * Emits deprecated legacy stubs and enums (`ToneMapping`, `TargetBufferFlags`, `setToneMapping`, `getToneMapping`, `setSampleCount`, `getSampleCount`, `setAmbientOcclusion`, `getAmbientOcclusion`, `getUserTime`, `resetUserTime`) with `#pragma clang diagnostic push/pop` warning suppression in JNI C++.

---

## 5. Usage & Tools

### Running JavaGen Directly
```bash
python3 tools/apigen/javagen.py \
  --java-dir output/java \
  --jni-dir output/jni \
  path/to/Input.json
```

Or via Python import from the modular package:
```python
from tools.apigen.javagen import process_file, ClassContext, JavaEmitter, JniEmitter
```

### Full Android Bindings Generator
```bash
python3 tools/apigen/generate_android.py
```
Automatically extracts and generates verified production bindings for the active target suite (41 classes across core objects, buffers, tone mappers, managers, utility classes, and math types: `ACESLegacyToneMapper`, `ACESToneMapper`, `AgxToneMapper`, `Box`, `BufferObject`, `Camera`, `ColorGrading`, `Colors`, `DisplayRangeToneMapper`, `EntityManager`, `Exposure`, `Fence`, `FilmicToneMapper`, `FramePacer`, `Frustum`, `GT7ToneMapper`, `GenericToneMapper`, `IndexBuffer`, `IndirectLight`, `InstanceBuffer`, `LightManager`, `LinearToneMapper`, `Material`, `MaterialInstance`, `MorphTargetBuffer`, `PBRNeutralToneMapper`, `RenderableManager`, `RenderTarget`, `Renderer`, `Scene`, `SkinningBuffer`, `Skybox`, `Stream`, `SwapChain`, `Texture`, `TextureSampler`, `ToneMapper`, `TransformManager`, `VertexBuffer`, `View`, `Viewport`) directly into `android/filament-android/`. Automatically formats and organizes C++ JNI headers via `tools/reorganize-headers/run.py` per generated target.

### Batch WIP Update Script
```bash
python3 tools/apigen/wip/update.py
```

---

## 6. Feature Working Log & Changelog

### 2026-09-09
* **`View` Migration & Complete Beamsplitter Decoupling**:
  * Transitioned `View` to the automated APIGen pipeline as the 41st active target, eliminating handwritten Java and JNI C++ code for `View`.
  * Implemented POJO struct archetype in `java_emitter.py` for 14 post-processing option structs defined in `Options.h`: `DynamicResolutionOptions`, `AmbientOcclusionOptions`, `BloomOptions`, `FogOptions`, `DepthOfFieldOptions`, `VignetteOptions`, `ColorGradingOptions`, `TemporalAntiAliasingOptions`, `ScreenSpaceReflectionsOptions`, `GuardBandOptions`, `StereoscopicOptions`, `VsmShadowOptions`, `SoftShadowOptions`, `MultiSampleAntiAliasingOptions`, and `RenderQuality`.
  * Added field default value desugaring in `type_resolver.py` for C++ vector initializers (`{0.5f, 0.5f}` $\rightarrow$ `0.5f`, `{0, -1, 0}` $\rightarrow$ `{0f, -1f, 0f}`), enums, pointers, and infinity.
  * Implemented sub-struct field flattening (`apigen:flatten`): recursively inlines `Ssct` and `Gtao` child fields into `AmbientOcclusionOptions` with prefixed names (`ssct*`, `gtao*`).
  * Implemented symmetrical option property caching: lazy-initializing pure Java getters (`get<Option>Options()`), unrolled scalar setter JNI calls, and stack reconstruction in `jni_emitter.py` (`View::<Option>Options optionsCpp;`).
  * Added custom reflection bridge for `View.pick` (`OnPickCallback`, `InternalOnPickCallback`, and `nPick`).
  * Added `#pragma clang diagnostic ignored "-Wdeprecated-declarations"` for deprecated method calls in JNI.
  * Completely decoupled `beamsplitter`: deleted `emitters/java.go` and `emitters/java.template`; `beamsplitter` now strictly handles JS/TS bindings and settings serialization.
### 2026-09-10
* **`Engine` Migration & 100% Core Automated Coverage**:
  * Transitioned `Engine` to the automated APIGen pipeline (`Engine.java` and `Engine.cpp`) as the 42nd target, completing 100% automated binding coverage across Filament core.
  * Added declarative C++ annotations in `Engine.h`: `UTILS_APIGEN_USED_BY_NATIVE` on `Engine` class, `UTILS_NOAPIGEN` on all 21 `isValid` overloads and internal methods (`createSync`, `destroy(Sync*)`, `destroy(Engine**)`, `destroy(Engine*)`, `getDriver`, `getPlatform`, etc.).
  * Implemented type-qualified resource destruction bridges (`nDestroy<Type>`) across 20 resource types, generating backward-compatible `destroy<Type>(T target)` and overloaded `destroy(T target)` forwarders with `assertDestroy(success)` and `target.clearNativeObject()`.
  * Generated nested `Engine.Config` POJO struct with public fields and native `#define` constants matching handwritten signatures.
  * Generated `Engine.Builder` supporting platform surface and shared context validation via `Platform.get()`, manager accessor caching, and feature flag management.
  * Validated full static parity with zero errors across all 40 active targets and successfully compiled Android release (`sample-hello-triangle`, `sample-gltf-viewer`).
* **Refactored JobSystem Extraction to `FilamentHelper`**:
  * Deleted `int64_t getNativeJobSystem()` from `Engine.h`, unpolluting the public C++ header to strictly expose `utils::JobSystem& getJobSystem() noexcept;`.
  * Moved the native job system accessor to `FilamentHelper.getJobSystem(@NonNull Engine engine)` in `filament-android`, backed by `FilamentHelper.cpp`.
  * Updated `MaterialBuilder.java` to reflectively query `FilamentHelper.getJobSystem(Engine)` first, retaining a backward-compatible fallback to `Engine.getNativeJobSystem()`.
  * Purged `"getNativeJobSystem"` from `intercepted_engine_methods` in `java_emitter.py` and `jni_emitter.py`, and eliminated handwritten JNI bridges from `Engine.cpp`.

### 2026-09-09
* **Bitfield Auto-Deduction & Dynamic Primitive Sizing**:
  * Eliminated `UTILS_APIGEN_BITFIELD` macro from `compiler.h` and removed annotation from `TextureSampler.h`.
  * Implemented automatic AST bitfield deduction in `extractor.py`: detects structs comprised entirely of bitfield members (`cursor.is_bitfield()`) as well as wrapper classes whose single data member is a bitfield struct.
  * Added dynamic primitive sizing: `sizeof <= 2` $\rightarrow$ `short` / `jshort`, `sizeof <= 4` $\rightarrow$ `int` / `jint` (transitions `TextureSampler` to `int mSampler`), `sizeof <= 8` $\rightarrow$ `long` / `jlong`. Structs $> 8$ bytes (> 64 bits) gracefully fall back to standard class/struct generation.
  * Added templated and overloaded `to_short`/`from_short`, `to_int`/`from_int`, and `to_long`/`from_long` helpers in `android/common/JniUtils.h`.
  * Updated `java_emitter.py`, `jni_emitter.py`, `context.py`, and `type_resolver.py` to marshal dynamically sized primitives. Regenerated Android bindings and updated all tests and fixtures.
* **Elimination of `POJO_STRUCT_NAMES` & Dynamic Options Struct Deduction**:
  * Completely removed hardcoded `POJO_STRUCT_NAMES` set from `config.py`.
  * Implemented AST header-origin deduction in `extractor.py`: all structs declared in `filament/Options.h` (or matching `*Options.h`) or nested within them are tagged with `is_pojo_struct = True` and assigned archetype `pojo_struct`.
  * Updated `type_resolver.py` (`is_pojo_struct` and `is_aggregate_struct`) to classify POJO structs dynamically from IR metadata and location without hardcoded name whitelists.
  * Verified 100% zero-diff regeneration across all 41 targets and added unit test coverage for new options structs.

### 2026-09-08
* **`Stream` Migration & `StreamHelper` Architecture**:
  * Transitioned `Stream` to the automated APIGen pipeline as the 40th active target.
  * Annotated `Builder::stream(void*)` and `setAcquiredImage(...)` with `UTILS_NOAPIGEN` in `Stream.h` to decouple Android-specific interop from the core engine binding.
  * Created `com.google.android.filament.android.StreamHelper` and `StreamHelper.cpp` to handle `SurfaceTexture` stream source association and `HardwareBuffer` image acquisition with release callback trampolines.
  * Updated `sample-hello-camera` and `sample-stream-test` call sites to use `StreamHelper`.
* **`SwapChain` Migration & Nullable String Handling**:
  * Transitioned `SwapChain` to the automated APIGen pipeline as the 39th active target.
  * Applied `UTILS_APIGEN_RETAINED` to `SwapChain::getNativeWindow()` to retain native window references.
  * Implemented generic async callback convenience overloads for trailing default arguments on `setFrameScheduledCallback`.
  * Decoupled `SwapChainFlags` into a standalone helper class.
  * Fixed nullable return handling for C-strings (`UTILS_NULLABLE`): JNI returns `nullptr` for null strings, delivering `null` to Java rather than empty string `""` fallback (e.g. `Material.getParameterTransformName()`).
* **Unhardcoded `Builder::name`**:
  * Builder `name` methods accept `utils::ImmutableCString` directly, removing hardcoded method name checks and specialized `strlen` expansion from JNI code generation.

### 2026-09-07
* **Generic Retained Objects (`UTILS_APIGEN_RETAINED`)**:
  * Added generic data-driven support for retained object references in handle classes (`Renderer.getEngine()`, `MaterialInstance.getMaterial()`, `MaterialInstance.getEngine()`), standardizing constructor parameter order to `(long nativeObject, Object ref)` and bypassing JNI on getters.
  * Added `BuilderWrapper` with `std::vector<std::unique_ptr<AutoBuffer>> retainedBuffers` in JNI code generation to retain direct NIO buffers across builder configuration.
* **Generic Async Callback Emission**:
  * Added automated SAM interface generation (`@FunctionalInterface`) and convenience overload synthesis for asynchronous completion callbacks with optional callback handlers.
* **Unhardcoded Texture, Colors, and Final `Object` Collisions**:
  * Generalized `ByteBuffer` import resolution using `PixelBufferDescriptor` aliases.
  * Replaced hardcoded class name checks for `LinearColor` with alias resolution.
  * Added `JAVA_OBJECT_FINAL_NOARG_METHODS` to prevent collisions with final `java.lang.Object` methods (`wait()`).

### 2026-09-06
* **`javagen.py` Package Decomposition & Modular Refactoring**:
  * Refactored monolithic 8,264-line `javagen.py` script into a clean, PEP 8-compliant Python package under `tools/apigen/javagen/`.
  * Decomposed into 10 cohesive modules:
    * `errors.py`: Custom exception hierarchy (`JavaGenError`, `TypeResolutionError`, `TemplateExpansionError`, `EmissionError`, `IRParseError`).
    * `config.py`: Static configuration tables (`TYPE_MAP`, `MATH_TYPES`, `VALUE_TYPES`, `TAGGED_SCALAR_FAMILIES`, `JAVA_KEYWORDS`), standard headers, and dynamic class registry (`KNOWN_CLASSES`, `register_known_classes`).
    * `utils.py`: Pure helper functions for AST attributes, identifier sanitization, enum naming, and tagged array inspection.
    * `doc.py`: CommonMark AST parsing, Markdown-to-Javadoc translation, HTML table rendering, and Javadoc tag synthesis (`format_see_tag`, `generate_javadoc`).
    * `type_resolver.py`: Dedicated `TypeResolver` engine for type classification, struct leaf traversal, buffer classification, JNI stack reconstruction, and slice cleanup.
    * `context.py`: `ClassContext` data model capturing class IR parsing, AST attribute analysis, SFINAE template specialization unrolling, signature tracking, and delegation.
    * `java_emitter.py`: `JavaEmitter` for emitting Java class wrapper files, builder inner classes, aggregate structs, vector array overloads, convenience overloads, and JNI declarations.
    * `jni_emitter.py`: `JniEmitter` for emitting zero-cost JNI C++ bridges, buffer pointer unwrapping, aggregate reconstruction, and slice handlers.
    * `orchestrator.py`: Multi-file IR pipeline execution (`process_file`) and CLI driver `main()`.
    * `__init__.py`: Public package interface re-exporting key classes and constants.
  * Preserved `tools/apigen/javagen.py` as a backward-compatible CLI entry point and API facade, ensuring external build tools and tests continue working seamlessly.
  * Added standard Android Apache 2.0 license headers and detailed module docstrings to every file.
  * Verified 100% functional parity: all 38 Android Java and JNI C++ binding targets generate bit-for-bit identical code (`git diff android/` is completely empty), and all 77 unit tests in `tools/apigen/tests/` pass cleanly.

### 2026-09-04
* **`RenderableManager` Production Promotion & Special Features**:
  * **`AttributeBitset` to `Set<VertexBuffer.VertexAttribute>`**: Added detection for `utils::bitset` representing vertex attribute slots (`is_attribute_bitset`). Generated `@NonNull Set<VertexBuffer.VertexAttribute>` return types backed by unmodifiable `EnumSet` instances via cached `sVertexAttributeValues` array, with `private static Set<VertexBuffer.VertexAttribute> getAttributes(int bitSet)` decoding helper. Emitted `import java.util.Collections;`, `import java.util.EnumSet;`, and `import java.util.Set;`. JNI bridges invoke `.getValue()` on native single-word bitsets.
  * **Native `setGeometryAt` Convenience Overloads**: Added native C++ convenience overloads to `RenderableManager.h` and `RenderableManager.cpp` matching `Builder::geometry` defaults (`0, indices->getIndexCount()` and `0, vertices->getVertexCount()`), avoiding generator special-casing and binding them naturally across C++, Java, and JNI.
  * **Dual `Builder.skinning` & `skinningAsMatrices` Overloads**: Added dual NIO `Buffer` and typed `float[]` overloads for both quaternion bones (`skinning`) and transform matrices (`skinningAsMatrices`) inside `RenderableManager.Builder`. Synthesized private native methods `nBuilderSkinningBones` and `nBuilderSkinningMatrices` with `AutoBuffer` safety, bounds checking, and `BufferOverflowException` handling.
  * **`RenderableManager` Target Promotion**: Promoted `RenderableManager` to active target in `generate_android.py`, bringing active Android auto-generated target count to 36 classes (70.6% total coverage).

### 2026-09-02
* **`VertexBuffer` & Buffer Bindings**: Integrated `VertexBuffer` generation with generic `BufferDescriptor` / `NioUtils` / `JniBufferCallback` zero-copy asynchronous buffer copy bindings, `VertexAttribute` explicit integer constant mappings with aliases (`MORPH_POSITION_0`, etc.), and `AttributeType` (`backend::ElementType`) enums.
* **Sub-32-Bit Integer Unification**: Unified mapping of all sub-32-bit scalar integer types (`int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `short`, `unsigned short`, `char`, `unsigned char`, `signed char`) to Java `int` and JNI `jint` (with `@IntRange(from = 0)` on unsigned types), matching idiomatic Java/Android conventions and casting to C++ target types in JNI bridges.
* **Nested Enum Aliases in Builders**: Added generic support for builder-nested enum aliases (e.g. `IndexBuffer.Builder.IndexType`), generating nested enum definitions inside `Builder` classes with proper AST-derived integer value and ordinal handling.
* **Sync Class Annotation**: Annotated `Sync.h` with `UTILS_NOAPIGEN` to cleanly skip Java binding generation.
* **Buffer & ToneMapper Expansion**: Added `BufferObject`, `IndexBuffer`, `InstanceBuffer`, `MorphTargetBuffer`, `SkinningBuffer`, `ToneMapper` and all 9 concrete `ToneMapper` subclasses (`ACESLegacyToneMapper`, `ACESToneMapper`, `AgxToneMapper`, `DisplayRangeToneMapper`, `FilmicToneMapper`, `GenericToneMapper`, `GT7ToneMapper`, `LinearToneMapper`, `PBRNeutralToneMapper`) to active targets in `generate_android.py`.
* **Nested Aggregate Structs & Hierarchical Flattening**: Implemented recursive nested aggregate struct emission within parent classes in Java (e.g. `LightManager.ShadowOptions.Vsm`), suppressed redundant standalone file generation, tracked getter access chains (`options.getVsm().getElvsm()`), and generated scoped stack reconstructions in JNI.
* **Fixed-Size Array Struct Flattening**: Added generic fixed-size array mapping `T[N]` $\rightarrow$ `@NonNull @Size(min = N) T[]` with indexed scalar unrolling (`var0`, `var1`, ...) and element-by-element assignment in JNI C++ bridges.
* **Builder Struct Parameter Marshalling**: Added support for aggregate struct parameters in builder methods (`Builder.shadowOptions(ShadowOptions)`), unrolling leaves to JNI with scoped C++ struct reconstruction.
* **Quaternion Support**: Registered quaternion types (`math::quatf`, `math::quat`, `TQuaternion<float>`) in `MATH_TYPES` with 4-component vector flattening.
* **Active Android Target Suite**: Extended `generate_android.py` to target 33 core classes.
* **`UTILS_APIGEN_BITFIELD` & `apigen:` Annotation Namespace**: Standardized all APIGen annotations on the `apigen:` namespace prefix (`apigen:skip`, `apigen:size_param:<param>`, `apigen:bitfield`). Implemented `UTILS_APIGEN_BITFIELD` to drive dynamic archetype resolution for packed 64-bit value classes like `TextureSampler` without hardcoding in `VALUE_TYPES`.
* **Dynamic Zero-Allocation `EnumCache`**: Any class whose native methods return an enum dynamically synthesizes an inner `EnumCache` with cached `.values()` arrays, eliminating heap allocations on high-frequency queries.
* **`TextureSampler` Support**: Fully integrated `TextureSampler` as a `BITFIELD` archetype with dynamic constructor telescoping, formatted Javadoc lists, and `filament::JniUtils::to_long`/`from_long` marshaling.
* **Utility Class Support (`Colors`, `Exposure`)**: Implemented code generation for pure utility classes with private constructors, all-static methods, omission of `mNativeObject` handles, and direct JNI invocation of static C++ member functions.
* **`@LinearColor` Annotation Synthesis**: Generates `@Retention(SOURCE)` and `@Target(...) public @interface LinearColor {}` annotation definitions and applies `@LinearColor` to method parameters and return types across math vectors.
* **Conditional Math Namespace in JNI**: JNI `.cpp` generation now conditionally emits `using namespace filament::math;` only when math vector/matrix types or headers are referenced.
* **Generated File Warnings**: Added standard top and bottom generator warning comments (`// THIS FILE IS GENERATED BY APIGEN AND MUST NOT BE HAND-MODIFIED.`) to generated `.java` and `.cpp` outputs.

### 2026-09-04
* **Template Trait Specialization Expansion & JNI Bridge**: Added automated generation for methods with `specializations` from SFINAE traits. Implemented vector scalar unrolling and array overload generation, return-type template name unmangling (e.g. `getConstantFloat`, `getConstantInt`, `getConstantBoolean`), Java signature deduplication (preventing duplicate methods from types like `int32_t` and `uint32_t`), nested Builder template support, and explicit JNI C++ template dispatch (`that->method<Type>(...)` / `builder->method<Type>(...)`).
* **Universal Javadoc Metadata Tag Rendering (`note`, `warning`, `deprecated`, `see`)**: Updated `generate_javadoc` in `javagen.py` to extract and emit `note` and `warning` metadata tags into clean `<p>...</p>` paragraph blocks, `@deprecated` tags, and scope-normalized `@see` links (`Builder#method`). Restored missing advisory paragraphs across classes including `RenderableManager.Builder.culling` and `TransformManager.getInstance`.

### 2026-09-03
* **Bone Transforms Buffers & Dual-Mode Marshalling (`SkinningBuffer`)**: Added full support for bone transforms buffers (`Bone const*` and `math::mat4f const*` paired with `count` and optional `offset`). Implemented aggregate struct float stride calculations (`RenderableManager::Bone` $\rightarrow$ 8 floats) to eliminate legacy heap corruption from 64-bit `long[]` bindings. Emitted dual Java method variants: zero-copy NIO `Buffer` (with `BufferOverflowException` capacity checks) and typed `float[]` (with `@Size(min = stride)` bounds validation, default `offset = 0`, and length auto-division). Emitted overloaded JNI static methods with standard signature mangling, using `AutoBuffer` for direct buffers and `GetFloatArrayElements` with `reinterpret_cast` for float arrays. Generalized `UTILS_APIGEN_ALTERNATE_NAME` across all methods and created test fixture `23_bone_buffer_bindings` in `tests/java/`.
* **Aggregate Struct Output Parameter & Box Out-Param Golden Test**: Added golden integration test coverage in `21_external_aggregates` (`ExternalAggregatesTest.getBoundingBox()`) verifying Java out-parameter signatures (`@NonNull Box getBoundingBox(@Nullable Box out)`), zero-allocation convenience overloads (`@NonNull Box getBoundingBox()`), and JNI C++ field setters (`JniBoxState`). Updated import resolution in `javagen.py` to ensure `androidx.annotation.Nullable` and `NonNull` are imported when methods return aggregate structs.
* **`backend::PixelBufferDescriptor&&` Method Unrolling & JNI Bridge**: Added full support in `javagen.py` for methods accepting `backend::PixelBufferDescriptor&&` (shared between `Renderer::readPixels` and `Texture::setImage`). Generates Java methods taking `@NonNull PixelBufferDescriptor buffer`, enforces read-only buffer checks for readback methods (`ReadOnlyBufferException`), unrolls buffer parameters into private native declarations, and handles buffer overflow errors (`BufferOverflowException`). Emits JNI C++ functions with overloaded method signature mangling (`Buffer` $\rightarrow$ `Ljava_nio_Buffer_2`, `Runnable` $\rightarrow$ `Ljava_lang_Runnable_2`), calculates buffer sizes via `backend::PixelBufferDescriptor::computeDataSize` with compressed texture handling, binds storage using `AutoBuffer`, dispatches callbacks with `JniBufferCallback::make`, and constructs `backend::PixelBufferDescriptor` before dispatching to the native C++ method.

### 2026-09-02
* **`UTILS_APIGEN_ALTERNATE_NAME(name)` Support**: Added support for designating alternate method names on C++ declarations via `[[clang::annotate("apigen:alternate_name:" #name)]]`. Activated if and only if the method's native name collides with a reserved keyword in the target language (e.g. Java keywords `package` in `Material::Builder::package` $\rightarrow$ `payload`, and `import` in `Texture::Builder::import` $\rightarrow$ `importTexture`). Emits target-safe Java signatures and JNI bridge export functions while preserving the native C++ method call in the C++ JNI bridge.
* **Standalone `PixelBufferDescriptor` with Native `computeDataSize` Synchronization**: Decoupled `PixelBufferDescriptor` from `Texture.java` and APIGen generation via `UTILS_NOAPIGEN`, maintaining it as a clean handwritten class in `filament-android` and delegating `computeDataSize` to native C++ `backend::PixelBufferDescriptor::computeDataSize` via JNI.

### 2026-09-01
* **`UTILS_NOAPIGEN` / `no_apigen` Annotation Support**: Added full support for excluding classes, structs, methods, fields, constants, and enums annotated with `UTILS_NOAPIGEN` / `[[clang::annotate("no_apigen")]]` (or `binding:skip`, `apigen:skip`, `skip_generation`), replacing heuristic filtering with explicit, declarative C++ annotation control.
* **Object Reference Returns & `utils::*` Handles**: Extended `is_filament_type` resolution to support `utils::*` types (e.g. `utils::EntityManager&`), emitting address-of cast expressions `(jlong)&(val)` in JNI and wrapper class instantiation `new EntityManager(result)` in Java.
* **Class Constants Generation (`cls["constants"]`)**: Emits `public static final <type> <NAME> = <value>;` with Javadoc from extracted class `constants`. Generically resolves typed cast expressions (e.g. `uint64_t(-1)` $\rightarrow$ `-1`), numeric limits, and constant reference alias chains.
* **Java Reserved Keyword Sanitization**: Added `JAVA_KEYWORDS` set and `sanitize_identifier()` to escape collisions in expanded vector components and parameters (e.g. `try` $\rightarrow$ `try_`) across Java signatures, JNI method calls, and Javadoc.
* **`Object.wait()` Collision Avoidance**: Automatically skips generating 0-argument `wait()` overloads in Java when default parameters are present to prevent overriding `final void java.lang.Object.wait()`.
* **JNI Reference Dereferencing**: Added pointer dereferencing (`*param`) in JNI bridge invocations when the underlying C++ method signature takes object reference types (`Frustum const&`, etc.).
* **Automated Android Target Generation**: Created `generate_android.py` to extract, generate, and verify bindings directly in `android/filament-android/` for `Box`, `Camera`, `EntityManager`, `Fence`, `Frustum`, `Scene`, and `TransformManager`.

### 2026-08-31
* **Phase 1 String Type Marshalling**: Full support for `std::string_view`, `std::basic_string_view<char>`, `utils::CString`, `utils::StaticString`, `utils::ImmutableCString`, `std::string`, and `const char*` across Java (`@NonNull String` / `@Nullable String`) and JNI (`jstring`, `GetStringUTFChars`, `ReleaseStringUTFChars`, `NewStringUTF`).
* **Phase 1 Chrono, Tribool, Optional, and Bitset Marshalling**: Supported `std::chrono::duration` and `std::chrono::time_point` (mapping to `@IntRange(from = 0) long` in nanoseconds), `utils::tribool` and `std::optional<bool>` (mapping to `boolean`), and `utils::bitset<uint32_t>` (mapping to `@IntRange(from = 0) int`).
* **Generic Aggregate Structs Pipeline (`Box`, `Aabb`)**: Built zero-allocation Java struct code generation (`_generate_aggregate_java`) with exploded primitive fields, out-parameter getters, exploded and array setters, and multi-signature constructors.
* **Register-Passed Zero-Copy JNI Struct Marshalling**: Implemented `KNOWN_CLASSES` registry and recursive leaf flattening (`get_flattened_leaves`), unrolling struct arguments across Java calls and JNI signatures (`Frustum.intersects(Box)`), passing scalars directly via CPU registers with stack reconstruction in JNI.
* **Encapsulated Value Classes (`Frustum`)**: Generated value classes holding backing primitive arrays (`mPlanes = new float[24]`) without `mNativeObject`, re-interpreting memory in JNI (`reinterpret_cast<Frustum*>(planes)`).
* **Dynamic JNI Header Discovery**: Automatically scans referenced aggregate types across methods and includes their respective C++ headers in generated `.cpp` files.
* **C++ Default Argument Telescoping**: Generated telescoping Java overloads for methods with trailing C++ default arguments (`INFINITY`, `Fov::VERTICAL`, unrolled vector defaults), forwarding translated Java expressions to the full method with filtered Javadoc.
* **Standard JNI Overload Name Mangling**: Implemented Oracle JNI specification name mangling (`__<Signature>`) for overloaded native methods, correctly encoding primitive types, object references, and array types (`_3F`, `_3D`, `JFF`, `JIF`, etc.).
* **Nested Enum Generation & Marshalling**: Added generation of nested Java `public enum <Name> { ... }` with docstrings. Emits `toFilamentNative()` in every enum. Method arguments take `@NonNull <Enum>` and marshal `toFilamentNative()` to `jint`, casting to `(<Class>::<Enum>)` in JNI C++. Enum returns call `<Enum>.values()[nMethod(...)]`.
* **Static Method Generation & Marshalling**: Generated `public static` Java methods omitting `getNativeObject()`, corresponding JNI declarations without `native<Class>` handles, and direct C++ static method calls (`Class::method(...)`).
* **`utils::Invocable` Functor & Callback Marshalling**: Implemented parsing and code generation for `utils::Invocable<R(Args...)>&&` parameters. Synthesizes `@FunctionalInterface` SAM interfaces in Java, generates native lambda bridges in JNI, and enforces JNI exception check guards (`env->ExceptionCheck()`) to safely propagate Java lambda exceptions across all combinations of `noexcept` and return types.
* **Managed Field Caching**: Auto-detection of symmetrical setter/getter pairs for Filament object handles (`setSkybox`/`getSkybox`, `setIndirectLight`/`getIndirectLight`). Automatically emits cached fields (`mSkybox`, `mIndirectLight`), manages Java-side retention in setters, returns cached fields in getters, and prunes unnecessary native JNI getter functions.
* **Pointer + Size Pair Recognition & Collapsing**: Implemented recognition of `[[clang::annotate("size_param:<count>")]]` annotations across primitive pointers, entity buffers, and math arrays. Automatically omits single-consumer count parameters from Java public signatures and forwards `array.length` / `array.length / stride` to native methods with `JNI_ABORT` release semantics.
* **Handle Validation on Method Calls**: Updated Java method generators to invoke `getNativeObject()` instead of referencing `mNativeObject` directly, enforcing destroyed object assertions at all call sites.
* **`size_t` Sizing Contexts**: Mapped `size_t` / `ssize_t` to Java `int` / JNI `jint` (with `@IntRange(from = 0)`) matching Android collection count and indexing conventions.
* **C++ Exception Handling via `wrapJni`**: Added automatic wrapping for non-`noexcept` methods (`wrapJni(env, ...)` and `wrapJni<R>(env, ...)`), including safe bracketed JNI array element acquisition/release for math outputs and buffer inputs.
* **Include `<common/JniUtils.h>`**: Added header inclusion and `using namespace filament::android;` to all generated JNI C++ files.
* **FQN & Typedef Desugaring Support**: Updated `resolve_type_info` to match against canonical desugared FQNs and fixed-width integer types (`int8_t`, `uint32_t`, etc.).
* **Refined Annotation Imports**: Added strict conditional importing of `androidx.annotation.*` and standard `java.lang.IllegalStateException`.
* **Automated WIP Sync Script**: Created `tools/apigen/wip/update.py` for single-command extraction and generation of `Box`, `Camera`, `Frustum`, `LightManager`, and `Scene`.

### Earlier
* **Math Return & Input Unrolling**: Support for returning and passing libmath vectors and matrices.
* **Filament Engine Types**: Marshalling of object pointers to native handle integers.
* **Markdown Javadoc Converter**: Formatting CommonMark documentation into HTML Javadoc blocks.
