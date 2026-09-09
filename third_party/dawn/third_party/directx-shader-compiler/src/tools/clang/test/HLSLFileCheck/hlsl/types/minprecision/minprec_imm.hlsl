// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: storeOutput

float main() : SV_Target
{
    return sqrt(min16float(1.2));
}
