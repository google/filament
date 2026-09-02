// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation,UAVSize=128,upstreamSVPositionRow=2 -hlsl-dxilemit | %FileCheck %s

// Check that the basic starting header is present:

// Check we added the UAV:                                                                      v----metadata position: not important for this check
// CHECK: %PIX_DebugUAV_Handle = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 [[S:[0-9]+]], i32 0, i1 false)
// CHECK: %XPos = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: %YPos = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)
// CHECK: %XIndex = fptoui float %XPos to i32
// CHECK: %YIndex = fptoui float %YPos to i32
// CHECK: %CompareToX = icmp eq i32 %XIndex, 0
// CHECK: %CompareToY = icmp eq i32 %YIndex, 0
// CHECK: %ComparePos = and i1 %CompareToX, %CompareToY


// Check for branches-for-interest and AND value and counter location for a UAV size of 128
// CHECK: br i1 %ComparePos, label %PIXInterestingBlock, label %PIXNonInterestingBlock
// CHECK: %PIXOffsetOr = phi i32 [ 0, %PIXInterestingBlock ], [ 64, %PIXNonInterestingBlock ]
// CHECK: %PIXCounterLocation = phi i32 [ 60, %PIXInterestingBlock ], [ 124, %PIXNonInterestingBlock ]

// Check the first block header was emitted: (increment, AND + OR)
// CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle %PIX_DebugUAV_Handle, i32 0
// CHECK: and i32 
// CHECK: or i32

// Check that the correct metadata was emitted for the requested SV_Position at row 2, col 0, 1 row, 4 cols.
// See DxilMDHelper::EmitSignatureElement for the meaning of these entries:
//             ID                 TypeF32 SemKin Sem-Idx-Vec           interp  Rows Cols   Row    Col
//              |                     |     |       |                    |      |     |      |     |
// CHECK: !{i32 0, !"SV_Position", i8 9, i8 3, ![[SEMIDXVEC:[0-9]*]], i8 2, i32 1, i8 4, i32 2, i8 0, null}
// CHECK: ![[SEMIDXVEC]] = !{i32 0}

[RootSignature("")]
float4 main() : SV_Target {
    return float4(0,0,0,0);
}