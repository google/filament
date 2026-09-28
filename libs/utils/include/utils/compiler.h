/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_UTILS_COMPILER_H
#define TNT_UTILS_COMPILER_H

#include <stddef.h>

// compatibility with non-clang compilers...
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_feature
#define __has_feature(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_attribute(visibility)
#    define UTILS_PUBLIC  __attribute__((visibility("default")))
#else
#    define UTILS_PUBLIC
#endif

// UTILS_SHARED_LINKING marks symbols that need default visibility only when
// Filament is consumed as a shared/dynamic library. Unlike UTILS_PUBLIC,
// which denotes the intentional public API surface, these symbols are
// implementation details that must be visible across shared-library
// boundaries.
#if __has_attribute(visibility)
#    define UTILS_SHARED_LINKING __attribute__((visibility("default")))
#else
#    define UTILS_SHARED_LINKING
#endif

#if __has_attribute(deprecated)
#   define UTILS_DEPRECATED [[deprecated]]
#else
#   define UTILS_DEPRECATED
#endif

#if __has_attribute(packed)
#   define UTILS_PACKED __attribute__((packed))
#else
#   define UTILS_PACKED
#endif

#if __has_attribute(noreturn)
#    define UTILS_NORETURN __attribute__((noreturn))
#else
#    define UTILS_NORETURN
#endif

#if __has_attribute(fallthrough)
#   define UTILS_FALLTHROUGH [[fallthrough]]
#else
#   define UTILS_FALLTHROUGH
#endif

#if __has_attribute(visibility)
#    ifndef TNT_DEV
#        define UTILS_PRIVATE __attribute__((visibility("hidden")))
#    else
#        define UTILS_PRIVATE
#    endif
#else
#    define UTILS_PRIVATE
#endif

#define UTILS_NO_SANITIZE_THREAD
#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
#undef UTILS_NO_SANITIZE_THREAD
#define UTILS_NO_SANITIZE_THREAD __attribute__((no_sanitize("thread")))
#endif

#define UTILS_HAS_SANITIZE_THREAD 0
#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
#undef UTILS_HAS_SANITIZE_THREAD
#define UTILS_HAS_SANITIZE_THREAD 1
#endif

#define UTILS_HAS_SANITIZE_MEMORY 0
#if __has_feature(memory_sanitizer)
#undef UTILS_HAS_SANITIZE_MEMORY
#define UTILS_HAS_SANITIZE_MEMORY 1
#endif

#define UTILS_HAS_SANITIZE_ADDRESS 0
#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#undef UTILS_HAS_SANITIZE_ADDRESS
#define UTILS_HAS_SANITIZE_ADDRESS 1
#endif

#if UTILS_HAS_SANITIZE_ADDRESS
#ifdef __cplusplus
extern "C" {
#endif
void __asan_poison_memory_region(void const volatile *addr, size_t size);
void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
#ifdef __cplusplus
}
#endif
#define UTILS_POISON_MEMORY_REGION(addr, size) __asan_poison_memory_region((addr), (size))
#define UTILS_UNPOISON_MEMORY_REGION(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
#define UTILS_POISON_MEMORY_REGION(addr, size) ((void)0)
#define UTILS_UNPOISON_MEMORY_REGION(addr, size) ((void)0)
#endif

/*
 * helps the compiler's optimizer predicting branches
 */
#if __has_builtin(__builtin_expect)
#   ifdef __cplusplus
#      define UTILS_LIKELY( exp )    (__builtin_expect( !!(exp), true ))
#      define UTILS_UNLIKELY( exp )  (__builtin_expect( !!(exp), false ))
#   else
#      define UTILS_LIKELY( exp )    (__builtin_expect( !!(exp), 1 ))
#      define UTILS_UNLIKELY( exp )  (__builtin_expect( !!(exp), 0 ))
#   endif
#else
#   define UTILS_LIKELY( exp )    (!!(exp))
#   define UTILS_UNLIKELY( exp )  (!!(exp))
#endif

#if __has_builtin(__builtin_mul_overflow)
#   define UTILS_HAS_BUILTIN_MUL_OVERFLOW 1
#   define UTILS_MUL_OVERFLOW(a, b, res) __builtin_mul_overflow((a), (b), (res))
#else
#   define UTILS_HAS_BUILTIN_MUL_OVERFLOW 0
#   define UTILS_MUL_OVERFLOW(a, b, res) (*(res) = (a) * (b), false)
#endif

#if __has_builtin(__builtin_expect_with_probability)
#   ifdef __cplusplus
#      define UTILS_VERY_LIKELY( exp )    (__builtin_expect_with_probability( !!(exp), true, 0.995 ))
#      define UTILS_VERY_UNLIKELY( exp )  (__builtin_expect_with_probability( !!(exp), false, 0.995 ))
#   else
#      define UTILS_VERY_LIKELY( exp )    (__builtin_expect_with_probability( !!(exp), 1, 0.995 ))
#      define UTILS_VERY_UNLIKELY( exp )  (__builtin_expect_with_probability( !!(exp), 0, 0.995 ))
#   endif
#else
#   define UTILS_VERY_LIKELY( exp )    (!!(exp))
#   define UTILS_VERY_UNLIKELY( exp )  (!!(exp))
#endif

#if __has_builtin(__builtin_prefetch)
#   define UTILS_PREFETCH( exp ) (__builtin_prefetch(exp))
#else
#   define UTILS_PREFETCH( exp )
#endif

#if __has_builtin(__builtin_assume)
#   define UTILS_ASSUME( exp ) (__builtin_assume(exp))
#else
#   define UTILS_ASSUME( exp )
#endif

#if (defined(__i386__) || defined(__x86_64__))
#   define UTILS_HAS_HYPER_THREADING 1      // on x86 we assume we have hyper-threading.
#else
#   define UTILS_HAS_HYPER_THREADING 0
#endif

#if defined(FILAMENT_SINGLE_THREADED)
#   define UTILS_HAS_THREADING 0
#elif defined(__EMSCRIPTEN__)
#   if defined(__EMSCRIPTEN_PTHREADS__) && defined(FILAMENT_WASM_THREADS)
#      define UTILS_HAS_THREADING 1
#   else
#      define UTILS_HAS_THREADING 0
#   endif
#else
#   define UTILS_HAS_THREADING 1
#endif

#if __has_attribute(noinline)
#define UTILS_NOINLINE __attribute__((noinline))
#else
#define UTILS_NOINLINE
#endif

#if __has_attribute(always_inline)
#define UTILS_ALWAYS_INLINE __attribute__((always_inline))
#else
#define UTILS_ALWAYS_INLINE
#endif

#if __has_attribute(pure)
#define UTILS_PURE __attribute__((pure))
#else
#define UTILS_PURE
#endif

#if __has_attribute(maybe_unused) || (defined(_MSC_VER) && _MSC_VER >= 1911)
#define UTILS_UNUSED [[maybe_unused]]
#define UTILS_UNUSED_IN_RELEASE [[maybe_unused]]
#define UTILS_UNUSED_WITHOUT_TRACING [[maybe_unused]]
#elif __has_attribute(unused)
#define UTILS_UNUSED __attribute__((unused))
#define UTILS_UNUSED_IN_RELEASE __attribute__((unused))
#define UTILS_UNUSED_WITHOUT_TRACING __attribute__((unused))
#else
#define UTILS_UNUSED
#define UTILS_UNUSED_IN_RELEASE
#define UTILS_UNUSED_WITHOUT_TRACING
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900
#    define UTILS_RESTRICT __restrict
#elif (defined(__clang__) || defined(__GNUC__))
#    define UTILS_RESTRICT __restrict__
#else
#    define UTILS_RESTRICT
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900
#   define UTILS_HAS_FEATURE_CXX_THREAD_LOCAL 1
#elif __has_feature(cxx_thread_local)
#   define UTILS_HAS_FEATURE_CXX_THREAD_LOCAL 1
#else
#   define UTILS_HAS_FEATURE_CXX_THREAD_LOCAL 0
#endif

#if defined(__clang__)
/**
 * @def UTILS_NONNULL
 * Clang pointer nullability attribute indicating that a pointer or reference cannot be null.
 *
 * APIGen consumes this attribute to synthesize `@NonNull` annotations on generated target
 * language parameters and method return types.
 *
 * @note Enforces compile-time diagnostics under Clang when null pointers are passed.
 */
#define UTILS_NONNULL _Nonnull

/**
 * @def UTILS_NULLABLE
 * Clang pointer nullability attribute indicating that a pointer or reference may be null.
 *
 * APIGen consumes this attribute to synthesize `@Nullable` annotations on generated target
 * language parameters and method return types.
 */
#define UTILS_NULLABLE _Nullable

/**
 * @def UTILS_NOAPIGEN
 * Directs APIGen to exclude the annotated C++ entity from language binding generation.
 *
 * May be applied to classes, structs, member functions, constructors, member fields,
 * type aliases, enums, and enum constants.
 *
 * @invariant The annotated entity produces no target language declarations, JNI bridge
 *            functions, or runtime wrappers.
 *
 * @note Used for internal utility methods, platform-specific helpers, entities with complex
 *       C++ constructs (e.g. unsupported template metaprogramming), or symbols backed by
 *       dedicated handwritten bindings.
 */
#define UTILS_NOAPIGEN [[clang::annotate("filament:apigen:skip")]]

/**
 * @def UTILS_APIGEN_RETAINED
 * Marks an instance getter method whose returned object is retained in a target language field.
 *
 * Applied to getters returning a parent or peer handle (e.g. `MaterialInstance::getMaterial()`,
 * `Renderer::getEngine()`, or `SwapChain::getNativeWindow()`).
 *
 * @pre The referenced object is supplied during construction or factory creation of the receiver.
 * @invariant The generated target language class caches the referenced object in a private final
 *            field initialized during construction.
 * @invariant The getter is served directly from the cached field without bridging across JNI,
 *            preventing temporary wrapper allocation and safeguarding against premature
 *            garbage collection of the native parent/peer object while the child handle remains
 *            reachable.
 */
#define UTILS_APIGEN_RETAINED [[clang::annotate("filament:apigen:retained")]]

/**
 * @def UTILS_APIGEN_FLAGS
 * Designates an enumeration whose entries represent combinable bitwise flags (bitmask).
 *
 * Applied to `enum` or `enum class` declarations where values may be combined with
 * bitwise OR (`|`).
 *
 * @invariant Target language bindings generate methods accepting and returning
 *            `@IntRange(from = 0) int` rather than the type-safe enum class itself, enabling
 *            bitwise operations.
 * @invariant APIGen synthesizes public static final integer constants for all enumerated flag
 *            values.
 */
#define UTILS_APIGEN_FLAGS [[clang::annotate("filament:apigen:flags")]]

/**
 * @def UTILS_APIGEN_ALTERNATE_NAME(name)
 * Overrides the emitted method identifier in target language bindings and JNI bridges.
 *
 * Applied to C++ member functions and static methods.
 *
 * @param name The alternate identifier to use in target language bindings.
 *
 * @invariant APIGen emits target language methods and JNI bridge symbols named @p name while
 *            dispatching directly to the original C++ member in native code.
 * @invariant Resolves naming collisions with target language reserved keywords (e.g. renaming
 *            `package` to `payload` or `import` to `importTexture`).
 * @invariant Disambiguates C++ overloads that collapse into identical signatures in target
 *            languages (e.g. `setBones(..., Bone*)` -> `setBonesAsQuaternions` vs
 *            `setBones(..., mat4f*)` -> `setBonesAsMatrices`).
 */
#define UTILS_APIGEN_ALTERNATE_NAME(name) [[clang::annotate("filament:apigen:alternate_name:" #name)]]

/**
 * @def UTILS_APIGEN_TAGGED_ARRAY
 * Identifies Slice parameters in template setters that collapse into a tagged array family.
 *
 * Applied to Slice arguments in SFINAE-constrained template methods (e.g.
 * `MaterialInstance::setParameter<T>()` and `Material::setDefaultParameter<T>()`).
 *
 * @pre Applied to a `utils::Slice` parameter.
 * @invariant Instructs APIGen to collapse multiple template specializations (`float`,
 *            `int32_t`, `math::float4`, `math::mat4f`, etc.) into a unified typed array
 *            method family taking an element type tag or enum in target languages.
 * @invariant Consolidates JNI bridge functions, dispatching dynamically by element tag rather
 *            than generating redundant native entry points for each template specialization.
 */
#define UTILS_APIGEN_TAGGED_ARRAY [[clang::annotate("filament:apigen:tagged_array")]]

/**
 * @def UTILS_APIGEN_USED_BY_NATIVE
 * Marks a C++ class whose generated target class is accessed via native reflection.
 *
 * Applied to class and struct declarations whose target language counterpart (e.g. Java class)
 * is looked up via JNI reflection by native libraries (e.g. `gltfio`'s `AssetLoader.cpp`).
 *
 * @invariant Instructs APIGen to emit `@UsedByNative` on the generated Java class.
 * @invariant Guarantees that code shrinking, tree-shaking, and obfuscation tools (e.g. ProGuard,
 *            R8) preserve the class, methods, and field names in release builds.
 */
#define UTILS_APIGEN_USED_BY_NATIVE [[clang::annotate("filament:apigen:used_by_native")]]
#else
#define UTILS_NONNULL
#define UTILS_NULLABLE
#define UTILS_NOAPIGEN
#define UTILS_APIGEN_RETAINED
#define UTILS_APIGEN_FLAGS
#define UTILS_APIGEN_ALTERNATE_NAME(name)
#define UTILS_APIGEN_TAGGED_ARRAY
#define UTILS_APIGEN_USED_BY_NATIVE
#endif

#if defined(__clang__) && !defined(SWIG)
// We only enable Clang thread safety annotations if standard std::mutex annotations
// are manually activated via the _LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS define,
// AND multi-threading is enabled (UTILS_HAS_THREADING is not 0).
// This prevents compile failures on single-threaded targets or builds where standard
// annotations are disabled by default in the platform's standard library headers.
#if defined(_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS) && UTILS_HAS_THREADING
#define UTILS_THREAD_ANNOTATION_ATTRIBUTE(x)   __attribute__((x))
#else
#define UTILS_THREAD_ANNOTATION_ATTRIBUTE(x)
#endif
#else
#define UTILS_THREAD_ANNOTATION_ATTRIBUTE(x)
#endif

#define UTILS_CAPABILITY(x) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(capability(x))

#define UTILS_SCOPED_CAPABILITY \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(scoped_lockable)

#define UTILS_GUARDED_BY(x) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(guarded_by(x))

#define UTILS_PT_GUARDED_BY(x) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(pt_guarded_by(x))

#define UTILS_ACQUIRED_BEFORE(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(acquired_before(__VA_ARGS__))

#define UTILS_ACQUIRED_AFTER(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(acquired_after(__VA_ARGS__))

#define UTILS_REQUIRES(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(requires_capability(__VA_ARGS__))

#define UTILS_REQUIRES_SHARED(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(requires_shared_capability(__VA_ARGS__))

#define UTILS_ACQUIRE(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(acquire_capability(__VA_ARGS__))

#define UTILS_ACQUIRE_SHARED(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(acquire_shared_capability(__VA_ARGS__))

#define UTILS_TRY_ACQUIRE(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(try_acquire_capability(__VA_ARGS__))

#define UTILS_TRY_ACQUIRE_SHARED(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(try_acquire_shared_capability(__VA_ARGS__))

#define UTILS_RELEASE(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(release_capability(__VA_ARGS__))

#define UTILS_RELEASE_SHARED(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(release_shared_capability(__VA_ARGS__))

#define UTILS_EXCLUDES(...) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(excludes_capability(__VA_ARGS__))

#define UTILS_RETURN_CAPABILITY(x) \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(lock_returned(x))

#define UTILS_NO_THREAD_SAFETY_ANALYSIS \
    UTILS_THREAD_ANNOTATION_ATTRIBUTE(no_thread_safety_analysis)

#if defined(_MSC_VER)
// MSVC does not support loop unrolling hints
#   define UTILS_UNROLL
#   define UTILS_NOUNROLL
#else
// C++11 allows pragmas to be specified as part of defines using the _Pragma syntax.
#   define UTILS_UNROLL _Pragma("unroll")
#   define UTILS_NOUNROLL _Pragma("nounroll")
#endif

#if __has_feature(cxx_rtti) || defined(_CPPRTTI)
#   define UTILS_HAS_RTTI 1
#else
#   define UTILS_HAS_RTTI 0
#endif

#ifdef __ARM_ACLE
#   include <arm_acle.h>
#   define UTILS_WAIT_FOR_INTERRUPT()   __wfi()
#   define UTILS_WAIT_FOR_EVENT()       __wfe()
#   define UTILS_BROADCAST_EVENT()      __sev()
#   define UTILS_SIGNAL_EVENT()         __sevl()
#   define UTILS_PAUSE()                __yield()
#   define UTILS_PREFETCHW(addr)        __pldx(1, 0, 0, addr)
#else // !__ARM_ACLE
#   if (defined(__i386__) || defined(__x86_64__))
#       define UTILS_X86_PAUSE              {__asm__ __volatile__( "rep; nop" : : : "memory" );}
#       define UTILS_WAIT_FOR_INTERRUPT()   UTILS_X86_PAUSE
#       define UTILS_WAIT_FOR_EVENT()       UTILS_X86_PAUSE
#       define UTILS_BROADCAST_EVENT()
#       define UTILS_SIGNAL_EVENT()
#       define UTILS_PAUSE()                UTILS_X86_PAUSE
#       define UTILS_PREFETCHW(addr)        UTILS_PREFETCH(addr)
#   else // !x86
#       define UTILS_WAIT_FOR_INTERRUPT()
#       define UTILS_WAIT_FOR_EVENT()
#       define UTILS_BROADCAST_EVENT()
#       define UTILS_SIGNAL_EVENT()
#       define UTILS_PAUSE()
#       define UTILS_PREFETCHW(addr)        UTILS_PREFETCH(addr)
#   endif // x86
#endif // __ARM_ACLE


// ssize_t is a POSIX type.
#if defined(WIN32) || defined(_WIN32)
#include <Basetsd.h>
typedef SSIZE_T ssize_t;
#endif

#ifdef _MSC_VER
#   define UTILS_EMPTY_BASES __declspec(empty_bases)
#else
#   define UTILS_EMPTY_BASES
#endif

#if defined(WIN32) || defined(_WIN32)
    #define IMPORTSYMB __declspec(dllimport)
#else
    #define IMPORTSYMB
#endif

#if defined(_MSC_VER) && !defined(__PRETTY_FUNCTION__)
#    define __PRETTY_FUNCTION__ __FUNCSIG__
#endif


#if defined(_MSC_VER)
#   define UTILS_WARNING_PUSH _Pragma("warning( push )")
#   define UTILS_WARNING_POP _Pragma("warning( pop )")
#   define UTILS_WARNING_ENABLE_PADDED _Pragma("warning(1: 4324)")
#elif defined(__clang__)
#   define UTILS_WARNING_PUSH _Pragma("clang diagnostic push")
#   define UTILS_WARNING_POP  _Pragma("clang diagnostic pop")
#   define UTILS_WARNING_ENABLE_PADDED _Pragma("clang diagnostic warning \"-Wpadded\"")
#else
#   define UTILS_WARNING_PUSH
#   define UTILS_WARNING_POP
#   define UTILS_WARNING_ENABLE_PADDED
#endif

#endif // TNT_UTILS_COMPILER_H
