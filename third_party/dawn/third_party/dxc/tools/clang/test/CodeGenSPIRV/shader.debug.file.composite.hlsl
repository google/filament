// RUN: %dxc -E MainPs -T ps_6_0 -fspv-target-env=vulkan1.1 -fspv-debug=vulkan-with-source -fcgl  %s -spirv | FileCheck %s

// Just check that compilation completes
// CHECK: %MainPs = OpFunction %void None

#line 43 "lpv_debug.fxc"
struct PS_INPUT
{

    float3 vPositionWithOffsetWs : TEXCOORD0 ;

} ;

#line 430 "csgo_common_ps_code.fxc"
struct PS_OUTPUT
{
    float4 vColor : SV_Target0 ;
} ;

#line 135 "lpv_debug_grid.vfx"
PS_OUTPUT MainPs ( PS_INPUT i )
{
    float3 v = i . vPositionWithOffsetWs ;

    PS_OUTPUT o ;
    o . vColor . rgba = float4 ( 0.0 , 0.0 , 0.0 , 0.0 ) ;
    return o ;

}

