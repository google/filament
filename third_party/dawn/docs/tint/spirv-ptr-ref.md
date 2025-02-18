# SPIR-V translation of WGSL pointers and references

WGSL was updated to have two kinds of memory views: pointers and references.
See https://github.com/gpuweb/gpuweb/pull/1569

In summary:

* Reference types are never explicitly mentioned in WGSL source.
* A use of a variable is a value of reference type corresponding
  to the reference memory view of the storage allocated for the
  variable.
* Let-declared constants can be of pointer type, but not reference
  type.
* Function parameter can be of pointer type, but not reference type.
* A variable's store type is never a pointer type, and never a
  reference type.
* The "Load Rule" allows a reference to decay to the underlying
  store type, by issuing a load of the value in the underlying memory.
* For an assignment:
  * The right-hand side evaluates to a non-reference type (atomic-free
    plain type).
  * The left-hand side evaluates to a reference type, whose store
    type is the same as the result of evaluating the right hand side.
* The address-of (unary `&`) operator converts a reference to a
  pointer.
* The dereference (unary `*`) operator converts a pointer to a
  reference.

TODO: Passing textures and samplers to helper functions might be
done by "handler value", or by pointer-to-handle.

## Writing SPIR-V from WGSL

The distinction in WGSL between reference and pointer disappears
at the SPIR-V level.  Both types map into pointer types in SPIR-V.

To translate a valid WGSL program to SPIR-V:

* The dereference operator (unary `*`) is the identity operation.
* The address-of operator (unary `&`) is the identity operation.
* Assignment maps to OpStore.
* The Load Rule translates to OpLoad.

## Reading SPIR-V to create WGSL

The main changes to the SPIR-V reader are:

* When translating a SPIR-V pointer expression, track whether the
  corresponding WGSL expression is of corresponding WGSL pointer
  type or correspoinding WGSL type.
* Insert dereference (unary-`*`) or address-of (unary-`&`) operators
  as needed to generate valid WGSL expressions.

The choices can be made deterministic, as described below.

The SPIR-V reader only supports baseline functionality in Vulkan.
Therefore we assume no VariablePointers or VariablePointersStorageBuffer
capabilities.  All pointers are
[SPIR-V logical pointers](https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#LogicalPointerType).
The [SPIR-V Universal Validation Rules](https://www.khronos.org/registry/spir-v/specs/unified1/SPIRV.html#_universal_validation_rules)
specify where logical pointers can appear as results of instructions
or operands of instructions.

Each SPIR-V pointer result expression is a logical pointer, and
therefore is one of:

* OpVariable: map to the reference type.
* OpFunctionParameter: map to the pointer type.
* OpCopyObject:
   * When these only have one use, then these often fold away.
     Otherwise, they map to a a let-declared constant.
   * Map to the pointer type.
* OpAccessChain, OpInBoundsAccessChain:
   * This could map to either pointer or reference, and adjustments
     in other areas could make it work.  However, we recommend mapping
     this to the reference type.
* OpImageTexelPointer is not supported in WGSL.
   It is used to get a pointer into a storage texture, for use with
   atomic instructions.  But image atomics is not supported in
   WebGPU/WGSL.

Each SPIR-V pointer operand is also a logical pointer, and is an
operand to one of:
* OpLoad Pointer operand:
   * Map to reference, inserting a dereference operator if needed.
* OpStore Pointer operand:
   * Map to reference, inserting a dereference operator if needed.
* OpStore Pointer operand:
* OpAccessChain, OpInBoundsAccessChain Base operand:
   * WGSL array-access and subfield access only works on references.
      * [Gpuweb issue 1530](https://github.com/gpuweb/gpuweb/issues/1530)
        is filed to allow those operations to work on pointers.
   * Map to reference, inserting a dereference operator if needed.
* OpFunctionCall function argument pointer operands
   * Function operands can't be references.
   * Map to pointer, inserting an address-of operator if needed.
* OpAtomic instruction Pointer operand
   * These map to WGSL atomic builtins.
   * Map to pointer, inserting an address-of operator if needed.
   * Note: As of this writing, the atomic instructions are not supported
     by the SPIR-V reader.
* OpCopyObject source operand
   * This could have been mapped either way, but it's easiest to
     map to pointer, to match the choice for OpCopyObject result type.
   * Map to pointer, inserting an address-of operator if needed.
* OpCopyMemory, source and destination operands
   * This acts as an assignment.
   * Map both source and destination to reference, inserting dereference
     operators if needed.
   * Note: As of this writing, OpCopyMemory is not supported by the
     SPIR-V reader.
* Extended instruction set instructions Modf and Frexp
   * These map to builtins.
   * Map the pointer operand to pointer, inserting an address-of
     operator if needed.
