// RUN: %dxc -T cs_6_7 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: %bool_type = OpTypeStruct
template<bool val>
struct bool_type
{
    static const bool value = val;
};

template<>
struct bool_type<false>
{
    static const bool value = false;
};

[numthreads(1, 1, 1)]
void main(uint3 invocationID : SV_DispatchThreadID)
{
// CHECK: OpStore %truthVal %true
    bool_type<true> trueType;
    bool truthVal = trueType.value;

// CHECK: OpStore %falseVal %false
    bool_type<false> falseType;
    bool falseVal = falseType.value;
}
