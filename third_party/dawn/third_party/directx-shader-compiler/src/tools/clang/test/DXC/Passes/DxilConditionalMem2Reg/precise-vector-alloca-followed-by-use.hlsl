// RUN: %dxc -T vs_6_0 %s | FileCheck %s

// The following HLSL resulted in an ASAN use-after-free in DxilConditionalMem2Reg
// in ScalarizePreciseVectorAlloca. ScalarizePreciseVectorAlloca would iterate over
// all instructions, then for each instruction use, would iterate and potentially
// erase the instruction. If the erased instruction was the immediate next
// instruction after the alloca, this would invalidate the outer instruction iterator.

// Unfortunately, we cannot create an IR test for this because dxil-cond-mem2reg
// executes between scalarrepl-param-hlsl and hlsl-dxil-precise, and the former
// temporarily marks empty functions as 'precise' while the latter pass uses this
// information, and then deletes these functions. But splitting the passes in between
// these two fails validation because empty functions cannot have attributes on them.
// So we use a full HLSL test for this.

// The IR before dxil-cond-mem2reg for this HLSL contains a precise vector alloca
// followed immediately by a use of the alloca (a store in this case):
//
//  %sp.0 = alloca <4 x float>, !dx.precise !3
//  store <4 x float> zeroinitializer, <4 x float>* %sp.0, !dbg !4
//
// After dxil-cond-mem2reg, it should look like:
//
//  %1 = alloca float, !dx.precise !3
//  %2 = alloca float, !dx.precise !3
//  %3 = alloca float, !dx.precise !3
//  %4 = alloca float, !dx.precise !3
//  store float 0.000000e+00, float* %1, !dbg !4
//  store float 0.000000e+00, float* %2, !dbg !4
//  store float 0.000000e+00, float* %3, !dbg !4
//  store float 0.000000e+00, float* %4, !dbg !4

// CHECK:      call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)
// CHECK-NEXT: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 1.000000e+00)
// CHECK-NEXT: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float 1.000000e+00)
// CHECK-NEXT: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float 1.000000e+00)

struct S {
  float4 b;
};

struct SP {
  precise float4 b : SV_Position;
};

static S s = {(1.0f).xxxx};

SP main() {
  SP sp = (SP)0;
  sp.b = s.b;
  return sp;
}
