# Translations

This document attempts to document how WGSL translates into the various backends
for the cases where the translation is not a direct mapping.

# Access Control

## HLSL
 * ReadOnly -> `ByteAddressBuffer`
 * ReadWrite -> `RWByteAddressBuffer`

## MSL
 * ReadOnly -> `const`

## SPIR-V
There are two ways this can be achieved in SPIR-V. Either the variable can be
decorated with `NonWritable` or each member of the struct can be decorated with
`NonWritable`. We chose to go the struct member route.
 * The read-only becomes part of the type in this case. Otherwise, you are
   treating the readonly type information as part of the variable which is
   confusing.
 * Treating the readonly as part of the variable means we should be
   deduplicating the types behind the access control, which causes confusing
   with the type_names and various tracking systems within Tint.


# Builtin Decorations
| Name | SPIR-V | MSL | HLSL |
|------|--------|-----|------|
| position | SpvBuiltInPosition |position | SV_Position |
| vertex_index | SpvBuiltInVertexIndex |vertex_id | SV_VertexID |
| instance_index | SpvBuiltInInstanceIndex | instance_id| SV_InstanceID |
| front_facing | SpvBuiltInFrontFacing | front_facing | SV_IsFrontFacing |
| frag_coord | SpvBuiltInFragCoord | position | SV_Position |
| frag_depth | SpvBuiltInFragDepth | depth(any) | SV_Depth |
| local_invocation_id | SpvBuiltInLocalInvocationId | thread_position_in_threadgroup | SV_GroupThreadID |
| local_invocation_index | SpvBuiltInLocalInvocationIndex | thread_index_in_threadgroup | SV_GroupIndex |
| global_invocation_id | SpvBuiltInGlobalInvocationId | thread_position_in_grid | SV_DispatchThreadID |


# Builtins Methods
| Name | SPIR-V | MSL | HLSL |
| ------|--------|-----|------ |
| abs | GLSLstd450FAbs or GLSLstd450SAbs| fabs or abs | abs |
| acos | GLSLstd450Acos | acos | acos |
| all | SpvOpAll | all | all |
| any | SpvOpAny | any | any |
| arrayLength | SpvOpArrayLength | | |
| asin | GLSLstd450Asin | asin | asin |
| atan | GLSLstd450Atan | atan | atan |
| atan2 | GLSLstd450Atan2| atan2 | atan2 |
| ceil | GLSLstd450Ceil| ceil | ceil |
| clamp | GLSLstd450NClamp or GLSLstd450UClamp or GLSLstd450SClamp| clamp | clamp |
| cos | GLSLstd450Cos | cos | cos |
| cosh | GLSLstd450Cosh | cosh | cosh |
| countOneBits | SpvOpBitCount | popcount | countbits |
| cross | GLSLstd450Cross | cross | cross |
| determinant | GLSLstd450Determinant | determinant | determinant |
| distance | GLSLstd450Distance | distance | distance |
| dot | SpOpDot | dot | dot |
| dpdx | SpvOpDPdx | dpdx | ddx |
| dpdxCoarse | SpvOpDPdxCoarse | dpdx | ddx_coarse |
| dpdxFine | SpvOpDPdxFine | dpdx | ddx_fine |
| dpdy | SpvOpDPdy | dpdy | ddy |
| dpdyCoarse | SpvOpDPdyCoarse | dpdy | ddy_coarse |
| dpdyFine | SpvOpDPdyFine | dpdy | ddy_fine |
| exp | GLSLstd450Exp | exp |  exp |
| exp2 | GLSLstd450Exp2 | exp2 | exp2 |
| faceForward | GLSLstd450FaceForward | faceforward | faceforward |
| floor | GLSLstd450Floor | floor | floor |
| fma | GLSLstd450Fma | fma | fma |
| fract | GLSLstd450Fract | fract | frac |
| frexp | GLSLstd450Frexp | | |
| fwidth | SpvOpFwidth | fwidth | fwidth |
| fwidthCoarse | SpvOpFwidthCoarse | fwidth | fwidth |
| fwidthFine | SpvOpFwidthFine | fwidth | fwidth |
| inverseSqrt | GLSLstd450InverseSqrt | rsqrt | rsqrt |
| ldexp | GLSLstd450Ldexp | | |
| length | GLSLstd450Length | length | length |
| log | GLSLstd450Log | log | log |
| log2 | GLSLstd450Log2 | log2 | log2 |
| max | GLSLstd450NMax or GLSLstd450SMax or GLSLstd450UMax | fmax or max | max |
| min | GLSLstd450NMin or GLSLstd450SMin or GLSLstd450UMin | fmin or min | min |
| mix | GLSLstd450FMix | mix | mix |
| modf | GLSLstd450Modf | | |
| normalize | GLSLstd450Normalize | normalize | normalize |
| pow | GLSLstd450Pow | pow | pow |
| reflect | GLSLstd450Reflect | reflect | reflect |
| reverseBits | SpvOpBitReverse | reverse_bits | reversebits |
| round | GLSLstd450Round | round | round |
| select | SpvOpSelect | select | |
| sign | GLSLstd450FSign | sign | sign |
| sin | GLSLstd450Sin | sin | sin |
| sinh | GLSLstd450Sinh | sinh | sinh |
| smoothstep | GLSLstd450SmoothStep | smoothstep | smoothstep |
| sqrt | GLSLstd450Sqrt | sqrt | sqrt |
| step | GLSLstd450Step | step | step |
| tan | GLSLstd450Tan | tan | tan |
| tanh | GLSLstd450Tanh | tanh | tanh |
| trunc | GLSLstd450Trunc | trunc | trunc |

# Types
## Sampler Types
| WGSL | SPIR-V | MSL | HLSL |
|------|--------|-----|------|
| sampler | OpTypeSampler | sampler | SamplerState |
| sampler_comparison | OpTypeSampler | sampler | SamplerComparisonState |

## Texture Types
| WGSL | SPIR-V | MSL | HLSL |
|------|--------|-----|------|
| texture_1d&lt;type&gt; | OpTypeImage 1D Sampled=1 | texture1d&lt;type, access::sample&gt; | Texture1D |
| texture_2d&lt;type&gt; | OpTypeImage 2D Sampled=1 | texture2d&lt;type, access::sample&gt; | Texture2D |
| texture_2d_array&lt;type&gt; | OpTypeImage 2D Arrayed=1 Sampled=1 | texture2d_array&lt;type, access::sample&gt; | Texture2DArray |
| texture_3d&lt;type&gt; | OpTypeImage 3D Sampled=1 | texture3d&lt;type, access::sample&gt; | Texture3D |
| texture_cube&lt;type&gt; | OpTypeImage Cube Sampled=1 | texturecube&lt;type, access::sample&gt; | TextureCube |
| texture_cube_array&lt;type&gt; | OpTypeImage Cube Arrayed=1 Sampled=1 | texturecube_array&lt;type, access::sample&gt; | TextureCubeArray |
| | | |
| texture_multisampled_2d&lt;type&gt; | OpTypeImage 2D MS=1 Sampled=1 | texture2d_ms&lt;type, access::sample&gt; | Texture2D |
| | | |
| texture_depth_2d | OpTypeImage 2D Sampled=1 | depth2d&lt;float, access::sample&gt;| Texture2D |
| texture_depth_2d_array | OpTypeImage 2D Arrayed=1 Sampled=1 | depth2d_array&lt;float, access::sample&gt; | Texture2DArray |
| texture_depth_cube | OpTypeImage Cube Sampled=1 | depthcube&lt;float, access::sample&gt; | TextureCube |
| texture_depth_cube_array | OpTypeImage Cube Arrayed=1 Sampled=1 | depthcube_array&lt;float, access::sample&gt; | TextureCubeArray |
| texture_depth_multisampled_2d | OpTypeImage 2D MS=1 Sampled=1 | depth2d&lt;float, access::sample&gt;| Texture2DMSArray |
| | | |
| texture_storage_1d&lt;image_storage_type&gt; | OpTypeImage 1D Sampled=2| texture1d&lt;type, access::read&gt; | RWTexture1D |
| texture_storage_2d&lt;image_storage_type&gt; | OpTypeImage 2D Sampled=2 | texture2d&lt;type, access::read&gt; | RWTexture2D |
| texture_storage_2d_array&lt;image_storage_type&gt; | OpTypeImage 2D Arrayed=1 Sampled=2 | texture2d_array&lt;type, access::read&gt; | RWTexture2DArray |
| texture_storage_3d&lt;image_storage_type&gt; | OpTypeImage 3D Sampled=2 | texture3d&lt;type, access::read&gt; | RWTexture3D |
| | | |
| texture_storage_1d&lt;image_storage_type&gt; | OpTypeImage 1D Sampled=2 | texture1d&lt;type, access::write&gt; | RWTexture1D |
| texture_storage_2d&lt;image_storage_type&gt; | OpTypeImage 2D Sampled=1 | texture2d&lt;type, access::write&gt; | RWTexture2D |
| texture_storage_2d_array&lt;image_storage_type&gt; | OpTypeImage 2D Arrayed=1 Sampled=2 | texture2d_array&lt;type, access::write&gt; | RWTexture2DArray |
| texture_storage_3d&lt;image_storage_type&gt; | OpTypeImage 3D Sampled=2 | texture3d&lt;type, access::write&gt; | RWTexture3D|

# Short-circuting
## HLSL
TODO(dsinclair): Nested if's

## SPIR-V
TODO(dsinclair): Nested if's

# Address spaces
TODO(dsinclair): do ...

# Storage buffers
## HLSL
TODO(dsinclair): Rewriting of accessors to loads

# Loop blocks
## HLSL
TODO(dsinclair): Rewrite with bools

## MSL
TODO(dsinclair): Rewrite with bools

# Input / Output address spaces
## HLSL
TODO(dsinclair): Structs and params

## MSL
TODO(dsinclair): Structs and params

# Discard
## HLSL
 * `discard`

## MSL
 * `discard_fragment()`


# Specialization constants
## HLSL
```
#ifndef WGSL_SPEC_CONSTANT_<id>
-- if default provided
#define WGSL_SPEC_CONSTANT_<id> default value
-- else
#error spec constant required for constant id
--
#endif
static const <type> <name> = WGSL_SPEC_CONSTANT_<id>
```

## MSL
`@function_constant(<id>)`
