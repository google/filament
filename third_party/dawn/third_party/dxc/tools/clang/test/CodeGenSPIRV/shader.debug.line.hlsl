// RUN: %dxc -T ps_6_0 -E MainPs -fspv-debug=vulkan -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[ext:%[0-9]+]] = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK:      [[src:%[0-9]+]] = OpExtInst %void [[ext]] DebugSource {{%[0-9]+}}

// CHECK:      DebugLine [[src]] %uint_31 %uint_31 %uint_57 %uint_78
// CHECK-NEXT: OpAccessChain %_ptr_Function_v2float %i %int_0

// CHECK:      DebugLine [[src]] %uint_31 %uint_31 %uint_26 %uint_81
// CHECK-NEXT: OpSampledImage
// CHECK-NEXT: OpImageSampleImplicitLod

Texture2D g_tColor;

SamplerState g_sAniso;

struct PS_INPUT
{
    float2 vTextureCoords : TEXCOORD2 ;
} ;

struct PS_OUTPUT
{
    float4 vColor : SV_Target0 ;
} ;

PS_OUTPUT MainPs ( PS_INPUT i )
{
    PS_OUTPUT ps_output ;

    ps_output . vColor = g_tColor . Sample ( g_sAniso , i . vTextureCoords . xy ) ;
    return ps_output ;
}

