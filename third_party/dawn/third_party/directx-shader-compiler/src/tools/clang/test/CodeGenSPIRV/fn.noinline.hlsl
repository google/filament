// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

[noinline]
float4 foo()
{
    return 0;
}

void main()
{
    foo();
}

// CHECK:  %foo = OpFunction %v4float DontInline {{%[0-9]+}}
