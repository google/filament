// Test indexing a swizzled bool vector
// RUN: %dxc -fcgl -T cs_6_0 %s | FileCheck %s

// This was asserting in Instructions.cpp with:
// void llvm::StoreInst::AssertOK(): Assertion `getOperand(0)->getType() == cast<PointerType>(getOperand(1)->getType())->getElementType() && "Ptr must be a pointer to Val type!"' failed.

// Make sure load of i32 gets truncated to i1 when indexing bool vectors
// CHECK:       [[TMP:%[a-z0-9\.]+]] = alloca <2 x i1>
// CHECK:       [[VA0:%[a-z0-9\.]+]] = getelementptr <2 x i1>, <2 x i1>* [[TMP]], i32 0, i32 0,
// CHECK-NEXT:  [[VA1:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds (<4 x i32>, <4 x i32>* @"\01?v_bool4@?1??main@@YAXXZ@3V?$vector@_N$03@@B", i32 0, i32 2),
// CHECK-NEXT:  [[VA2:%[a-z0-9\.]+]] = trunc i32 [[VA1]] to i1,
// CHECK-NEXT:  store i1 [[VA2]], i1* [[VA0]],
// CHECK-NEXT:  [[VB0:%[a-z0-9\.]+]] = getelementptr <2 x i1>, <2 x i1>* [[TMP]], i32 0, i32 1,
// CHECK-NEXT:  [[VB1:%[a-z0-9\.]+]] = load i32, i32* getelementptr inbounds (<4 x i32>, <4 x i32>* @"\01?v_bool4@?1??main@@YAXXZ@3V?$vector@_N$03@@B", i32 0, i32 3),
// CHECK-NEXT:  [[VB2:%[a-z0-9\.]+]] = trunc i32 [[VB1]] to i1,
// CHECK-NEXT:  store i1 [[VB2]], i1* [[VB0]],


cbuffer cbuffer_tint_symbol_3 : register(b0) {
    uint4 global_uint4[1];
};

[numthreads(1, 1, 1)]
void main() {
    const bool4 v_bool4 = bool4(true, true, true, true);
    const uint gx = global_uint4[0].x;
    if (v_bool4.zw[gx] == 0) {
        GroupMemoryBarrierWithGroupSync();
    }
}
