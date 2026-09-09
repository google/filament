// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: define void @main()

float3x3 main(float3 normal : IN) : OUT
{
    precise float3x3 ret; // <---- precise
    ret[0] = normal;
    ret[1] = normal;
    ret[2] = normal;
    return ret;
}
