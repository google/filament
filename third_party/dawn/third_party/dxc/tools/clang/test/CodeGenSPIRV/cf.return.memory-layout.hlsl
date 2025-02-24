// RUN: %dxc -T cs_6_5 -E main -fvk-use-gl-layout -Zpr -WX -enable-16bit-types -Zi -fcgl  %s -spirv | FileCheck %s

struct Data
{
    uint a;
    uint b;
    uint c;
    uint d;
};

RWStructuredBuffer<Data> buffer;


// CHECK:     OpName [[Data:%[a-zA-Z0-9_]+]] "Data"
// CHECK-NOT: OpMemberDecorate [[Data]] 0 Offset 0
// CHECK:     OpName [[Data_0:%[a-zA-Z0-9_]+]] "Data"
// CHECK-NOT: OpMemberDecorate [[Data_0]] 0 Offset 0
// CHECK:     OpMemberDecorate [[Data]] 0 Offset 0
// CHECK:     OpMemberDecorate [[Data]] 1 Offset 4
// CHECK:     OpMemberDecorate [[Data]] 2 Offset 8
// CHECK:     OpMemberDecorate [[Data]] 3 Offset 12
// CHECK:     [[Data]] = OpTypeStruct %uint %uint %uint %uint
// CHECK:     [[Data_0]] = OpTypeStruct %uint %uint %uint %uint

Data returnDataWithoutPhysicalMemoryLayout(uint idx)
{
// CHECK: [[comp:%[0-9]+]] = OpLoad [[Data]]
// CHECK: [[a:%[0-9]+]] = OpCompositeExtract %uint [[comp]] 0
// CHECK: [[b:%[0-9]+]] = OpCompositeExtract %uint [[comp]] 1
// CHECK: [[c:%[0-9]+]] = OpCompositeExtract %uint [[comp]] 2
// CHECK: [[d:%[0-9]+]] = OpCompositeExtract %uint [[comp]] 3
// CHECK: OpCompositeConstruct [[Data_0]] [[a]] [[b]] [[c]] [[d]]
    return buffer[idx];
}

[numthreads(1, 1, 1)]
void main(uint3 groupThreadID : SV_GroupThreadID)
{
    Data foo = returnDataWithoutPhysicalMemoryLayout(groupThreadID.x);
}
