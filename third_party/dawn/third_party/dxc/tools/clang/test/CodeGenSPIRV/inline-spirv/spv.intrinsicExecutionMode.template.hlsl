// RUN: %dxc -T ps_6_0 -E main -spirv -fcgl -Vd  %s -spirv | FileCheck %s

// CHECK: OpExecutionMode %main DenormFlushToZero 32

template<uint width>
struct DenormFlushToZero
{
    static void declare()
    {
        vk::ext_execution_mode(4460, width);
    }
};

void main()
{
    DenormFlushToZero<32>::declare();
}
