# SPIR-V translation of shader input and output variables

WGSL [MR 1315](https://github.com/gpuweb/gpuweb/issues/1315) changed WGSL so
that pipeline inputs and outputs are handled similar to HLSL:

- Shader pipeline inputs are the WGSL entry point function arguments.
- Shader pipeline outputs are the WGSL entry point return value.

Note: In both cases, a struct may be used to pack multiple values together.
In that case, I/O specific attributes appear on struct members at the struct declaration.

Resource variables, e.g. buffers, samplers, and textures, are still declared
as variables at module scope.

## Vulkan SPIR-V today

SPIR-V for Vulkan models inputs and outputs as module-scope variables in
the Input and Output address spaces, respectively.

The `OpEntryPoint` instruction has a list of module-scope variables that must
be a superset of all the input and output variables that are statically
accessed in the shader call tree.
From SPIR-V 1.4 onward, all interface variables that might be statically accessed
must appear on that list.
So that includes all resource variables that might be statically accessed
by the shader call tree.

## Translation scheme for SPIR-V to WGSL

A translation scheme from SPIR-V to WGSL is as follows:

Each SPIR-V entry point maps to a set of Private variables proxying the
inputs and outputs, and two functions:

- An inner function with no arguments or return values, and whose body
  is the same as the original SPIR-V entry point.
- Original input variables are mapped to pseudo-in Private variables
  with the same store types, but no other attributes or properties copied.
  In Vulkan, Input variables don't have initalizers.
- Original output variables are mapped to pseudo-out Private variables
  with the same store types and optional initializer, but no other attributes
  or properties are copied.
- A wrapper entry point function whose arguments correspond in type, location
  and builtin attributes the original input variables, and whose return type is
  a structure containing members correspond in type, location, and builtin
  attributes to the original output variables.
  The body of the wrapper function the following phases:
  - Copy formal parameter values into pseudo-in variables.
    - Insert a bitcast if the WGSL builtin variable has different signedness
      from the SPIR-V declared type.
  - Execute the inner function.
  - Copy pseudo-out variables into the return structure.
    - Insert a bitcast if the WGSL builtin variable has different signedness
      from the SPIR-V declared type.
  - Return the return structure.

- Replace uses of the the original input/output variables to the pseudo-in and
  pseudo-out variables, respectively.
- Remap pointer-to-Input with pointer-to-Private
- Remap pointer-to-Output with pointer-to-Private

We are not concerned with the cost of extra copying input/output values.
First, the pipeline inputs/outputs tend to be small.
Second, we expect the backend compiler in the driver will be able to see
through the copying and optimize the result.

### Example


```glsl
    #version 450

    layout(location = 0) out vec4 frag_colour;
    layout(location = 0) in vec4 the_colour;

    void bar() {
      frag_colour = the_colour;
    }

    void main() {
        bar();
    }
```

Current translation, through SPIR-V, SPIR-V reader, WGSL writer:

```groovy
    @location(0) var<out> frag_colour : vec4<f32>;
    @location(0) var<in> the_colour : vec4<f32>;

    fn bar_() -> void {
      const x_14 : vec4<f32> = the_colour;
      frag_colour = x_14;
      return;
    }

    @fragment
    fn main() -> void {
      bar_();
      return;
    }
```

Proposed translation, through SPIR-V, SPIR-V reader, WGSL writer:

```groovy
    // 'in' variables are now 'private'.
    var<private> frag_colour : vec4<f32>;
    var<private> the_colour : vec4<f32>;

    fn bar_() -> void {
      // Accesses to the module-scope variables do not change.
      // This is a big simplifying advantage.
      const x_14 : vec4<f32> = the_colour;
      frag_colour = x_14;
      return;
    }

    fn main_inner() -> void {
      bar_();
      return;
    }

    // Declare a structure type to collect the return values.
    struct main_result_type {
      @location(0) frag_color : vec4<f32>;
    };

    @fragment
    fn main(

      // 'in' variables are entry point parameters
      @location(0) the_color_arg : vec4<f32>

    ) -> main_result_type {

      // Save 'in' arguments to 'private' variables.
      the_color = the_color_arg;

      // Initialize 'out' variables.
      // Use the zero value, since no initializer was specified.
      frag_color = vec4<f32>();

      // Invoke the original entry point.
      main_inner();

      // Collect outputs into a structure and return it.
      var result : main_outer_result_type;
      result.frag_color = frag_color;
      return result;
    }
```

Alternately, we could emit the body of the original entry point at
the point of invocation.
However that is more complex because the original entry point function
may return from multiple locations, and we would like to have only
a single exit path to construct and return the result value.

### Handling fragment discard

In SPIR-V `OpKill` causes immediate termination of the shader.
Is the shader obligated to write its outputs when `OpKill` is executed?

The Vulkan fragment operations are as follows:
(see [6. Fragment operations](https://www.khronos.org/registry/vulkan/specs/1.2/html/vkspec.html#fragops)).

* Scissor test
* Sample mask test
* Fragment shading
* Multisample coverage
* Depth bounds test
* Stencil test
* Depth test
* Sample counting
* Coverage reduction

After that, the fragment results are used to update output attachments, including
colour, depth, and stencil attachments.

Vulkan says:

> If a fragment operation results in all bits of the coverage mask being 0,
> the fragment is discarded, and no further operations are performed.
> Fragments can also be programmatically discarded in a fragment shader by executing one of
>
>     OpKill.

I interpret this to mean that the outputs of a discarded fragment are ignored.

Therefore, `OpKill` does not require us to modify the basic scheme from the previous
section.

The `OpDemoteToHelperInvocationEXT`
instruction is an alternative way to throw away a fragment, but which
does not immediately terminate execution of the invocation.
It is introduced in the [`SPV_EXT_demote_to_helper_invocation](http://htmlpreview.github.io/?https://github.com/KhronosGroup/SPIRV-Registry/blob/master/extensions/EXT/SPV_EXT_demote_to_helper_invocation.html)
extension.  WGSL does not have this feature, but we expect it will be introduced by a
future WGSL extension.  The same analysis applies to demote-to-helper.  When introduced,
it will not affect translation of pipeline outputs.

### Handling depth-replacing mode

A Vulkan fragment shader must write to the fragment depth builtin if and only if it
has a `DepthReplacing` execution mode. Otherwise behaviour is undefined.

We will ignore the case where the SPIR-V shader writes to the `FragDepth` builtin
and then discards the fragment.
This is justified because "no further operations" are performed by the pipeline
after the fragment is discarded, and that includes writing to depth output attachments.

Assuming the shader is valid, no special translation is required.

### Handling output sample mask

By the same reasoning as for depth-replacing, it is ok to incidentally not write
to the sample-mask builtin variable when the fragment is discarded.

### Handling clip distance and cull distance

Most builtin variables are scalars or vectors.
However, the `ClipDistance` and `CullDistance` builtin variables are arrays of 32-bit float values.
Each entry defines a clip half-plane (respectively cull half-plane)
A Vulkan implementation must support array sizes of up to 8 elements.

How prevalent are shaders that use these features?
These variables are supported when Vulkan features `shaderClipDistance` and `shaderCullDistance`
are supported.
According to gpuinfo.org as of this writing, those
Vulkan features appear to be nearly universally supported on Windows devices (>99%),
but by only 70% on Android.
It appears that Qualcomm devices support them, but Mali devices do not (e.g. Mali-G77).

The proposed translation scheme forces a copy of each array from private
variables into the return value of a vertex shader, or into a private
variable of a fragment shader.
In addition to the register pressure, there may be a performance degradation
due to the bulk copying of data.

We think this is an acceptable tradeoff for the gain in usability and
consistency with other pipeline inputs and outputs.

## Translation scheme for WGSL AST to SPIR-V

To translate from the WGSL AST to SPIR-V, do the following:

- Each entry point formal parameter is mapped to a SPIR-V `Input` variable.
  - Struct and array inputs may have to be broken down into individual variables.
- The return of the entry point is broken down into fields, with one
  `Output` variable per field.
- In the above, builtins must be separated from user attributes.
  - Builtin attributes are moved to the corresponding variable.
  - Location and interpolation attributes are moved to the corresponding
    variables.
- This translation relies on the fact that pipeline inputs and pipeline
  outputs are IO-shareable types. IO-shareable types are always storable,
  and can be the store type of input/output variables.
- Input function parameters will be automatically initialized by the system
  as part of setting up the pipeline inputs to the entry point.
- Replace each return statement in the entry point with a code sequence
  which writes the return value components to the synthesized output variables,
  and then executes an `OpReturn` (without value).

This translation is sufficient even for fragment shaders with discard.
In that case, outputs will be ignored because downstream pipeline
operations will not be performed.
This is the same rationale as for translation from SPIR-V to WGSL AST.
