/**
 * Copyright 2025 WebGPU-Native developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file */

/**
 * \mainpage
 *
 * This is a bare minimum extensions file to verify all extension types.
 */

#ifndef WEBGPU_EXTENSION_H_
#define WEBGPU_EXTENSION_H_

#include "webgpu.h"

#if !defined(_wgpu_EXTEND_ENUM)
#ifdef __cplusplus
#define _wgpu_EXTEND_ENUM(E, N, V) static const E N = E(V)
#else
#define _wgpu_EXTEND_ENUM(E, N, V) static const E N = (E)(V)
#endif
#endif // !defined(_wgpu_EXTEND_ENUM)

/**
 * \defgroup Constants Constants
 * \brief Constants.
 *
 * @{
 */

/**
 * 'True' value of @ref WGPUBool.
 *
 * @remark It's not usually necessary to use this, as `true` (from
 * `stdbool.h` or C++) casts to the same value.
 */
#define WGPU_TRUE (UINT32_C(1))
/**
 * 'False' value of @ref WGPUBool.
 *
 * @remark It's not usually necessary to use this, as `false` (from
 * `stdbool.h` or C++) casts to the same value.
 */
#define WGPU_FALSE (UINT32_C(0))
/**
 * New constant should have the namespace prefix in the name by default.
 */
#define WGPU_PREFIX_NEW_CONSTANT1 (UINT32_MAX)
/**
 * New constant can be declared in the 'webgpu' namespace explicitly.
 */
#define WGPU_NEW_CONSTANT2 (UINT64_MAX)
/**
 * The 'compatibility_mode' namespace also has no prefix.
 * (This test is not really related to extensions, it's just easiest to test here.)
 */
#define WGPU_NEW_CONSTANT3 (UINT64_MAX)

/** @} */

/**
 * \defgroup UtilityTypes Utility Types
 *
 * @{
 */

/**
 * New typedef should have the namespace prefix in the name by default.
 */
typedef uint32_t WGPUPrefixNewTypedef1;
/**
 * New typedef can be declared in the 'webgpu' namespace explicitly.
 */
typedef uint64_t WGPUNewTypedef2;

/** @} */

/**
 * \defgroup Objects Objects
 * \brief Opaque, non-dispatchable handles to WebGPU objects.
 *
 * @{
 */
/**
 * New object should have the namespace prefix in the name by default.
 */
typedef struct WGPUPrefixNewObject1Impl* WGPUPrefixNewObject1 WGPU_OBJECT_ATTRIBUTE;
/**
 * New object can be declared in the 'webgpu' namespace explicitly.
 */
typedef struct WGPUNewObject2Impl* WGPUNewObject2 WGPU_OBJECT_ATTRIBUTE;

/** @} */

// Structure forward declarations
struct WGPUPrefixNewStruct1;
struct WGPUNewStruct2;

// Callback info structure forward declarations
struct WGPUPrefixNewCallback1CallbackInfo;
struct WGPUNewCallback2CallbackInfo;

/**
 * \defgroup Enumerations Enumerations
 * \brief Enums.
 *
 * @{
 */

/**
 * Extended enum shouldn't have the namespace prefix in the name by default.
 */
/**
 * New enum entries that extend should have the prefix in the name by default.
 */
_wgpu_EXTEND_ENUM(WGPUOldEnum, WGPUOldEnum_PrefixNewValue, 0x7FFF0000);

/**
 * New enum should have the namespace prefix in the name by default.
 */
typedef enum WGPUPrefixNewEnum1 {
    /**
     * New enum entries for a new enum should not duplicate prefix.
     */
    WGPUPrefixNewEnum1_NewValue = 0x7FFF0000,
    WGPUPrefixNewEnum1_Force32 = 0x7FFFFFFF
} WGPUPrefixNewEnum1 WGPU_ENUM_ATTRIBUTE;

/**
 * New enum can be declared in the 'webgpu' namespace explicitly.
 */
typedef enum WGPUNewEnum2 {
    /**
     * New enum entries for a new enum should not duplicate prefix.
     */
    WGPUNewEnum2_NewValue = 0x7FFF0000,
    WGPUNewEnum2_Force32 = 0x7FFFFFFF
} WGPUNewEnum2 WGPU_ENUM_ATTRIBUTE;

/** @} */

/**
 * \defgroup Bitflags Bitflags
 * \brief Type and constant definitions for bitflag types.
 *
 * @{
 */

/**
 * Extended bitflag shouldn't have the namespace prefix in the name by default.
 *
 * For reserved non-standard bitflag values, see @ref BitflagRegistry.
 */
/**
 * New bitflag entries that extend should have the prefix in the name by default.
 */
static const WGPUOldBitflag WGPUOldBitflag_PrefixNewValue = 0x1000000000000000;

/**
 * New bitflag should have the namespace prefix in the name by default.
 *
 * For reserved non-standard bitflag values, see @ref BitflagRegistry.
 */
typedef WGPUFlags WGPUPrefixNewBitflag1;
/**
 * `0`.
 */
static const WGPUPrefixNewBitflag1 WGPUPrefixNewBitflag1_None = 0x0000000000000000;
/**
 * New bitflag entries shouldn't have the namespace prefix in the name by default.
 */
static const WGPUPrefixNewBitflag1 WGPUPrefixNewBitflag1_NewValue = 0x0000000000000001;

/**
 * New bitflag can be declared in the 'webgpu' namespace explicitly.
 *
 * For reserved non-standard bitflag values, see @ref BitflagRegistry.
 */
typedef WGPUFlags WGPUNewBitflag2;
/**
 * `0`.
 */
static const WGPUNewBitflag2 WGPUNewBitflag2_None = 0x0000000000000000;
/**
 * New bitflag entries shouldn't have the namespace prefix in the name by default.
 */
static const WGPUNewBitflag2 WGPUNewBitflag2_NewValue = 0x0000000000000001;

/** @} */

/**
 * \defgroup Callbacks Callbacks
 * \brief Callbacks through which asynchronous functions return.
 *
 * @{
 */

/**
 * New callback type should have the namespace prefix in the names by default.
 *
 * See also @ref CallbackError.
 *
 * @param message
 * This parameter is @ref PassedWithoutOwnership.
 */
typedef void (*WGPUPrefixNewCallback1Callback)(WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/**
 * New callback type can be declared in the 'webgpu' namespace explicitly.
 *
 * See also @ref CallbackError.
 *
 * @param message
 * This parameter is @ref PassedWithoutOwnership.
 */
typedef void (*WGPUNewCallback2Callback)(WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup Structures Structures
 * \brief Descriptors and other transparent structures.
 *
 * @{
 */

/**
 * \defgroup CallbackInfoStructs Callback Info Structs
 * \brief Callback info structures that are used in asynchronous functions.
 *
 * @{
 */

/**
 * New callback type should have the namespace prefix in the names by default.
 */
typedef struct WGPUPrefixNewCallback1CallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPUPrefixNewCallback1Callback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUPrefixNewCallback1CallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUPrefixNewCallback1CallbackInfo.
 */
#define WGPU_PREFIX_NEW_CALLBACK1_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUPrefixNewCallback1CallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

/**
 * New callback type can be declared in the 'webgpu' namespace explicitly.
 */
typedef struct WGPUNewCallback2CallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPUNewCallback2Callback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUNewCallback2CallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUNewCallback2CallbackInfo.
 */
#define WGPU_NEW_CALLBACK2_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUNewCallback2CallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

/** @} */

/**
 * New struct should have the namespace prefix in the name by default.
 *
 * Default values can be set using @ref WGPU_PREFIX_NEW_STRUCT1_INIT as initializer.
 */
typedef struct WGPUPrefixNewStruct1 {
    /**
     * New struct members should not have namespace prefix in the name.
     *
     * The `INIT` macro sets this to @ref WGPU_PREFIX_NEW_CONSTANT1.
     */
    uint32_t member1;
    /**
     * The `INIT` macro sets this to @ref WGPU_NEW_CONSTANT2.
     */
    uint64_t member2;
} WGPUPrefixNewStruct1 WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUPrefixNewStruct1.
 */
#define WGPU_PREFIX_NEW_STRUCT1_INIT _wgpu_MAKE_INIT_STRUCT(WGPUPrefixNewStruct1, { \
    /*.member1=*/WGPU_PREFIX_NEW_CONSTANT1 _wgpu_COMMA \
    /*.member2=*/WGPU_NEW_CONSTANT2 _wgpu_COMMA \
})

/**
 * New struct can be declared in the 'webgpu' namespace explicitly.
 *
 * Default values can be set using @ref WGPU_NEW_STRUCT2_INIT as initializer.
 */
typedef struct WGPUNewStruct2 {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to @ref WGPU_PREFIX_NEW_STRUCT1_INIT.
     */
    WGPUPrefixNewStruct1 structMember;
} WGPUNewStruct2 WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUNewStruct2.
 */
#define WGPU_NEW_STRUCT2_INIT _wgpu_MAKE_INIT_STRUCT(WGPUNewStruct2, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.structMember=*/WGPU_PREFIX_NEW_STRUCT1_INIT _wgpu_COMMA \
})

/** @} */

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(WGPU_SKIP_PROCS)
// Global procs
/**
 * Proc pointer type for @ref wgpuPrefixNewFunction1:
 * > @copydoc wgpuPrefixNewFunction1
 */
typedef void (*WGPUProcPrefixNewFunction1)() WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuNewFunction2:
 * > @copydoc wgpuNewFunction2
 */
typedef void (*WGPUProcNewFunction2)() WGPU_FUNCTION_ATTRIBUTE;

// Procs of PrefixNewObject1
/**
 * Proc pointer type for @ref wgpuPrefixNewObject1NewMethod:
 * > @copydoc wgpuPrefixNewObject1NewMethod
 */
typedef void (*WGPUProcPrefixNewObject1NewMethod)(WGPUPrefixNewObject1 newObject1) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuPrefixNewObject1AddRef:
 * > @copydoc wgpuPrefixNewObject1AddRef
 */
typedef void (*WGPUProcPrefixNewObject1AddRef)(WGPUPrefixNewObject1 newObject1) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuPrefixNewObject1Release:
 * > @copydoc wgpuPrefixNewObject1Release
 */
typedef void (*WGPUProcPrefixNewObject1Release)(WGPUPrefixNewObject1 newObject1) WGPU_FUNCTION_ATTRIBUTE;

// Procs of NewObject2
/**
 * Proc pointer type for @ref wgpuNewObject2NewMethod:
 * > @copydoc wgpuNewObject2NewMethod
 */
typedef void (*WGPUProcNewObject2NewMethod)(WGPUNewObject2 newObject2) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuNewObject2AddRef:
 * > @copydoc wgpuNewObject2AddRef
 */
typedef void (*WGPUProcNewObject2AddRef)(WGPUNewObject2 newObject2) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuNewObject2Release:
 * > @copydoc wgpuNewObject2Release
 */
typedef void (*WGPUProcNewObject2Release)(WGPUNewObject2 newObject2) WGPU_FUNCTION_ATTRIBUTE;

// Procs of OldObject
/**
 * Proc pointer type for @ref wgpuOldObjectPrefixNewMethod1:
 * > @copydoc wgpuOldObjectPrefixNewMethod1
 */
typedef void (*WGPUProcOldObjectPrefixNewMethod1)(WGPUOldObject oldObject) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuOldObjectNewMethod2:
 * > @copydoc wgpuOldObjectNewMethod2
 */
typedef void (*WGPUProcOldObjectNewMethod2)(WGPUOldObject oldObject) WGPU_FUNCTION_ATTRIBUTE;

#endif  // !defined(WGPU_SKIP_PROCS)

#if !defined(WGPU_SKIP_DECLARATIONS)
/**
 * \defgroup GlobalFunctions Global Functions
 * \brief Functions that are not specific to an object.
 *
 * @{
 */
/**
 * New function should have the namespace prefix in the name by default.
 */
WGPU_EXPORT void wgpuPrefixNewFunction1() WGPU_FUNCTION_ATTRIBUTE;
/**
 * New function can be declared in the 'webgpu' namespace explicitly.
 */
WGPU_EXPORT void wgpuNewFunction2() WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup Methods Methods
 * \brief Functions that are relative to a specific object.
 *
 * @{
 */

/**
 * \defgroup WGPUPrefixNewObject1Methods WGPUPrefixNewObject1 methods
 * \brief Functions whose first argument has type WGPUPrefixNewObject1.
 *
 * @{
 */
/**
 * Method on new object should not have the namespace prefix in the name by default.
 */
WGPU_EXPORT void wgpuPrefixNewObject1NewMethod(WGPUPrefixNewObject1 newObject1) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuPrefixNewObject1AddRef(WGPUPrefixNewObject1 newObject1) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuPrefixNewObject1Release(WGPUPrefixNewObject1 newObject1) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUNewObject2Methods WGPUNewObject2 methods
 * \brief Functions whose first argument has type WGPUNewObject2.
 *
 * @{
 */
/**
 * Method on new object should not have the namespace prefix in the name by default.
 */
WGPU_EXPORT void wgpuNewObject2NewMethod(WGPUNewObject2 newObject2) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuNewObject2AddRef(WGPUNewObject2 newObject2) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuNewObject2Release(WGPUNewObject2 newObject2) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUOldObjectMethods WGPUOldObject methods
 * \brief Functions whose first argument has type WGPUOldObject.
 *
 * @{
 */
/**
 * New method on old object should have the namespace prefix in the name by default.
 */
WGPU_EXPORT void wgpuOldObjectPrefixNewMethod1(WGPUOldObject oldObject) WGPU_FUNCTION_ATTRIBUTE;
/**
 * New method on old object can be declared in the 'webgpu' namespace explicitly.
 */
WGPU_EXPORT void wgpuOldObjectNewMethod2(WGPUOldObject oldObject) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/** @} */

#endif  // !defined(WGPU_SKIP_DECLARATIONS)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // WEBGPU_EXTENSION_H_
