# Converting SPIR-V to WGSL

This document describes the challenges in converting SPIR-V into WGSL.

Note: Unless otherwise specified, the namespace for C++ code is
`tint::spirv::reader::`.

## Overall flow

1. Validate the SPIR-V input.

   The SPIR-V module (binary blob) is validated against rules for
   Vulkan 1.1, using the SPIRV-Tools validator.

   This allows the rest of the flow to ignore invalid inputs.
   However, the SPIR-V might still be rejected in a later step because:

   - it uses features unavailable in WGSL, or
   - the SPIR-V Reader is insufficiently smart, or
   - the translated program tries to do something rejected by WGSL's rules
     (which are checked by Tint's Resolver).

2. Load the SPIR-V binary into an in-memory representation.

   The SPIR-V reader uses the in-memory representation of the SPIR-V
   module defined by the SPIRV-Tools optimizer.  That provides
   convenient representation of basic structures such as:

    - instructions
    - types
    - constants
    - functions
    - basic blocks

   and provides analyses for:

    - relating definitions to uses (spvtools::opt::analysis::DefUseMgr)
    - types (spvtools::opt::analysis:TypeManager)
    - constants (spvtools::opt::analysis:ConstantManager)

   Note: The SPIR-V is not modified by the SPIR-V Reader.

3. Translate the SPIR-V module into Tint's AST.

   The AST is valid for WGSL except for some small exceptions which are
   cleaned up by transformations.

4. Post-process the AST to make it valid for WGSL.

   Example:
   - Rewrite strided arrays and matrices (remove `@stride` attribute)
   - Rewrite atomic functions
   - Remove unreachable statements, to satisfy WGSL's behaviour analysis.


## Overcoming mismatches between SPIR-V and WGSL

### Remapping builtin inputs and outputs

SPIR-V for Vulkan models builtin inputs and outputs as variables
in Input and Output storage classes.

WGSL builtin inputs are parameters to the entry point, and
builtin outputs are result values of the entry point.

See [spirv-input-output-variables.md](spirv-input-output-variables.md)

### We only care about `gl_Position` from `gl_PerVertex`

Glslang SPIR-V output for a vertex shader has a `gl_PerVertex`
output variable with four members:

- `gl_Position`
- `gl_PointSize`
- `gl_ClipDistance`
- `gl_CullDistance`

WGSL only supports the `position` builtin variable.

The SPIR-V Reader has a bunch of carveouts so it only generates the
position variable. In partcular, it tracks which expressions are actually
accesses into the per-vertex variable, and ignores accesses to other
parts of the structure, and remaps accesses of the position member.

### `gl_PointSize` must be 1.0

It's a WGSL rule.  SPIR-V is more flexible, and the SPIR-V Reader
checks that any assignment to (the equivalent of) `gl_PointSize`
must the constant value 1.0.

### Remapping sample mask inputs and outputs

There's some shenanigans here I don't recall.
See the SkipReason enum.

### Integer signedness

In SPIR-V, the instruction determines the signedness of an operation,
not the types of its operands.

For example:

    %uint = OpTypeInt 32 0 ; u32 type
    %int = OpTypeInt 32 1 ; i32 type

    %int_1 = OpConstant %int 1  ;  WGSL 1i
    %uint_2 = OpConstant %uint 2 ; WGSL 2u

    ; You can mix signs of an operand, and the instruction
    ; tells you the result type.
    %sum_uint = OpIAdd %uint %int %int_1 %uint_2
    %sum_int = OpIAdd %int %int %int_1 %uint_2

However, WGSL arithmetic tends to require the operands and
result type for an operation to all have the same signedness.

So the above might translate to WGSL as:

    let sum_uint: u32 = bitcast<u32>(1i) + 2u;
    let sum_int: i32 = 1i + bitcast<i32>(2u);

See:
* ParserImpl::RectifyOperandSignedness
* ParserImpl::RectifySecondOperandSignedness
* ParserImpl::RectifyForcedResultType

### Translating textures and samplers

SPIR-V textures and samplers are module-scope variables
in UniformConstant storage class.
These map directly to WGSL variables.

For a sampled-image operation, SPIR-V will:
- load the image value from a texture variable
- load the sampler value from a sampler variable
- form a "sampled image" value using `SpvOpSampledImage`
- then use that sampled image value in a image operation
  such as `SpvOpImageSampleImplicitLod`

For an image operation that is not a sampled-image operation
(e.g. OpImageLoad or OpImageWrite), then the steps are similar
except without a sampler (clearly), and without invoking
`OpSampledImage`.

In contrast to the SPIR-V code pattern, the WGSL builtin requires
the texture and sampler value to be passed in as separate parameters.
Secondly, they are passed in by value, by naming the variables
themselves and relying on WGSL's "Load Rule" to pass the handle
value into the callee.

When the SPIR-V Reader translates a texture builtin, it traces
backward through the `OpSampledImage` operation (if any),
back through the load, and all the way back to the `OpVariable`
declaration.  It does this for both the image/texture variable and
the sampler variable (if applicable).  It then uses the names
of those variables as the corresponding arguments to the WGSL
texture builtin.

### Passing textures and samplers into helper functions

Glslang generates SPIR-V where texture and sampler formal parameters
are as pointer-to-UniformConstant.

WGSL models them as passing texture and sampler values themselves,
conceptually as opaque handles.  This is similar to GLSL, but unlike
SPIR-V.

To support textures and samplers as arguments to user-defined functions,
we extend the tracing logic so it knows to bottom out at OpFunctionParameter.

Also, code that generates function declarations now understands formal
parameters declared as a pointer to uniform-constant as
well as direct image and sampler values.

Example GLSL compute shader:

    #version 450

    layout(set=0,binding=0) uniform texture2D im;
    layout(set=0,binding=1) uniform sampler s;

    vec4 helper(texture2D imparam, sampler sparam) {
      return texture(sampler2D(imparam,sparam),vec2(0));
    }

    void main() {
      vec4 v = helper(im,s);
    }

SPIR-V generated by Glslang (Shaderc's glslc):

    ; SPIR-V
    ; Version: 1.0
    ; Generator: Google Shaderc over Glslang; 10
    ; Bound: 32
    ; Schema: 0
                   OpCapability Shader
              %1 = OpExtInstImport "GLSL.std.450"
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint GLCompute %main "main"
                   OpExecutionMode %main LocalSize 1 1 1
                   OpSource GLSL 450
                   OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
                   OpSourceExtension "GL_GOOGLE_include_directive"
                   OpName %main "main"
                   OpName %helper_t21_p1_ "helper(t21;p1;"
                   OpName %imparam "imparam"
                   OpName %sparam "sparam"
                   OpName %v "v"
                   OpName %im "im"
                   OpName %s "s"
                   OpDecorate %im DescriptorSet 0
                   OpDecorate %im Binding 0
                   OpDecorate %s DescriptorSet 0
                   OpDecorate %s Binding 1
           %void = OpTypeVoid
              %3 = OpTypeFunction %void
          %float = OpTypeFloat 32
              %7 = OpTypeImage %float 2D 0 0 0 1 Unknown
    %_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
              %9 = OpTypeSampler
    %_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
        %v4float = OpTypeVector %float 4
             %12 = OpTypeFunction %v4float %_ptr_UniformConstant_7 %_ptr_UniformConstant_9
             %19 = OpTypeSampledImage %7
        %v2float = OpTypeVector %float 2
        %float_0 = OpConstant %float 0
             %23 = OpConstantComposite %v2float %float_0 %float_0
    %_ptr_Function_v4float = OpTypePointer Function %v4float
             %im = OpVariable %_ptr_UniformConstant_7 UniformConstant
              %s = OpVariable %_ptr_UniformConstant_9 UniformConstant
           %main = OpFunction %void None %3
              %5 = OpLabel
              %v = OpVariable %_ptr_Function_v4float Function
             %31 = OpFunctionCall %v4float %helper_t21_p1_ %im %s
                   OpStore %v %31
                   OpReturn
                   OpFunctionEnd
    %helper_t21_p1_ = OpFunction %v4float None %12
        %imparam = OpFunctionParameter %_ptr_UniformConstant_7
         %sparam = OpFunctionParameter %_ptr_UniformConstant_9
             %16 = OpLabel
             %17 = OpLoad %7 %imparam
             %18 = OpLoad %9 %sparam
             %20 = OpSampledImage %19 %17 %18
             %24 = OpImageSampleExplicitLod %v4float %20 %23 Lod %float_0
                   OpReturnValue %24
                   OpFunctionEnd

What the SPIR-V Reader currently generates:

    @group(0) @binding(0) var im : texture_2d<f32>;

    @group(0) @binding(1) var s : sampler;

    fn helper_t21_p1_(imparam : texture_2d<f32>, sparam : sampler) -> vec4<f32> {
      let x_24 : vec4<f32> = textureSampleLevel(imparam, sparam, vec2<f32>(0.0f, 0.0f), 0.0f);
      return x_24;
    }

    fn main_1() {
      var v : vec4<f32>;
      let x_31 : vec4<f32> = helper_t21_p1_(im, s);
      v = x_31;
      return;
    }

    @compute @workgroup_size(1i, 1i, 1i)
    fn main() {
      main_1();
    }

### Dimensionality mismatch in texture builtins

Vulkan SPIR-V is fairly forgiving in the dimensionality
of input coordinates and result values of texturing operations.
There is some localized rewriting of values to satisfy the overloads
of WGSL's texture builtin functions.

### Reconstructing structured control flow

This is subtle.

- Use structural dominance (but we didn't have the name at the time).
  See SPIR-V 1.6 Rev 2 for updated definitions.
- See the big comment at the start of reader/spirv/function.cc
- See internal presentations.

Basically:
* Compute a "structured order" for structurally reachable basic blocks.
* Traversing in structured order, use a stack-based algorithn to
  identify intervals of blocks corresponding to structured constructs.
  For example, loop construct, continue construct, if-selection,
  switch-selection, and case-construct. Constructs can be nested,
  hence the need for a stack.  This is akin to "drawing braces"
  around statements, to form block-statements that will appear in
  the output. This step performs some validation, which may now be
  redundant with the SPIRV-Tools validator. This is defensive
  programming, and some tests skip use of the SPIRV-Tools validator.
* Traversing in structured order, identify structured exits from the
  constructs identified in the previous step. This determines what
  control flow edges correspond to `break`, `continue`, and `return`,
  as needed.
* Traversing in structured order, generate statements for instructions.
  This uses a stack corresponding to nested constructs. The kind of
  each construct being entered or exited determines emission of control
  flow constructs (WGSL's `if`, `loop`, `continuing`, `switch`, `case`).

### Preserving execution order

An instruction inside a SPIR-V instruction is one of:

- control flow: see the previous section
- combinatorial: think of this as an ALU operation, i.e. the effect
  is purely to evaluate a result value from the values of its operands.
  It has no side effects, and is not affected by external state such
  as memory or the actions of other invocations in its subgroup.
  Examples:  arithmetic, OpCopyObject
- interacts with memory or other invocations in some way.
  Examples: load, store, atomics, barriers, (subgroup operations when we
  get them)
- function calls: functions are not analyzed to see if they are pure,
  so we assume function calls are non-combinatorial.

To preserve execution order, all non-combinatorial instructions must
be translated as their own separate statement.  For example, an OpStore
maps to an assignment statement.

However, combinatorial instructions can be emitted at any point
in evaluation, provided data flow constraints are satisfied: input
values are available, and such that the resulting value is generated
in time for consumption by downstream uses.

The SPIR-V Reader uses a heuristic to choose when to emit combinatorial
values:
- if a combinatorial expression only has one use, *and*
- its use is in the same structured construct as its definition, *then*
- emit the expression at the place where it is consumed.

Otherwise, make a `let` declaration for the value.

Why:
- If a value has many uses, then computing it once can save effort.
  Preserve that choice if it was made by an upstream optimizing compiler.
- If a value is consumed in a different structured construct, then the
  site of its consumption may be inside a loop, and we don't want to
  sink the computation into the loop, thereby causing spurious extra
  evaluation.

This heuristic generates halfway-readable code, greatly reducing the
varbosity of code in the common case.

### Hoisting and phis

SPIR-V uses SSA (static single assignment).  The key requirement is
that the definition of a value must dominate its uses.

WGSL uses lexical scoping.

It is easy enough for a human or an optimizing compiler to generate
SSA cases which do not map cleanly to a lexically scoped value.

Example pseudo-GLSL:

    void main() {
      if (cond) {
        const uint x = 1;
      } else {
        return;
      }
      const uint y = x;  // x's definition dominates this use.
    }

This isn't valid GLSL and its analog would not be a valid WGSL
program because x is used outside the scope of its declaration.

Additionally, SSA uses `phi` nodes to transmit values from predecessor
basic blocks that would otherwise not be visible (because the
parent does not dominate the consuming basic block).  An example
is sending the updated value of a loop induction variable back to
the top of the loop.

The SPIR-V reader handles these cases by tracking:
- where a value definition occurs
- the span of basic blocks, in structured order, where there
  are uses of the value.

If the uses of a value span structured contructs which are not
contained by the construct containing the definition (or
if the value is a `phi` node), then we "hoist" the value
into a variable:

- create a function-scope variable at the top of the structured
  construct that spans all the uses, so that all the uses
  are in scope of that variable declaration.

- for a non-phi: generate an assignment to that variable corresponding
  to the value definition in the original SPIR-V.

- for a phi: generate an assigment to that variable at the end of
  each predecessor block for that phi, assigning the value to be
  transmitted from that phi.

This scheme works for values which can be the stored in a variable.

It does not work for pointers. However, we don't think we need
to solve this case any time soon as it is uncommon or hard/impossible
to generate via standard tooling.
See https://crbug.com/tint/98 and https://crbug.com/tint/837

## Mapping types

SPIR-V has a recursive type system. Types are defined, given result IDs,
before any functions are defined, and before any constant values using
the corresponding types.

WGSL also has a recursive type system. However, except for structure types,
types are spelled inline at their uses.

## Texture and sampler types

SPIR-V image types map to WGSL types, but the WGSL type is determined
more by usage (what operations are performed on it) than by declaration.

For example, Vulkan ignores the "Depth" operand of the image type
declaration (OpTypeImage).
See [16.1 Image Operations Overview](https://registry.khronos.org/vulkan/specs/1.3/html/vkspec.html#_image_operations_overview).
Instead, we must infer that a texture is a depth texture because
it is used by image instructions using a depth-reference, e.g.
OpImageSampleDrefImplicitLod vs. OpImageSampleImplicitLod.

Similarly, SPIR-V only has one sampler type.  The use of the
sampler determines whether it maps to a WGSL `sampler` or
`sampler_comparison` (for depth sampling).

The SPIR-V Reader scans uses of each texture and sampler
in the module to infer the appropriate target WGSL type.
See ParserImpl::RegisterHandleUsage

In Vulkan SPIR-V it is possible to use the same sampler for regular
sampling and depth-reference sampling.  In this case the SPIR-V Reader
will infer a depth texture, but then the generated program will fail WGSL
validation.

For example, this GLSL fragment shader:

    #version 450

    layout(set=1,binding=0) uniform texture2D tInput;
    layout(set=1,binding=1) uniform sampler s;

    void main() {
      vec4 v = texture(sampler2D(tInput,s),vec2(0));
      float f = texture(sampler2DShadow(tInput,s),vec3(0));
    }

Converts to this WGSL shader:

    @group(1) @binding(0) var tInput : texture_depth_2d;

    @group(1) @binding(1) var s : sampler_comparison;

    fn main_1() {
      var v : vec4<f32>;
      var f : f32;
      let x_23 : vec4<f32> = vec4<f32>(textureSample(tInput, s, vec2<f32>(0.0f, 0.0f)), 0.0f, 0.0f, 0.0f);
      v = x_23;
      let x_34 : f32 = textureSampleCompare(tInput, s, vec3<f32>(0.0f, 0.0f, 0.0f).xy, vec3<f32>(0.0f, 0.0f, 0.0f).z);
      f = x_34;
      return;
    }

    @fragment
    fn main() {
      main_1();
    }

But then this fails validation:

    error: no matching call to textureSample(texture_depth_2d, sampler_comparison, vec2<f32>)
    15 candidate functions: ...

## References and pointers

SPIR-V has a pointer type.

A SPIR-V pointer type corresponds to a WGSL memory view. WGSL has two
memory view types: a reference type, and a pointer type.

See [spirv-ptr-ref.md](spirv-ptr-ref.md) for details on the translation.

## Mapping buffer types

Vulkan SPIR-V expresses a Uniform Buffer Object (UBO), or
a WGSL 'uniform buffer' as:

- an OpVariable in Uniform storage class
- its pointee type (store type) is a Block-decorated structure type

Vulkan SPIR-V has two ways to express a Shader Storage Buffer Object (SSBO),
or a WGSL 'storage buffer' as either deprecated-style:

- an OpVariable in Uniform storage class
- its pointee type (store type) is a BufferBlock-decorated structure type

or as new-style:

- an OpVariable in StorageBuffer storage class
- its pointee type (store type) is a Block-decorated structure type

Deprecated-style storage buffer was the only option in un-extended
Vulkan 1.0. It is generated by tools that want to generate code for
the broadest reach.  This includes DXC.

New-style storage buffer requires the use of the `OpExtension
"SPV_KHR_storage_buffer_storage_class"` or SPIR-V 1.3 or later
(Vulkan 1.1 or later).

Additionally, a storage buffer in SPIR-V may be marked as NonWritable.
Perhaps surprisingly, this is typically done by marking *all* the
members of the top-level (Buffer)Block-decorated structure as NonWritable.
(This is the common translation because that's what Glslang does.)

Translation of uniform buffers is straightforward.

However, the SPIR-V Reader must support both the deprecated and the new
styles of storage buffers.

Additionally:
- a storage buffer with all NonWritable members is translated with `read`
  access mode. This becomes a part of its WGSL reference type (and hence
  corresponding pointer type).
- a storage buffer without all NonWritable members is translated with
  an explicit `read_write` access mode. This becomes a part of its
  WGSL reference type (and hence corresponding pointer type).

Note that read-only vs. read-write is a property of the pointee-type in SPIR-V,
but in WGSL it's part of the reference type (not the store type).

To handle this mismatch, the SPIR-V Reader has bookkeeping to map
each pointer value (inside a function) back to through to the originating
variable. This originating variable may be a buffer variable which then
tells us which address space and access mode to use for a locally-defined
pointer value.

Since baseline SPIR-V does not allow passing pointers to buffers into
user-defined helper functions, we don't need to handle this buffer type
remapping into function formal parameters.

## Mapping OpArrayLength

The OpArrayLength instruction takes a pointer to the enclosing
structure (the pointee type of the storage buffer variable).

But the WGSL arrayLength builtin variable takes a pointer to the
member inside that structure.

A small local adjustment is sufficient here.
