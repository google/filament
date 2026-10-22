# Extensions {#Extensions}

`webgpu.h` is designed to be an extensible and forward-compatible API.
The following types of extensions are supported:

```
wgpuPrefixNewFunction

WGPUPrefixNewObject
wgpuPrefixNewObjectNewMethod
wgpuOldObjectPrefixNewMethod

WGPUPrefixNewEnum
WGPUPrefixNewEnum_NewValue
WGPUOldEnum_PrefixNewValue

WGPUPrefixNewStruct

WGPUPrefixNewBitflagType
WGPUPrefixNewBitflagType_NewValue
WGPUOldBitflagType_PrefixNewValue

WGPUPrefixNewCallback

WGPU_PREFIX_NEW_CONSTANT
```

("Prefix" is the name of the implementation that owns the extension, if any; see below.)

When an application is running against an unknown `webgpu.h` implementation, extension support may be detected at runtime as follows:

- New functions/methods may be runtime-detected by loading them dynamically, and checking whether loading succeeds. (@ref wgpuGetProcAddress() returns `NULL` for unknown function names.)
    - New objects may be detected by the presence of the methods that create them.
    - New (root) structs, enum/bitflag types, and callback types are always supported if the methods that accept them exist.
- New enum/bitflag values and [chained structs](@ref StructChaining) are available iff the corresponding "feature" was already explicitly enabled for the context where they're used:
    - Device features are detected via @ref wgpuAdapterGetFeatures() and enabled via @ref WGPUDeviceDescriptor::requiredFeatures.
    - Instance features are detected via @ref wgpuHasInstanceFeature() and @ref wgpuGetInstanceFeatures(), and enabled via @ref WGPUInstanceDescriptor::requiredFeatures.
    - Instance limits are detected via @ref wgpuGetInstanceLimits(), and enabled via @ref WGPUInstanceDescriptor::requiredLimits.

The following design principles should be followed to ensure future extensibility:

- Enums always have a `Force32 = 0x7FFFFFFF` value to force them to be 32-bit (and have a stable ABI representation).
- Bitflag types are always 64-bit.
- Structures should be extensible (have a `nextInChain`), or at least be associated with some struct (e.g. child, sibling, or parent) that is extensible.

Note also:

- Whenever necessary a version `2` or implementation-specific version of an existing method or type can be added.

## Registry of prefixes and enum blocks

Implementation-specific extensions **should** use the naming conventions listed above, with the name prefixes listed here.

Implementation-specific extensions **must** use their assigned block when adding new values to existing enum types. (Implementation-specific enum types do not need to use these blocks since they are exclusive to one implementation.)

If an implementation does not have an assigned prefix and block, it **should** be added to this registry.

|                      | Prefix       | Enum Block    | Description
|----------------------|--------------|---------------|------------
| Standard             | (none)       | `0x0000_????` | Extensions standardized in webgpu.h
| *(Reserved)*         | -            | `0x0001_????` | Reserved for future use
| *(Not used)*         | -            | `0x0002_????` | Do not use this block (historical)
| wgpu-native          | `Wgpu`       | `0x0003_????` | -
| Emscripten           | `Emscripten` | `0x0004_????` | -
| Dawn                 | `Dawn`       | `0x0005_????` | -
| Wagyu                | `Wagyu`      | `0x0006_????` | -

Note all negative values (values with the most-significant bit set to 1) are reserved for future use.

## Bitflag Registry {#BitflagRegistry}

Implementation-specific extensions **must** choose one of the following options when adding new bitflag values:
- Register their reserved bitflag values in this document.
- Add a new bitflag type, and use it via an extension struct.

Core and Compatibility Mode bits will always be in the least-significant 53 bits, because the JS API can only represent 53 bits.
Therefore, extended bitflag values **should** be in the most-significant 11 bits, overflowing into the most-significant end of the least-significant 53 bits if necessary (or avoiding doing so by adding a new bitflag type entirely).

- (None have been registered yet!)
