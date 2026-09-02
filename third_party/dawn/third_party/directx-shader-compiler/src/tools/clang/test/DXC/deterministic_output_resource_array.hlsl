// Test that compiling a shader with resource arrays produces deterministic output.
// This is a regression test for https://github.com/microsoft/DirectXShaderCompiler/issues/8171
// where compiling the same shader multiple times with -Zi and SM 6.6+ produced
// non-identical disassembly due to non-deterministic handle name suffixes.

// RUN: %dxc -T cs_6_6 -E CSMain -Zi -Fc %t1 %s
// RUN: %dxc -T cs_6_6 -E CSMain -Zi -Fc %t2 %s
// RUN: diff %t1 %t2

Texture2D<float4> inMaps[16];
RWTexture2D<float4> Output;

[numthreads(1,1,1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
    Output[threadID.xy] = inMaps[0][threadID.xy];
    Output[threadID.xy] = inMaps[1][threadID.xy];
    Output[threadID.xy] = inMaps[2][threadID.xy];
    Output[threadID.xy] = inMaps[3][threadID.xy];
    Output[threadID.xy] = inMaps[4][threadID.xy];
    Output[threadID.xy] = inMaps[5][threadID.xy];
    Output[threadID.xy] = inMaps[6][threadID.xy];
}
