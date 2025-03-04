// RUN: %dxc -T ps_6_0 -E main -spirv -fspv-target-env=vulkan1.2 %s

struct PS_INPUT 
{ 
    bool isFrontFace : SV_IsFrontFace ; 
} ; 

struct PS_OUTPUT 
{ 
    float4 target0 : SV_Target0 ; 
} ; 

PS_OUTPUT main ( PS_INPUT i ) 
{ 
    float4 test = float4(1.0, 1.0, 1.0, 1.0);
    test *= i.isFrontFace ? -1.0 : 1.0;

    PS_OUTPUT o;

    return o ; 
}
