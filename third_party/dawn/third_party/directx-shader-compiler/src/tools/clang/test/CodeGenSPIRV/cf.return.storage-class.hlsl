// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct BufferType {
    float     a;
    float3    b;
    float3x2  c;
};

RWStructuredBuffer<BufferType> sbuf;  // %BufferType

// CHECK: %retSBuffer5 = OpFunction %BufferType_0 None {{%[0-9]+}}
BufferType retSBuffer5() {            // BufferType_0
// CHECK:    %temp_var_ret = OpVariable %_ptr_Function_BufferType_0 Function

// CHECK-NEXT: [[sbuf:%[0-9]+]] = OpAccessChain %_ptr_Uniform_BufferType %sbuf %int_0 %uint_5
// CHECK-NEXT:  [[val:%[0-9]+]] = OpLoad %BufferType [[sbuf]]
// CHECK-NEXT:    [[a:%[0-9]+]] = OpCompositeExtract %float [[val]] 0
// CHECK-NEXT:    [[b:%[0-9]+]] = OpCompositeExtract %v3float [[val]] 1
// CHECK-NEXT:    [[c:%[0-9]+]] = OpCompositeExtract %mat3v2float [[val]] 2
// CHECK-NEXT:  [[tmp:%[0-9]+]] = OpCompositeConstruct %BufferType_0 [[a]] [[b]] [[c]]
// CHECK-NEXT:                 OpStore %temp_var_ret [[tmp]]
// CHECK-NEXT:  [[tmp_0:%[0-9]+]] = OpLoad %BufferType_0 %temp_var_ret
// CHECK-NEXT:       OpReturnValue [[tmp_0]]
// CHECK-NEXT:       OpFunctionEnd
    return sbuf[5];
}

void main() {
    sbuf[6] = retSBuffer5();
}
