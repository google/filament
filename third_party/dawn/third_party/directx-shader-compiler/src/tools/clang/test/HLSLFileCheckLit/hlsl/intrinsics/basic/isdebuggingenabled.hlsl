// REQUIRES: dxil-1-10

// RUN: %dxc -T cs_6_10 %s | FileCheck %s
// RUN: %dxc -T cs_6_10 -fcgl %s | FileCheck %s --check-prefix=FCGL
// RUN: %dxc -T cs_6_10 -ast-dump %s | FileCheck %s --check-prefix=AST

// CHECK: call i1 @dx.op.isDebuggingEnabled(i32 -2147483614)  ; IsDebuggingEnabled()
// CHECK: call i1 @dx.op.isDebuggingEnabled(i32 -2147483614)  ; IsDebuggingEnabled()

// FCGL: call i1 @"dx.hl.op..i1 (i32)"(i32 421)
// FCGL: call i1 @"dx.hl.op..i1 (i32)"(i32 421)
// FCGL-NOT: dx.hl.op.ro

// AST: CallExpr {{.*}} 'bool'
// AST-NEXT: `-ImplicitCastExpr {{.*}} 'bool (*)()' <FunctionToPointerDecay>
// AST-NEXT:   `-DeclRefExpr {{.*}} 'IsDebuggingEnabled' 'bool ()'

RWStructuredBuffer<uint> Output : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID) {
    // Debugging state can change between observations.
    bool first = dx::IsDebuggingEnabled();
    bool second = dx::IsDebuggingEnabled();
    Output[threadId.x] = (uint)first | ((uint)second << 1);
}
