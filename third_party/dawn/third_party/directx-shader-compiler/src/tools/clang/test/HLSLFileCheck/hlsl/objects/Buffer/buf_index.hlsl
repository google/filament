// RUN: %dxc -E main -T ps_6_0 -ast-dump-implicit %s | FileCheck %s -check-prefix=AST
// RUN: %dxc -E main -T ps_6_0 -fcgl %s | FileCheck %s -check-prefix=FCGL
// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s -check-prefix=OPTIMIZED

Buffer<float4> buf;

float4 main(uint i:I) : SV_Target {
  return buf[2][i];
}

// Tests vector index converting to an expected load sequence.

// Verify the generated AST for the subscript operator.

// AST: ClassTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit Buffer
// AST: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit class Buffer definition
// AST-NOT: CXXRecordDecl

// This match is a little intentionally fuzzy. There's some oddities in DXC's
// ASTs which are a little tricky to resolve but we might want to fix someday.

// AST: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[] 'const element &(unsigned int) const'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
// AST-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit

// Verify the initial IR generation correctly generates the handle and subscript
// intrinsics. These matches are some crazy long lines, so they are broken into
// multiple checks on the same line of text.

// FCGL: [[Buffer:%.+]] = load %"class.Buffer<vector<float, 4> >",
// FCGL-SAME: %"class.Buffer<vector<float, 4> >"* @"\01?buf{{[@$?.A-Za-z0-9_]+}}"

// FCGL: [[Handle:%.+]] = call %dx.types.Handle
// FCGL-SAME: @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Buffer<vector<float, 4> >\22)"
// FCGL-SAME: (i32 0, %"class.Buffer<vector<float, 4> >" [[Buffer]])

// FCGL: [[AnnHandle:%.+]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle
// FCGL-SAME: (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Buffer<vector<float, 4> >\22)"
// FCGL-SAME: (i32 {{[0-9]+}},
// FCGL-SAME: %dx.types.Handle [[Handle]],
// FCGL-SAME: %dx.types.ResourceProperties { i32 {{[0-9]+}}, i32 {{[0-9]+}} },
// FCGL-SAME: %"class.Buffer<vector<float, 4> >" undef)

// FCGL: {{%.+}} = call <4 x float>* @"dx.hl.subscript.[].rn.<4 x float>* (i32, %dx.types.Handle, i32)"
// FCGL-SAME: (i32 0, %dx.types.Handle [[AnnHandle]], i32 2)


// Verifies the optimized and loowered DXIL representation.
// OPTIMIZED: [[tmp:%.+]] = alloca [4 x float]
// OPTIMIZED: [[loaded:%.+]] = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(
// OPTIMIZED: [[X:%.+]] = extractvalue %dx.types.ResRet.f32 [[loaded]], 0
// OPTIMIZED: [[Y:%.+]] = extractvalue %dx.types.ResRet.f32 [[loaded]], 1
// OPTIMIZED: [[Z:%.+]] = extractvalue %dx.types.ResRet.f32 [[loaded]], 2
// OPTIMIZED: [[W:%.+]] = extractvalue %dx.types.ResRet.f32 [[loaded]], 3

// OPTIMIZED-DAG: [[XAddr:%.+]] = getelementptr inbounds [4 x float], [4 x float]* [[tmp]], i32 0, i32 0
// OPTIMIZED-DAG: [[YAddr:%.+]] = getelementptr inbounds [4 x float], [4 x float]* [[tmp]], i32 0, i32 1
// OPTIMIZED-DAG: [[ZAddr:%.+]] = getelementptr inbounds [4 x float], [4 x float]* [[tmp]], i32 0, i32 2
// OPTIMIZED-DAG: [[WAddr:%.+]] = getelementptr inbounds [4 x float], [4 x float]* [[tmp]], i32 0, i32 3

// OPTIMIZED-DAG: store float [[X]], float* [[XAddr]]
// OPTIMIZED-DAG: store float [[Y]], float* [[YAddr]]
// OPTIMIZED-DAG: store float [[Z]], float* [[ZAddr]]
// OPTIMIZED-DAG: store float [[W]], float* [[WAddr]]

// OPTIMIZED: [[elAddr:%.+]] = getelementptr inbounds [4 x float], [4 x float]* [[tmp]], i32 0, i32 {{.*}}
// OPTIMIZED: [[Result:%.+]] = load float, float* [[elAddr]], align 4

// OPTIMIZED: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float [[Result]])
// OPTIMIZED: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float [[Result]])
// OPTIMIZED: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2, float [[Result]])
// OPTIMIZED: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float [[Result]])
