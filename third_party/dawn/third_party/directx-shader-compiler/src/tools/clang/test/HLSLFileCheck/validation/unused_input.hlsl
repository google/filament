// RUN: %dxc -E main -T cs_6_0 -Od %s | FileCheck %s

// Regression test for unused resource being cleaned up in Od build so
// they don't fail validation (for not being mapped in root signature)

// CHECK: @main

RWTexture3D<unorm float> s_uav: register(u0);

struct SOME_CONST_BUFFER { float m_someFloat; };

ConstantBuffer<SOME_CONST_BUFFER> s_constBuffer: register(b0);

float someFunction() { return s_constBuffer.m_someFloat; }

static const float UNUSED_CONSTANT = someFunction();

[RootSignature("DescriptorTable(UAV(u0, numDescriptors = 1, flags = DATA_STATIC_WHILE_SET_AT_EXECUTE), visibility = SHADER_VISIBILITY_ALL)")]
[numthreads(32, 32, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{ 
    s_uav[dispatchThreadID] = 0;
}

