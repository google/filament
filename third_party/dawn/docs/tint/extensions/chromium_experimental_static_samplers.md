# Chromium Experimental Static Samplers

The chromium_experimental_static_samplers extension is an experimental extension that allows the use of static samplers in WGSL.
Static samplers translate to immutable samplers in Vulkan, constexpr samplers in Metal Shading Language, and static samplers in Direct3D 12.

# Status

This extension is experimental and the syntax is being discussed. No official WebGPU specification has been written yet.

# Design considerations

## const or var

`@group(X) @binding(Y) const mySampler;`
or
`@group(X) @binding(Y) var mySampler;`


In HLSL and SPIR-V, static samplers need a binding slot, hence the `@group(X) @binding(Y)` syntax.
Vulkan and Direct3D 12 supply static samplers via the API.
Metal Shading Language declares static samplers with `constexpr`, which is ready to be used in the shader without Metal API interaction.

Normal samplers in WGSL are declared with `var` because they have a handle address space, which is read-only.
Using the `const` keyword for static samplers is sensible, as static samplers are immutable objects.
However, `const` does not have a handle address space which is required for HLSL and SPIR-V and would require spec changes to support it.

Using `var` for static samplers is recommended for consistency with the rest of the language and they are already immutable.

## Using a struct

```wgsl
@group(X) @binding(Y) const mySampler = sampler {
  addressModeU: mirror_repeat, // enumerant
  magFilter: linear,
  lodMinClamp: 1, // abstract float
  ...
};
```

The `sampler` is a built-in struct that can be used to define samplers.
Advantages are its readability and similarity to the WebGPU API's `GPUSamplerDescriptor` struct.

This approach would require a new WGSL feature for initializing struct values.

If the new WGSL feature is introduced, this approach is recommended for readability and intuitiveness.
However, for the initial implementation, it is not desirable to introduce a substantial change in WGSL.

## Using a function

```wgsl
const clamp_to_edge = 0;
const mirror_repeat = 1;
...
sampler(mirror_repeat, linear, 0.0, 16);
sampler_comparison(mirror_repeat, linear, 0.0, 16);
```

or

```wgsl
sampler<mirror_repeat, linear>(0.0, 16);
sampler_comparison<mirror_repeat, linear>(0.0, 16);
```

In both cases, `sampler` is a built-in function that returns a static sampler.
The first approach takes a list of arguments. These arguments are predefined constant values and not enums.
The second approach takes a list of enum template parameters and a list of arguments. The arguments are LOD min and max clamps, because they are not enums.

Both approaches are readable and intuitive but due to the large amount of parameters that can be set on a sampler, the approaches may become hard to read and write.
The approaches do not need any new WGSL features and are sufficient for a prototype implementation.

For the prototype implementation, the first function approach is good for its simplicity.

# Pseudo-specification

This extension adds `sampler(...)` and `sampler_comparison(...)` functions to return a sampler type.
The functions take integer values that are predefined constants. If a sampler is initialized with any of the two functions, it is considered a static sampler, otherwise it is a normal sampler.
The predefined constant variable names are similar to the WebGPU API enums.

Function restrictions include:
- The function can only be called to initialize a static sampler.
- Only built-in functions can return a sampler, user-defined functions cannot.
- The function can not be used in an expression, only in a declaration.

Order of arguments:
- addressModeU
- addressModeV
- addressModeW
- magFilter
- minFilter
- mipmapFilter
- compare (only for `sampler_comparison(...)`)
- lodMinClamp
- lodMaxClamp

New Constants:
- Address Mode:
  - WGSL: `clamp_to_edge=0`, `repeat=1`, `mirror_repeat=2`
  - WebGPU equivalent: `"clamp-to-edge"`, `"repeat"`, `"mirror-repeat"`
- Filter Mode:
  - WGSL: `nearest=0`, `linear=1`
  - WebGPU equivalent: `"nearest"`, `"linear"`
- Mipmap Filter Mode:
  - WGSL: `mipmap_nearest=0`, `mipmap_linear=1`
  - WebGPU equivalent: `"nearest"`, `"linear"`
- Comparison Function:
  - WGSL: `never=0`, `less=1`, `equal=2`, `less_equal=3`, `greater=4`, `not_equal=5`, `greater_equal=6`, `always=7`
  - WebGPU equivalent: `"never"`, `"less"`, `"equal"`, `"less-equal"`, `"greater"`, `"not-equal"`, `"greater-equal"`, `"always"`

# Example usage

```wgsl
@group(0) @binding(0) var mySampler = sampler(mirror_repeat, clamp_to_edge, mirror_repeat, linear, linear, mipmap_linear, 0.0, 16.0)

@group(0) @binding(1) var myComparisonSampler = sampler_comparison(mirror_repeat, clamp_to_edge, mirror_repeat, linear, linear, mipmap_linear, less, 0.0, 16.0)

```

