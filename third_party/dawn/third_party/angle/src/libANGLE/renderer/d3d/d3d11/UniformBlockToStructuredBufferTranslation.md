# Uniform Block to StructuredBuffer Translation

## Background
In ANGLE's D3D11 backend, we normally translate GLSL uniform blocks to
HLSL constant buffers. We run into a compile performance issue with
[fxc](https://docs.microsoft.com/en-us/windows/win32/direct3dtools/fxc)
and dynamic constant buffer indexing,
[anglebug.com/40096608](https://bugs.chromium.org/p/angleproject/issues/detail?id=3682)

## Solution
We translate a uniform block into a StructuredBuffer when the following three
conditions are satisfied:
* The uniform block has only one array member, and the array size is larger than or
equal to 50;
* In the shader, all the accesses of the member are through indexing operator;
* The type of the array member must be any of the following:
  * a scalar or vector type.
  * a mat2x4, mat3x4 or mat4x4 matrix type in column major layout, a mat4x2, mat4x2 or
  mat4x3 matrix type in row major layout.
  * a structure type with no array or structure members, where all of the structure's
  fields satisify the prior type conditions.

## Analysis
A typical use case for uniform block to StructuredBuffer translation is for shaders with
one large array member in a uniform block. For example:
```
// GLSL code
uniform buffer {
    TYPE buf [100];
};
```
Will be translated into
```
// HLSL code
StructuredBuffer <TYPETRANSLATED>  bufTranslated: register(tN);
```

However, even with the above limitation, there are still many shaders where we cannot
apply our translation. They are divided into two classes. The first case is when the
shader accesses a "whole entity" uniform block array member element. The second is when
the shader uses the std140 layout.

### Operate uniform block array member as whole entity
According to ESSL spec 3.0, 5.7 Structure and Array Operations,  the following operators
are allowed to operate on arrays as whole entities:

|  Operator Name             |    Operator    |
|  :----------------         |    :------:    |
|  field or method selector  |        .       |
|  assignment                |     ==  !=     |
|  Ternary operator          |       ?:       |
|  Sequence operator         |       ,        |
|  indexing                  |       []       |

However, after translating to StructuredBuffer, the uniform array member cannot be used as
a whole entity since its type has been changed. The member is no longer an array.
After the change, we only support the indexed operation since it is the most common use case.
Other operator usage is unsupported. Example unsupported usages:

| Operator On the Uniform Array Member    |                  examples                        |
|             :------                     |                  :------                         |
| method selector      | buf.length();   // Angle don’t support it, too.                     |
| equality == !=       | TYPE var[NUMBER] = {…}; <br> if (var == buf);                       |
| assignment =         | TYPE var[NUMBER] = {…}; <br> var = buf;                             |
| Ternary operator ?:  | // Angle don’t support it, too.                                     |
| Sequence operator ,  | TYPE var1[NUMBER] = {…}; <br> TYPE var2[NUMBER] = (var1, buf);      |
| Function arguments   | void func(TYPE a[NUMBER]); <br> func(buf);                          |
| Function return type | TYPE[NUMBER] func() { return buf;}  <br> TYPE var[NUMBER] = func(); |

### Std140 limitation
GLSL uniform blocks follow std140 layout packing rules. StructuredBufer has a different set
of packing rules. So we may need to explicitly pad the type `TYPETRANSLATED` to follow std140
rules. Alternately, we can just simply only support those types which don't need to be padded.
These are the supported translation types which do not require translation emulation:

|         GLSL TYPE          |     TRANSLATED HLSL TYPE      |
|         :------            |          :------              |
|   vec4/ivec4/uvec4/bvec4   |     float4/int4/uint4/bool4   |
|   mat2x4 (column_major)    |     float2x4 (row_major)      |
|   mat3x4 (column_major)    |     float3x4 (row_major)      |
|   mat4x4 (column_major)    |     float4x4 (row_major)      |
|   mat4x2 (row_major)       |     float4x3 (column_major)   |
|   mat4x3 (row_major)       |     float4x3 (column_major)   |
|   mat4x4 (row_major)       |     float4x4 (column_major)   |

These are the supported translation types which require some basic translation emulation:

|         GLSL TYPE          |     TRANSLATED HLSL TYPE      |     examples        |
|         :------            |          :------              |     :------         |
|float/int/uint/bool   |float4/int4/uint4/bool4|GLSL: float var = buf[0]; <br> HLSL: float var = buf[0].x;  |
|vec2/ivec2/uvec2/bvec2|float4/int4/uint4/bool4|GLSL: vec2 var = buf[0]; <br> HLSL: float2 var = buf[0].xy; |
|vec3/ivec3/uvec3/bvec3|float4/int4/uint4/bool4|GLSL: vec3 var = buf[0]; <br> HLSL: float3 var = buf[0].xyz;|

These are the unsupported translation types which require more complex translation emulation:

|         GLSL TYPE          |     TRANSLATED HLSL TYPE      |
|         :------            |          :------              |
|   mat2x2 (column_major)    |     float2x4 (row_major)      |
|   mat2x3 (column_major)    |     float2x4 (row_major)      |
|   mat3x2 (column_major)    |     float3x4 (row_major)      |
|   mat3x3 (column_major)    |     float3x4 (row_major)      |
|   mat4x2 (column_major)    |     float4x4 (row_major)      |
|   mat4x3 (column_major)    |     float4x4 (row_major)      |
|   mat2x2 (row_major)       |     float4x2 (column_major)   |
|   mat2x3 (row_major)       |     float4x3 (column_major)   |
|   mat2x4 (row_major)       |     float4x4 (column_major)   |
|   mat3x2 (row_major)       |     float4x2 (column_major)   |
|   mat3x3 (row_major)       |     float4x3 (column_major)   |
|   mat3x4 (row_major)       |     float4x4 (column_major)   |


Take mat3x2(column_major) for an example, the uniform buffer's memory layout is as shown below.

|index|0                      |1                         |...      |
|:--- |      :------          |        :------           |  :---   |
|data |1 2 x x 3 4 x x 5 6 x x|7 8 x x 9 10 x x 11 12 x x|...      |


And the declaration of the uniform block in vertex shader may be as shown below.
```
layout(std140) uniform buffer {
    mat3x2 buf [100];
};

void main(void) {
    ...
    vec2 var = buf[0][2]
    ...
}
```
Will be translated to

```
#pragma pack_matrix(row_major)
StructuredBuffer<float3x4> bufTranslated: register(t0);

float3x2 GetFloat3x2FromFloat3x4Rowmajor(float3x4 mat)
{
    float3x2 res = { 0.0 };
    res[0] = mat[0].xy;
    res[1] = mat[1].xy;
    res[2] = mat[2].xy;
    return res;
}

VS_OUTPUT main(VS_INPUT input) {
    ...
    float3x2 var = GetFloat3x2FromFloat3x4Rowmajor(bufTranslated[0])
}
```

When accessing the element of the `buf` variable, we would need to extract a float3x2 from
a float3x4 for every element.