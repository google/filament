// RUN: %dxc -T lib_6_9  -ast-dump-implicit %s | FileCheck -check-prefix=ASTIMPL %s
// RUN: %dxc -T lib_6_9  -ast-dump %s | FileCheck -check-prefix=AST %s
// The HLSL source is just a copy of 
// tools\clang\test\HLSLFileCheck\shader_targets\raytracing\subobjects_raytracingPipelineConfig1.hlsl

// This test tests that the HLSLSubObjectAttr attribute is present on all
// HLSL subobjects, and tests the ast representation of subobjects

// ASTIMPL: CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced struct StateObjectConfig definition
// ASTIMPL-NEXT: HLSLSubObjectAttr 0x{{.+}} <<invalid sloc>> Implicit 0 2
// ASTIMPL-NEXT: FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit Flags 'unsigned int'
// ASTIMPL-NEXT: CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced struct GlobalRootSignature definition
// ASTIMPL-NEXT: HLSLSubObjectAttr 0x{{.+}} <<invalid sloc>> Implicit 1 2
// ASTIMPL-NEXT: FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit Data 'string'
// ASTIMPL-NEXT: CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced struct LocalRootSignature definition
// ASTIMPL-NEXT: HLSLSubObjectAttr 0x{{.+}} <<invalid sloc>> Implicit 2 2
// ASTIMPL-NEXT: FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit Data 'string'
// ASTIMPL-NEXT: CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced struct SubobjectToExportsAssociation definition
// ASTIMPL-NEXT: HLSLSubObjectAttr 0x{{.+}} <<invalid sloc>> Implicit 8 2
// ASTIMPL-NEXT: FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit Subobject 'string'
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit Exports 'string'
// ASTIMPL-NEXT: CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced struct RaytracingShaderConfig definition
// ASTIMPL-NEXT: HLSLSubObjectAttr 0x{{.+}} <<invalid sloc>> Implicit 9 2
// ASTIMPL-NEXT: FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit MaxPayloadSizeInBytes 'unsigned int'
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit MaxAttributeSizeInBytes 'unsigned int'
// ASTIMPL-NEXT: CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit struct RaytracingPipelineConfig definition
// ASTIMPL-NEXT: HLSLSubObjectAttr 0x{{.+}} <<invalid sloc>> Implicit 10 2
// ASTIMPL-NEXT: FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit MaxTraceRecursionDepth 'unsigned int'
// ASTIMPL-NEXT: CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced struct TriangleHitGroup definition
// ASTIMPL-NEXT: HLSLSubObjectAttr 0x{{.+}} <<invalid sloc>> Implicit 11 0
// ASTIMPL-NEXT: FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit AnyHit 'string'
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit ClosestHit 'string'
// ASTIMPL-NEXT: CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced struct ProceduralPrimitiveHitGroup definition
// ASTIMPL-NEXT: HLSLSubObjectAttr 0x{{.+}} <<invalid sloc>> Implicit 11 1
// ASTIMPL-NEXT: FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit AnyHit 'string'
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit ClosestHit 'string'
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit Intersection 'string'
// ASTIMPL-NEXT: CXXRecordDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit referenced struct RaytracingPipelineConfig1 definition
// ASTIMPL-NEXT: HLSLSubObjectAttr 0x{{.+}} <<invalid sloc>> Implicit 12 2
// ASTIMPL-NEXT: FinalAttr 0x{{.+}} <<invalid sloc>> Implicit final
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit MaxTraceRecursionDepth 'unsigned int'
// ASTIMPL-NEXT: FieldDecl 0x{{.+}} <<invalid sloc>> <invalid sloc> implicit Flags 'unsigned int'

// AST: VarDecl 0x{{.+}} grs 'GlobalRootSignature' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'GlobalRootSignature'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "CBV(b0)"
// AST-NEXT: VarDecl 0x{{.+}} soc 'StateObjectConfig' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'StateObjectConfig'
// AST-NEXT: BinaryOperator 0x{{.+}} 'unsigned int' '|'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'unsigned int' <LValueToRValue>
// AST-NEXT: DeclRefExpr 0x{{.+}} 'const unsigned int' lvalue Var 0x{{.+}} 'STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS' 'const unsigned int'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'unsigned int' <LValueToRValue>
// AST-NEXT: DeclRefExpr 0x{{.+}} 'const unsigned int' lvalue Var 0x{{.+}} 'STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS' 'const unsigned int'
// AST-NEXT: VarDecl 0x{{.+}} lrs 'LocalRootSignature' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'LocalRootSignature'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "UAV(u0, visibility = SHADER_VISIBILITY_GEOMETRY), RootFlags(LOCAL_ROOT_SIGNATURE)"
// AST-NEXT: VarDecl 0x{{.+}} sea 'SubobjectToExportsAssociation' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'SubobjectToExportsAssociation'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "grs"
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "a;b;foo;c"
// AST-NEXT: VarDecl 0x{{.+}} sea2 'SubobjectToExportsAssociation' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'SubobjectToExportsAssociation'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "grs"
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue ";"
// AST-NEXT: VarDecl 0x{{.+}} sea3 'SubobjectToExportsAssociation' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'SubobjectToExportsAssociation'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "grs"
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue ""
// AST-NEXT: VarDecl 0x{{.+}} rsc 'RaytracingShaderConfig' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'RaytracingShaderConfig'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'unsigned int' <IntegralCast>
// AST-NEXT: IntegerLiteral 0x{{.+}} 'literal int' 128
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'unsigned int' <IntegralCast>
// AST-NEXT: IntegerLiteral 0x{{.+}} 'literal int' 64
// AST-NEXT: VarDecl 0x{{.+}} rpc 'RaytracingPipelineConfig1' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'RaytracingPipelineConfig1'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'unsigned int' <IntegralCast>
// AST-NEXT: IntegerLiteral 0x{{.+}} 'literal int' 32
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'unsigned int' <LValueToRValue>
// AST-NEXT: DeclRefExpr 0x{{.+}} 'const unsigned int' lvalue Var 0x{{.+}} 'RAYTRACING_PIPELINE_FLAG_SKIP_TRIANGLES' 'const unsigned int'
// AST-NEXT: VarDecl 0x{{.+}} sea4 'SubobjectToExportsAssociation' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'SubobjectToExportsAssociation'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "rpc"
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue ";"
// AST-NEXT: VarDecl 0x{{.+}} rpc2 'RaytracingPipelineConfig1' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'RaytracingPipelineConfig1'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'unsigned int' <IntegralCast>
// AST-NEXT: IntegerLiteral 0x{{.+}} 'literal int' 32
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'unsigned int' <LValueToRValue>
// AST-NEXT: DeclRefExpr 0x{{.+}} 'const unsigned int' lvalue Var 0x{{.+}} 'RAYTRACING_PIPELINE_FLAG_NONE' 'const unsigned int'
// AST-NEXT: VarDecl 0x{{.+}} trHitGt 'TriangleHitGroup' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'TriangleHitGroup'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "a"
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "b"
// AST-NEXT: VarDecl 0x{{.+}} ppHitGt 'ProceduralPrimitiveHitGroup' static cinit
// AST-NEXT: InitListExpr 0x{{.+}} 'ProceduralPrimitiveHitGroup'
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "a"
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "b"
// AST-NEXT: ImplicitCastExpr 0x{{.+}} 'const string' <ArrayToPointerDecay>
// AST-NEXT: StringLiteral 0x{{.+}} 'literal string' lvalue "c"

GlobalRootSignature grs = {"CBV(b0)"};	
StateObjectConfig soc = { STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS | STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS };
LocalRootSignature lrs = {"UAV(u0, visibility = SHADER_VISIBILITY_GEOMETRY), RootFlags(LOCAL_ROOT_SIGNATURE)"};
SubobjectToExportsAssociation sea = { "grs", "a;b;foo;c" };
// Empty association is well-defined: it creates a default association
SubobjectToExportsAssociation sea2 = { "grs", ";" };
SubobjectToExportsAssociation sea3 = { "grs", "" };
RaytracingShaderConfig rsc = { 128, 64 };
RaytracingPipelineConfig1 rpc = { 32, RAYTRACING_PIPELINE_FLAG_SKIP_TRIANGLES };
SubobjectToExportsAssociation sea4 = {"rpc", ";"};
RaytracingPipelineConfig1 rpc2 = {32, RAYTRACING_PIPELINE_FLAG_NONE };
TriangleHitGroup trHitGt = {"a", "b"};
ProceduralPrimitiveHitGroup ppHitGt = { "a", "b", "c"};
