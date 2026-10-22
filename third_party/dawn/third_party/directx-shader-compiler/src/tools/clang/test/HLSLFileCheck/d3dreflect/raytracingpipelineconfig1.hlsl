// RUN: %dxilver 1.9 | %dxc -T lib_6_9  %s | FileCheck %s
// RUN: %dxilver 1.9 | %dxc -T lib_6_9 -ast-dump %s | FileCheck -check-prefix=AST %s
// RUN: %dxilver 1.9 | %dxc -T lib_6_9 -ast-dump-implicit %s | FileCheck -check-prefix=ASTIMPL %s


// CHECK: ; RaytracingPipelineConfig1 rpc = { MaxTraceRecursionDepth = 32, Flags = RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS };

// AST: TranslationUnitDecl 0x{{.+}} <<invalid sloc>> <invalid sloc>
// AST-NEXT: VarDecl 0x{{.+}} rpc 'RaytracingPipelineConfig1' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'RaytracingPipelineConfig1'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'unsigned int' <IntegralCast>
// AST-NEXT: IntegerLiteral 0x{{.+}} 'literal int' 32
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'unsigned int' <LValueToRValue>
// AST-NEXT: DeclRefExpr 0x{{.+}} 'const unsigned int' lvalue Var 0x{{.+}} 'RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS' 'const unsigned int'
// ASTIMPL: VarDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS 'const unsigned int' static cinit
// ASTIMPL-NEXT: IntegerLiteral 0x{{.+}} <<invalid sloc>> 'const unsigned int' 1024
// ASTIMPL-NEXT: AvailabilityAttr 0x{{.+}} <<invalid sloc>> Implicit  6.9 0 0 ""

RaytracingPipelineConfig1 rpc = { 32, RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS };
