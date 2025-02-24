// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-add-pixel-hit-instrmentation,rt-width=16,num-pixels=64,force-early-z=1 | %FileCheck %s

// Check the write to the UAV was emitted:
// CHECK: %UAVIncResult = call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle %PIX_CountUAV_Handle, i32 0, i32 %ByteIndex, i32 undef, i32 undef, i32 1)

// Early z flag value is 8. The flags are stored in an entry in the entry function description record. See:
// https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/DXIL.rst#shader-properties-and-capabilities
// CHECK: !{i32 0, i64 8}

float4 main(float4 pos : SV_Position) : SV_Target {
  return pos;
}




