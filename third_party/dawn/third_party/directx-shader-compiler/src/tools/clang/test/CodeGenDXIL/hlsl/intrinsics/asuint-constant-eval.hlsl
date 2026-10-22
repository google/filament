// REQUIRES: dxil-1-9
// RUN: %dxc /T ps_6_9 -fcgl %s | FileCheck %s

// Compiling this HLSL would fail this assertion in IntExprEvaluator::Success:
//
//     assert(SI.isSigned() == E->getType()->isSignedIntegerOrEnumerationType() &&
//            "Invalid evaluation result.");
//
// Bug was fixed in HandleIntrinsicCall by updating the APSInt's signednes.

// CHECK: %{{.*}} = call i32 @"dx.hl.op.rn.i32 (i32, i32)"(i32 {{.*}}, i32 1)

void main()
{
    int i = asuint(int(1));
}
