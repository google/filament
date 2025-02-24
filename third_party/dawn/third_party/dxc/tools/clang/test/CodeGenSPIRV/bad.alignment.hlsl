// RUN: not %dxc %s -HV 2021 -T cs_6_6 -E main -fspv-target-env=vulkan1.3 -fcgl -spirv 2>&1 | FileCheck %s -check-prefix=WARNING
// RUN: %dxc %s -HV 2021 -T cs_6_6 -E main -fspv-target-env=vulkan1.3 -fcgl -spirv -fvk-use-scalar-layout 2>&1 | FileCheck %s -check-prefix=SCALAR

struct complex
{
   float r;
   float i;
};

// When using the default layout, the member should be aligned at a multiple of 16.
// We expect an error
// WARNING: :20:31: warning: The offset provided in the attribute should be 16-byte aligned.

// When using scalar layout, the member should be placed at offset 8.
// SCALAR: OpMemberDecorate %Error 1 Offset 8

struct Error
{
    float2 a;
    [[vk::offset(8)]] complex b;
};

cbuffer Stuff
{
   Error b[10];
};

[numthreads(1, 1, 1)]
void main()
{
   Error testValue = b[0];
}

