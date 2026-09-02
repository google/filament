// REQUIRES: dxil-1-10

// RUN: %dxc -T cs_6_10 %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -fcgl %s | FileCheck %s --check-prefix=FCGL
// RUN: %dxc -T cs_6_10 -ast-dump %s | FileCheck %s --check-prefix=AST

// CHECK: call void @dx.op.debugBreak(i32 -2147483615)  ; DebugBreak()
// FCGL: call void @"dx.hl.op..void (i32)"(i32 420)

// AST: CallExpr {{.*}} 'void'
// AST-NEXT: `-ImplicitCastExpr {{.*}} 'void (*)()' <FunctionToPointerDecay>
// AST-NEXT:   `-DeclRefExpr {{.*}} 'DebugBreak' 'void ()'


[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID) {
    if (threadId.x == 0) {
        DebugBreak();
    }
}
