# Struct-Chaining {#StructChaining}

Struct-chaining is a C API pattern using linked lists and dynamic typing to extend existing structs with new members, while maintaining API and ABI compatibility. For example:

An extensible struct is statically typed. It is the root of a linked list, containing a pointer to the next struct in the chain:

```c
typedef struct WGPUMyStructBase {
    WGPUChainedStruct * nextInChain;
    uint32_t x;
} WGPUMyBaseStruct;
```

Each extension struct is a "subtype" of @ref WGPUChainedStruct; that is, its first member is a @ref WGPUChainedStruct, so that it can be safely cast to @ref WGPUChainedStruct. This allows the implementation to read its @ref WGPUChainedStruct::sType, which is some value of @ref WGPUSType ("Struct Type") that dynamically identifies the struct's type. (The `sType` may come from an implementation-specific extension; @ref WGPUSType is an "open" enum.)

Once the implementation identifies the struct by its `sType`, it casts the pointer back to the appropriate struct type in order to access its contents. Setting `sType` incorrectly (or pointing to any type that isn't a subtype of @ref WGPUChainedStruct) causes undefined behavior.

```c
typedef enum WGPUSType {
    // ...
    WGPUSType_MyStructExtension1 = /* ... */,
    WGPUSType_MyStructExtension2 = /* ... */,
    // ...
} WGPUStype;

typedef struct WGPUMyStructExtension1 {
    WGPUChainedStruct chain; // .chain.sType must be WGPUSType_MyStructExtension1
    uint32_t y;
} WGPUMyStructExtension1;

typedef struct WGPUMyStructExtension2 {
    WGPUChainedStruct chain; // .chain.sType must be WGPUSType_MyStructExtension2
    uint32_t z;
} WGPUMyStructExtension2;
```

This is used like so:

```c
WGPUMyStructExtension2 ext2 = WGPU_MY_STRUCT_EXTENSION2_INIT;
// .chain.sType is already set correctly by the INIT macro.
// .chain.next is set to NULL indicating the end of the chain.
ext2.z = 2;

WGPUMyStructExtension1 ext1 = WGPU_MY_STRUCT_EXTENSION1_INIT;
// .chain.sType is already set correctly by the INIT macro.
// .chain.next may be set in either of two ways, equivalently:
ext1.chain.next = &ext2.chain;
ext1.chain.next = (WGPUChainedStruct*) &ext2;
ext1.y = 1;

WGPUMyStructBase base = WGPU_MY_STRUCT_BASE_INIT;
// Note: base structs do not have STypes (they are statically typed).
base.nextInChain = &ext1.chain;
base.x = 0;
```

The pointer links in a struct chain are all mutable pointers. Whether the structs in the chain are actually mutated depends on the function they are passed to (whether the struct chain is passed as an input or an output). Some structs (e.g. @ref WGPULimits) may be either an input or an output depending on the function being called.

## Struct-Chaining Error {#StructChainingError}

A struct-chaining error occurs if a struct chain is incorrectly constructed (in a detectable way).
They occur if and only if:

- The `sType` of a struct in the chain is not valid _in the context of the chain root's static type_.
    - If this happens, the implementation must not downcast the pointer to access the rest of the struct (even if it would know how to downcast it in other contexts).
- Multiple instances of the same `sType` value are seen in the same struct chain. (Note this also detects and disallows cycles.)
    - Implementation-specific extensions also _should_ avoid designs that use unbounded recursion (such as linked lists) in favor of iterative designs (arrays or arrays-of-pointers). This is to avoid stack overflows in struct handling and serialization/deserialization.

Struct chains which are used in device-timeline validation/operations (e.g. @ref WGPUBufferDescriptor in @ref wgpuDeviceCreateBuffer) have their chain errors surfaced asynchronously, like any other validation error.

Struct chains which are used in content-timeline operations (e.g. @ref OutStructChainError) have their chain errors surfaced synchronously, like other content-timeline validation errors.

### Out-Struct-Chain Error {#OutStructChainError}

Operations which take out-struct-chains (e.g. @ref WGPULimits, in @ref wgpuAdapterGetLimits and @ref wgpuDeviceGetLimits, but not in @ref WGPUDeviceDescriptor) handle struct-chaining errors as follows:

- The output struct and struct chain is not modified.
- The operation produces a @ref SynchronousError (return value and log message).

## Ownership

`FreeMembers` functions do not traverse the [struct chain](@ref StructChaining) and must be called separately on each struct (that has a `FreeMembers` function) in the chain.
See @ref ReturnedWithOwnership.

## Callbacks

See @ref PassedWithoutOwnership.
