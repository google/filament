// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

[[vk::ext_instruction(/* OpLoad */ 61)]]
float4 load([[vk::ext_reference]] float4 pointer, [[vk::ext_literal]] int memoryOperands);

[[vk::ext_instruction(/* OpStore */ 62)]]
void store([[vk::ext_reference]] float4 pointer, float4 object, [[vk::ext_literal]] int memoryOperands);

struct SInstanceData {
  float4 Output;
};

struct VS_INPUT	{
  float4 Position : POSITION;
};

float4 main(const VS_INPUT v) : SV_Position {
    SInstanceData kk;
    kk.Output = v.Position;
    float4 output = float4(0.0, 0.0, 0.0, 1.0);
// CHECK: {{%[0-9]+}} = OpLoad %v4float {{%[0-9]+}} None
// CHECK: OpStore %output {{%[0-9]+}} Volatile    
    store(output, load(kk.Output, 0x0), 1);
    return output;
}

