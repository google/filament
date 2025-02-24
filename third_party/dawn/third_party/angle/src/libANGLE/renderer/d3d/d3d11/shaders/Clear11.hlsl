//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Clear11.hlsl: Shaders for clearing RTVs and DSVs using draw calls and
// specifying float depth values and either float, uint or sint clear colors.
// Notes:
//  - UINT & SINT clears can only be compiled with FL10+
//  - VS_Clear_FL9 requires a VB to be bound with vertices to create
//    a primitive covering the entire surface (in clip co-ordinates)

// Constants
static const float2 g_Corners[6] =
{
    float2(-1.0f,  1.0f),
    float2( 1.0f, -1.0f),
    float2(-1.0f, -1.0f),
    float2(-1.0f,  1.0f),
    float2( 1.0f,  1.0f),
    float2( 1.0f, -1.0f),
};

// Vertex Shaders
void VS_Clear(in uint id : SV_VertexID,
              out float4 outPosition : SV_POSITION)
{
    float2 corner = g_Corners[id];
    outPosition = float4(corner.x, corner.y, 0.0f, 1.0f);
}

void VS_Multiview_Clear(in uint id : SV_VertexID,
                        in uint instanceID : SV_InstanceID,
                        out float4 outPosition : SV_POSITION,
                        out uint outLayerID : TEXCOORD0)
{
    float2 corner = g_Corners[id];
    outPosition = float4(corner.x, corner.y, 0.0f, 1.0f);
    outLayerID = instanceID;
}

// Geometry shader for clearing multiview layered textures
struct GS_INPUT
{
    float4 inPosition : SV_Position;
    uint inLayerID : TEXCOORD0;
};

struct GS_OUTPUT
{
    float4 outPosition : SV_Position;
    uint outLayerID : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void GS_Multiview_Clear(triangle GS_INPUT input[3], inout TriangleStream<GS_OUTPUT> outStream)
{
    GS_OUTPUT output = (GS_OUTPUT)0;
    for (int i = 0; i < 3; i++)
    {
        output.outPosition = input[i].inPosition;
        output.outLayerID = input[i].inLayerID;
        outStream.Append(output);
    }
    outStream.RestartStrip();
}

// Pixel Shader Constant Buffers
cbuffer ColorAndDepthDataFloat : register(b0)
{
    float4 color_Float   : packoffset(c0);
    float  zValueF_Float : packoffset(c1);
}

cbuffer ColorAndDepthDataSint : register(b0)
{
    int4  color_Sint   : packoffset(c0);
    float zValueF_Sint : packoffset(c1);
}

cbuffer ColorAndDepthDataUint : register(b0)
{
    uint4 color_Uint   : packoffset(c0);
    float zValueF_Uint : packoffset(c1);
}

cbuffer DepthOnlyData : register(b0)
{
    float zValue_Depth : packoffset(c1);
}

// Pixel Shader Output Structs
struct PS_OutputFloat1
{
    float4 color0 : SV_TARGET0;
    float  depth  : SV_DEPTH;
};

struct PS_OutputFloat2
{
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float  depth  : SV_DEPTH;
};

struct PS_OutputFloat3
{
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float4 color2 : SV_TARGET2;
    float  depth  : SV_DEPTH;
};

struct PS_OutputFloat4
{
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float4 color2 : SV_TARGET2;
    float4 color3 : SV_TARGET3;
    float  depth  : SV_DEPTH;
};

struct PS_OutputFloat5
{
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float4 color2 : SV_TARGET2;
    float4 color3 : SV_TARGET3;
    float4 color4 : SV_TARGET4;
    float  depth  : SV_DEPTH;
};

struct PS_OutputFloat6
{
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float4 color2 : SV_TARGET2;
    float4 color3 : SV_TARGET3;
    float4 color4 : SV_TARGET4;
    float4 color5 : SV_TARGET5;
    float  depth  : SV_DEPTH;
};

struct PS_OutputFloat7
{
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float4 color2 : SV_TARGET2;
    float4 color3 : SV_TARGET3;
    float4 color4 : SV_TARGET4;
    float4 color5 : SV_TARGET5;
    float4 color6 : SV_TARGET6;
    float  depth  : SV_DEPTH;
};

struct PS_OutputFloat8
{
    float4 color0 : SV_TARGET0;
    float4 color1 : SV_TARGET1;
    float4 color2 : SV_TARGET2;
    float4 color3 : SV_TARGET3;
    float4 color4 : SV_TARGET4;
    float4 color5 : SV_TARGET5;
    float4 color6 : SV_TARGET6;
    float4 color7 : SV_TARGET7;
    float  depth  : SV_DEPTH;
};

struct PS_OutputUint1
{
    uint4 color0 : SV_TARGET0;
    float depth  : SV_DEPTH;
};

struct PS_OutputUint2
{
    uint4 color0 : SV_TARGET0;
    uint4 color1 : SV_TARGET1;
    float depth  : SV_DEPTH;
};

struct PS_OutputUint3
{
    uint4 color0 : SV_TARGET0;
    uint4 color1 : SV_TARGET1;
    uint4 color2 : SV_TARGET2;
    float depth  : SV_DEPTH;
};

struct PS_OutputUint4
{
    uint4 color0 : SV_TARGET0;
    uint4 color1 : SV_TARGET1;
    uint4 color2 : SV_TARGET2;
    uint4 color3 : SV_TARGET3;
    float depth  : SV_DEPTH;
};

struct PS_OutputUint5
{
    uint4 color0 : SV_TARGET0;
    uint4 color1 : SV_TARGET1;
    uint4 color2 : SV_TARGET2;
    uint4 color3 : SV_TARGET3;
    uint4 color4 : SV_TARGET4;
    float depth  : SV_DEPTH;
};

struct PS_OutputUint6
{
    uint4 color0 : SV_TARGET0;
    uint4 color1 : SV_TARGET1;
    uint4 color2 : SV_TARGET2;
    uint4 color3 : SV_TARGET3;
    uint4 color4 : SV_TARGET4;
    uint4 color5 : SV_TARGET5;
    float depth  : SV_DEPTH;
};

struct PS_OutputUint7
{
    uint4 color0 : SV_TARGET0;
    uint4 color1 : SV_TARGET1;
    uint4 color2 : SV_TARGET2;
    uint4 color3 : SV_TARGET3;
    uint4 color4 : SV_TARGET4;
    uint4 color5 : SV_TARGET5;
    uint4 color6 : SV_TARGET6;
    float depth  : SV_DEPTH;
};

struct PS_OutputUint8
{
    uint4 color0 : SV_TARGET0;
    uint4 color1 : SV_TARGET1;
    uint4 color2 : SV_TARGET2;
    uint4 color3 : SV_TARGET3;
    uint4 color4 : SV_TARGET4;
    uint4 color5 : SV_TARGET5;
    uint4 color6 : SV_TARGET6;
    uint4 color7 : SV_TARGET7;
    float depth  : SV_DEPTH;
};

struct PS_OutputSint1
{
    int4 color0 : SV_TARGET0;
    float depth : SV_DEPTH;
};

struct PS_OutputSint2
{
    int4 color0 : SV_TARGET0;
    int4 color1 : SV_TARGET1;
    float depth : SV_DEPTH;
};

struct PS_OutputSint3
{
    int4 color0 : SV_TARGET0;
    int4 color1 : SV_TARGET1;
    int4 color2 : SV_TARGET2;
    float depth : SV_DEPTH;
};

struct PS_OutputSint4
{
    int4 color0 : SV_TARGET0;
    int4 color1 : SV_TARGET1;
    int4 color2 : SV_TARGET2;
    int4 color3 : SV_TARGET3;
    float depth : SV_DEPTH;
};

struct PS_OutputSint5
{
    int4 color0 : SV_TARGET0;
    int4 color1 : SV_TARGET1;
    int4 color2 : SV_TARGET2;
    int4 color3 : SV_TARGET3;
    int4 color4 : SV_TARGET4;
    float depth : SV_DEPTH;
};

struct PS_OutputSint6
{
    int4 color0 : SV_TARGET0;
    int4 color1 : SV_TARGET1;
    int4 color2 : SV_TARGET2;
    int4 color3 : SV_TARGET3;
    int4 color4 : SV_TARGET4;
    int4 color5 : SV_TARGET5;
    float depth : SV_DEPTH;
};

struct PS_OutputSint7
{
    int4 color0 : SV_TARGET0;
    int4 color1 : SV_TARGET1;
    int4 color2 : SV_TARGET2;
    int4 color3 : SV_TARGET3;
    int4 color4 : SV_TARGET4;
    int4 color5 : SV_TARGET5;
    int4 color6 : SV_TARGET6;
    float depth : SV_DEPTH;
};

struct PS_OutputSint8
{
    int4 color0 : SV_TARGET0;
    int4 color1 : SV_TARGET1;
    int4 color2 : SV_TARGET2;
    int4 color3 : SV_TARGET3;
    int4 color4 : SV_TARGET4;
    int4 color5 : SV_TARGET5;
    int4 color6 : SV_TARGET6;
    int4 color7 : SV_TARGET7;
    float depth : SV_DEPTH;
};

struct PS_OutputDepth
{
    float depth : SV_DEPTH;
};

// Pixel Shaders
PS_OutputFloat1 PS_ClearFloat1(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat1 outData;
    outData.color0 = color_Float;
    outData.depth  = zValueF_Float;
    return outData;
}

PS_OutputFloat2 PS_ClearFloat2(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat2 outData;
    outData.color0 = color_Float;
    outData.color1 = color_Float;
    outData.depth  = zValueF_Float;
    return outData;
}

PS_OutputFloat3 PS_ClearFloat3(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat3 outData;
    outData.color0 = color_Float;
    outData.color1 = color_Float;
    outData.color2 = color_Float;
    outData.depth  = zValueF_Float;
    return outData;
}

PS_OutputFloat4 PS_ClearFloat4(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat4 outData;
    outData.color0 = color_Float;
    outData.color1 = color_Float;
    outData.color2 = color_Float;
    outData.color3 = color_Float;
    outData.depth  = zValueF_Float;
    return outData;
}

PS_OutputFloat5 PS_ClearFloat5(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat5 outData;
    outData.color0 = color_Float;
    outData.color1 = color_Float;
    outData.color2 = color_Float;
    outData.color3 = color_Float;
    outData.color4 = color_Float;
    outData.depth  = zValueF_Float;
    return outData;
}

PS_OutputFloat6 PS_ClearFloat6(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat6 outData;
    outData.color0 = color_Float;
    outData.color1 = color_Float;
    outData.color2 = color_Float;
    outData.color3 = color_Float;
    outData.color4 = color_Float;
    outData.color5 = color_Float;
    outData.depth  = zValueF_Float;
    return outData;
}

PS_OutputFloat7 PS_ClearFloat7(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat7 outData;
    outData.color0 = color_Float;
    outData.color1 = color_Float;
    outData.color2 = color_Float;
    outData.color3 = color_Float;
    outData.color4 = color_Float;
    outData.color5 = color_Float;
    outData.color6 = color_Float;
    outData.depth  = zValueF_Float;
    return outData;
}

PS_OutputFloat8 PS_ClearFloat8(in float4 inPosition : SV_POSITION)
{
    PS_OutputFloat8 outData;
    outData.color0 = color_Float;
    outData.color1 = color_Float;
    outData.color2 = color_Float;
    outData.color3 = color_Float;
    outData.color4 = color_Float;
    outData.color5 = color_Float;
    outData.color6 = color_Float;
    outData.color7 = color_Float;
    outData.depth  = zValueF_Float;
    return outData;
}

PS_OutputUint1 PS_ClearUint1(in float4 inPosition : SV_POSITION)
{
    PS_OutputUint1 outData;
    outData.color0 = color_Uint;
    outData.depth = zValueF_Uint;
    return outData;
}

PS_OutputUint2 PS_ClearUint2(in float4 inPosition : SV_POSITION)
{
    PS_OutputUint2 outData;
    outData.color0 = color_Uint;
    outData.color1 = color_Uint;
    outData.depth = zValueF_Uint;
    return outData;
}

PS_OutputUint3 PS_ClearUint3(in float4 inPosition : SV_POSITION)
{
    PS_OutputUint3 outData;
    outData.color0 = color_Uint;
    outData.color1 = color_Uint;
    outData.color2 = color_Uint;
    outData.depth = zValueF_Uint;
    return outData;
}

PS_OutputUint4 PS_ClearUint4(in float4 inPosition : SV_POSITION)
{
    PS_OutputUint4 outData;
    outData.color0 = color_Uint;
    outData.color1 = color_Uint;
    outData.color2 = color_Uint;
    outData.color3 = color_Uint;
    outData.depth = zValueF_Uint;
    return outData;
}

PS_OutputUint5 PS_ClearUint5(in float4 inPosition : SV_POSITION)
{
    PS_OutputUint5 outData;
    outData.color0 = color_Uint;
    outData.color1 = color_Uint;
    outData.color2 = color_Uint;
    outData.color3 = color_Uint;
    outData.color4 = color_Uint;
    outData.depth = zValueF_Uint;
    return outData;
}

PS_OutputUint6 PS_ClearUint6(in float4 inPosition : SV_POSITION)
{
    PS_OutputUint6 outData;
    outData.color0 = color_Uint;
    outData.color1 = color_Uint;
    outData.color2 = color_Uint;
    outData.color3 = color_Uint;
    outData.color4 = color_Uint;
    outData.color5 = color_Uint;
    outData.depth = zValueF_Uint;
    return outData;
}

PS_OutputUint7 PS_ClearUint7(in float4 inPosition : SV_POSITION)
{
    PS_OutputUint7 outData;
    outData.color0 = color_Uint;
    outData.color1 = color_Uint;
    outData.color2 = color_Uint;
    outData.color3 = color_Uint;
    outData.color4 = color_Uint;
    outData.color5 = color_Uint;
    outData.color6 = color_Uint;
    outData.depth = zValueF_Uint;
    return outData;
}

PS_OutputUint8 PS_ClearUint8(in float4 inPosition : SV_POSITION)
{
    PS_OutputUint8 outData;
    outData.color0 = color_Uint;
    outData.color1 = color_Uint;
    outData.color2 = color_Uint;
    outData.color3 = color_Uint;
    outData.color4 = color_Uint;
    outData.color5 = color_Uint;
    outData.color6 = color_Uint;
    outData.color7 = color_Uint;
    outData.depth = zValueF_Uint;
    return outData;
}

PS_OutputSint1 PS_ClearSint1(in float4 inPosition : SV_POSITION)
{
    PS_OutputSint1 outData;
    outData.color0 = color_Sint;
    outData.depth = zValueF_Sint;
    return outData;
}

PS_OutputSint2 PS_ClearSint2(in float4 inPosition : SV_POSITION)
{
    PS_OutputSint2 outData;
    outData.color0 = color_Sint;
    outData.color1 = color_Sint;
    outData.depth = zValueF_Sint;
    return outData;
}

PS_OutputSint3 PS_ClearSint3(in float4 inPosition : SV_POSITION)
{
    PS_OutputSint3 outData;
    outData.color0 = color_Sint;
    outData.color1 = color_Sint;
    outData.color2 = color_Sint;
    outData.depth = zValueF_Sint;
    return outData;
}

PS_OutputSint4 PS_ClearSint4(in float4 inPosition : SV_POSITION)
{
    PS_OutputSint4 outData;
    outData.color0 = color_Sint;
    outData.color1 = color_Sint;
    outData.color2 = color_Sint;
    outData.color3 = color_Sint;
    outData.depth = zValueF_Sint;
    return outData;
}

PS_OutputSint5 PS_ClearSint5(in float4 inPosition : SV_POSITION)
{
    PS_OutputSint5 outData;
    outData.color0 = color_Sint;
    outData.color1 = color_Sint;
    outData.color2 = color_Sint;
    outData.color3 = color_Sint;
    outData.color4 = color_Sint;
    outData.depth = zValueF_Sint;
    return outData;
}

PS_OutputSint6 PS_ClearSint6(in float4 inPosition : SV_POSITION)
{
    PS_OutputSint6 outData;
    outData.color0 = color_Sint;
    outData.color1 = color_Sint;
    outData.color2 = color_Sint;
    outData.color3 = color_Sint;
    outData.color4 = color_Sint;
    outData.color5 = color_Sint;
    outData.depth = zValueF_Sint;
    return outData;
}

PS_OutputSint7 PS_ClearSint7(in float4 inPosition : SV_POSITION)
{
    PS_OutputSint7 outData;
    outData.color0 = color_Sint;
    outData.color1 = color_Sint;
    outData.color2 = color_Sint;
    outData.color3 = color_Sint;
    outData.color4 = color_Sint;
    outData.color5 = color_Sint;
    outData.color6 = color_Sint;
    outData.depth = zValueF_Sint;
    return outData;
}

PS_OutputSint8 PS_ClearSint8(in float4 inPosition : SV_POSITION)
{
    PS_OutputSint8 outData;
    outData.color0 = color_Sint;
    outData.color1 = color_Sint;
    outData.color2 = color_Sint;
    outData.color3 = color_Sint;
    outData.color4 = color_Sint;
    outData.color5 = color_Sint;
    outData.color6 = color_Sint;
    outData.color7 = color_Sint;
    outData.depth = zValueF_Sint;
    return outData;
}

PS_OutputDepth PS_ClearDepth(in float4 inPosition : SV_POSITION)
{
    PS_OutputDepth outData;
    outData.depth = zValue_Depth;
    return outData;
}
