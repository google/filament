// RUN: %dxc -fcgl  -T vs_6_6 %s | FileCheck %s

// Source file for DxilGen IR test for typed buffer store lowering
// Focuses on converted types in addition to common float type.

RWBuffer<float3>    FTyBuf;
RWBuffer<bool2>     BTyBuf;
RWBuffer<uint64_t2> LTyBuf;
RWBuffer<double>    DTyBuf;

RWTexture1D<float3>    FTex1d;
RWTexture1D<bool2>     BTex1d;
RWTexture1D<uint64_t2> LTex1d;
RWTexture1D<double>    DTex1d;

RWTexture2D<float3>    FTex2d;
RWTexture2D<bool2>     BTex2d;
RWTexture2D<uint64_t2> LTex2d;
RWTexture2D<double>    DTex2d;

RWTexture3D<float3>    FTex3d;
RWTexture3D<bool2>     BTex3d;
RWTexture3D<uint64_t2> LTex3d;
RWTexture3D<double>    DTex3d;

RWTexture2DMS<float3>    FTex2dMs;
RWTexture2DMS<bool2>     BTex2dMs;
RWTexture2DMS<uint64_t2> LTex2dMs;
RWTexture2DMS<double>    DTex2dMs;

// CHECK: define void @main(i32 %ix1, <2 x i32> %ix2, <3 x i32> %ix3)
void main(uint ix1 : IX1, uint2 ix2 : IX2, uint3 ix3 : IX3) {

  // CHECK-DAG: [[ix3adr:%.*]] = alloca <3 x i32>, align 4
  // CHECK-DAG: [[ix2adr:%.*]] = alloca <2 x i32>, align 4
  // CHECK-DAG: [[ix1adr:%.*]] = alloca i32, align 4

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 0
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 3> >\22)"(i32 0, %"class.RWBuffer<vector<float, 3> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 3> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 777 }, %"class.RWBuffer<vector<float, 3> >" undef)
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ld:%.*]] = load <3 x float>, <3 x float>* [[sub]]
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 1
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 3> >\22)"(i32 0, %"class.RWBuffer<vector<float, 3> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 3> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 777 }, %"class.RWBuffer<vector<float, 3> >" undef)
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: store <3 x float> [[ld]], <3 x float>* [[sub]]
  FTyBuf[ix1 + 1] = FTyBuf[ix1 + 0];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 2
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 0, %"class.RWBuffer<vector<bool, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<bool, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ld:%.*]] = load <2 x i32>, <2 x i32>* [[sub]]
  // CHECK: [[bld:%.*]] = icmp ne <2 x i32> [[ld]], zeroinitializer
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 3
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 0, %"class.RWBuffer<vector<bool, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<bool, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ld:%.*]] = zext <2 x i1> [[bld]] to <2 x i32>
  // CHECK: store <2 x i32> [[ld]], <2 x i32>* [[sub]]
  BTyBuf[ix1 + 3] = BTyBuf[ix1 + 2];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 4
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWBuffer<vector<unsigned long long, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<unsigned long long, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ld:%.*]] = load <2 x i64>, <2 x i64>* [[sub]]
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 5
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWBuffer<vector<unsigned long long, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<unsigned long long, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: store <2 x i64> [[ld]], <2 x i64>* [[sub]]
  LTyBuf[ix1 + 5] = LTyBuf[ix1 + 4];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 6
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<double>\22)"(i32 0, %"class.RWBuffer<double>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<double>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 261 }, %"class.RWBuffer<double>" undef)
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ld:%.*]] = load double, double* [[sub]]
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 7
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<double>\22)"(i32 0, %"class.RWBuffer<double>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<double>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4106, i32 261 }, %"class.RWBuffer<double>" undef)
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: store double [[ld]], double* [[sub]]
  DTyBuf[ix1 + 7] = DTyBuf[ix1 + 6];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 8
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture1D<vector<float, 3> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 777 }, %"class.RWTexture1D<vector<float, 3> >" undef)
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ld:%.*]] = load <3 x float>, <3 x float>* [[sub]]
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 9
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture1D<vector<float, 3> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 777 }, %"class.RWTexture1D<vector<float, 3> >" undef)
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: store <3 x float> [[ld]], <3 x float>* [[sub]]
  FTex1d[ix1 + 9] = FTex1d[ix1 + 8];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 10
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture1D<vector<bool, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 517 }, %"class.RWTexture1D<vector<bool, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ld:%.*]] = load <2 x i32>, <2 x i32>* [[sub]]
  // CHECK: [[bld:%.*]] = icmp ne <2 x i32> [[ld]], zeroinitializer
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 11
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture1D<vector<bool, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 517 }, %"class.RWTexture1D<vector<bool, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ld:%.*]] = zext <2 x i1> [[bld]] to <2 x i32>
  // CHECK: store <2 x i32> [[ld]], <2 x i32>* [[sub]]
  BTex1d[ix1 + 11] = BTex1d[ix1 + 10];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 12
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture1D<vector<unsigned long long, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 517 }, %"class.RWTexture1D<vector<unsigned long long, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ld:%.*]] = load <2 x i64>, <2 x i64>* [[sub]]
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 13
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture1D<vector<unsigned long long, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 517 }, %"class.RWTexture1D<vector<unsigned long long, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: store <2 x i64> [[ld]], <2 x i64>* [[sub]]
  LTex1d[ix1 + 13] = LTex1d[ix1 + 12];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 14
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<double>\22)"(i32 0, %"class.RWTexture1D<double>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<double>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 261 }, %"class.RWTexture1D<double>" undef)
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ld:%.*]] = load double, double* [[sub]]
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[ix:%.*]] = add i32 [[ix1]], 15
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture1D<double>\22)"(i32 0, %"class.RWTexture1D<double>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture1D<double>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4097, i32 261 }, %"class.RWTexture1D<double>" undef)
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: store double [[ld]], double* [[sub]]
  DTex1d[ix1 + 15] = DTex1d[ix1 + 14];

  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 16, i32 16>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture2D<vector<float, 3> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 777 }, %"class.RWTexture2D<vector<float, 3> >" undef)
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load <3 x float>, <3 x float>* [[sub]]
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 17, i32 17>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture2D<vector<float, 3> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 777 }, %"class.RWTexture2D<vector<float, 3> >" undef)
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: store <3 x float> [[ld]], <3 x float>* [[sub]]
  FTex2d[ix2 + 17] = FTex2d[ix2 + 16];

  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 18, i32 18>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture2D<vector<bool, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 517 }, %"class.RWTexture2D<vector<bool, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load <2 x i32>, <2 x i32>* [[sub]]
  // CHECK: [[bld:%.*]] = icmp ne <2 x i32> [[ld]], zeroinitializer
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 19, i32 19>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture2D<vector<bool, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 517 }, %"class.RWTexture2D<vector<bool, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = zext <2 x i1> [[bld]] to <2 x i32>
  // CHECK: store <2 x i32> [[ld]], <2 x i32>* [[sub]]
  BTex2d[ix2 + 19] = BTex2d[ix2 + 18];

  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 20, i32 20>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture2D<vector<unsigned long long, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 517 }, %"class.RWTexture2D<vector<unsigned long long, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load <2 x i64>, <2 x i64>* [[sub]]
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 21, i32 21>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture2D<vector<unsigned long long, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 517 }, %"class.RWTexture2D<vector<unsigned long long, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: store <2 x i64> [[ld]], <2 x i64>* [[sub]]
  LTex2d[ix2 + 21] = LTex2d[ix2 + 20];

  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 22, i32 22>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<double>\22)"(i32 0, %"class.RWTexture2D<double>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<double>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 261 }, %"class.RWTexture2D<double>" undef)
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load double, double* [[sub]]
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 23, i32 23>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2D<double>\22)"(i32 0, %"class.RWTexture2D<double>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2D<double>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4098, i32 261 }, %"class.RWTexture2D<double>" undef)
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: store double [[ld]], double* [[sub]]
  DTex2d[ix2 + 23] = DTex2d[ix2 + 22];

  // CHECK: [[ix3:%.*]] = load <3 x i32>, <3 x i32>* [[ix3adr]], align 4
  // CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 24, i32 24, i32 24>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture3D<vector<float, 3> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 777 }, %"class.RWTexture3D<vector<float, 3> >" undef)
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <3 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load <3 x float>, <3 x float>* [[sub]]
  // CHECK: [[ix3:%.*]] = load <3 x i32>, <3 x i32>* [[ix3adr]], align 4
  // CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 25, i32 25, i32 25>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<float, 3> >\22)"(i32 0, %"class.RWTexture3D<vector<float, 3> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<float, 3> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 777 }, %"class.RWTexture3D<vector<float, 3> >" undef)
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <3 x i32> [[ix]])
  // CHECK: store <3 x float> [[ld]], <3 x float>* [[sub]]
  FTex3d[ix3 + 25] = FTex3d[ix3 + 24];

  // CHECK: [[ix3:%.*]] = load <3 x i32>, <3 x i32>* [[ix3adr]], align 4
  // CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 26, i32 26, i32 26>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture3D<vector<bool, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 517 }, %"class.RWTexture3D<vector<bool, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <3 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load <2 x i32>, <2 x i32>* [[sub]]
  // CHECK: [[bld:%.*]] = icmp ne <2 x i32> [[ld]], zeroinitializer
  // CHECK: [[ix3:%.*]] = load <3 x i32>, <3 x i32>* [[ix3adr]], align 4
  // CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 27, i32 27, i32 27>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<bool, 2> >\22)"(i32 0, %"class.RWTexture3D<vector<bool, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 517 }, %"class.RWTexture3D<vector<bool, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <3 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = zext <2 x i1> [[bld]] to <2 x i32>
  // CHECK: store <2 x i32> [[ld]], <2 x i32>* [[sub]]
  BTex3d[ix3 + 27] = BTex3d[ix3 + 26];

  // CHECK: [[ix3:%.*]] = load <3 x i32>, <3 x i32>* [[ix3adr]], align 4
  // CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 28, i32 28, i32 28>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture3D<vector<unsigned long long, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 517 }, %"class.RWTexture3D<vector<unsigned long long, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <3 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load <2 x i64>, <2 x i64>* [[sub]]
  // CHECK: [[ix3:%.*]] = load <3 x i32>, <3 x i32>* [[ix3adr]], align 4
  // CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 29, i32 29, i32 29>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<vector<unsigned long long, 2> >\22)"(i32 0, %"class.RWTexture3D<vector<unsigned long long, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<vector<unsigned long long, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 517 }, %"class.RWTexture3D<vector<unsigned long long, 2> >" undef)
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <3 x i32> [[ix]])
  // CHECK: store <2 x i64> [[ld]], <2 x i64>* [[sub]]
  LTex3d[ix3 + 29] = LTex3d[ix3 + 28];

  // CHECK: [[ix3:%.*]] = load <3 x i32>, <3 x i32>* [[ix3adr]], align 4
  // CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 30, i32 30, i32 30>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<double>\22)"(i32 0, %"class.RWTexture3D<double>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<double>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 261 }, %"class.RWTexture3D<double>" undef)
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <3 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load double, double* [[sub]]
  // CHECK: [[ix3:%.*]] = load <3 x i32>, <3 x i32>* [[ix3adr]], align 4
  // CHECK: [[ix:%.*]] = add <3 x i32> [[ix3]], <i32 31, i32 31, i32 31>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture3D<double>\22)"(i32 0, %"class.RWTexture3D<double>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture3D<double>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4100, i32 261 }, %"class.RWTexture3D<double>" undef)
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <3 x i32> [[ix]])
  // CHECK: store double [[ld]], double* [[sub]]
  DTex3d[ix3 + 31] = DTex3d[ix3 + 30];

  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 32, i32 32>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<float, 3>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 777 }, %"class.RWTexture2DMS<vector<float, 3>, 0>" undef)
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load <3 x float>, <3 x float>* [[sub]]
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 33, i32 33>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<float, 3>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 777 }, %"class.RWTexture2DMS<vector<float, 3>, 0>" undef)
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: store <3 x float> [[ld]], <3 x float>* [[sub]]
  FTex2dMs[ix2 + 33] = FTex2dMs[ix2 + 32];

  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 34, i32 34>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<bool, 2>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<bool, 2>, 0>" undef)
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load <2 x i32>, <2 x i32>* [[sub]]
  // CHECK: [[bld:%.*]] = icmp ne <2 x i32> [[ld]], zeroinitializer
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 35, i32 35>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<bool, 2>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<bool, 2>, 0>" undef)
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = zext <2 x i1> [[bld]] to <2 x i32>
  // CHECK: store <2 x i32> [[ld]], <2 x i32>* [[sub]]
  BTex2dMs[ix2 + 35] = BTex2dMs[ix2 + 34];

  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 36, i32 36>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" undef)
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load <2 x i64>, <2 x i64>* [[sub]]
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 37, i32 37>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" undef)
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: store <2 x i64> [[ld]], <2 x i64>* [[sub]]
  LTex2dMs[ix2 + 37] = LTex2dMs[ix2 + 36];

  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 38, i32 38>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<double, 0>\22)"(i32 0, %"class.RWTexture2DMS<double, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<double, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 261 }, %"class.RWTexture2DMS<double, 0>" undef)
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: [[ld:%.*]] = load double, double* [[sub]]
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 39, i32 39>
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<double, 0>\22)"(i32 0, %"class.RWTexture2DMS<double, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<double, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 261 }, %"class.RWTexture2DMS<double, 0>" undef)
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[].rn.double* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]])
  // CHECK: store double [[ld]], double* [[sub]]
  DTex2dMs[ix2 + 39] = DTex2dMs[ix2 + 38];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[sax:%.*]] = add i32 [[ix1]], 0
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<float, 3>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 777 }, %"class.RWTexture2DMS<vector<float, 3>, 0>" undef)
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 40, i32 40>
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[][].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]], i32 [[sax]])
  // CHECK: [[ld:%.*]] = load <3 x float>, <3 x float>* [[sub]]
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[sax:%.*]] = add i32 [[ix1]], 1
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<float, 3>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<float, 3>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 777 }, %"class.RWTexture2DMS<vector<float, 3>, 0>" undef)
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 41, i32 41>
  // CHECK: [[sub:%.*]] = call <3 x float>* @"dx.hl.subscript.[][].rn.<3 x float>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]], i32 [[sax]])
  // CHECK: store <3 x float> [[ld]], <3 x float>* [[sub]]
  FTex2dMs.sample[ix1 + 1][ix2 + 41] = FTex2dMs.sample[ix1 + 0][ix2 + 40];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[sax:%.*]] = add i32 [[ix1]], 2
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<bool, 2>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<bool, 2>, 0>" undef)
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 42, i32 42>
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[][].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]], i32 [[sax]])
  // CHECK: [[ld:%.*]] = load <2 x i32>, <2 x i32>* [[sub]]
  // CHECK: [[bld:%.*]] = icmp ne <2 x i32> [[ld]], zeroinitializer
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[sax:%.*]] = add i32 [[ix1]], 3
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<bool, 2>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<bool, 2>, 0>" undef)
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 43, i32 43>
  // CHECK: [[sub:%.*]] = call <2 x i32>* @"dx.hl.subscript.[][].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]], i32 [[sax]])
  // CHECK: [[ld:%.*]] = zext <2 x i1> [[bld]] to <2 x i32>
  // CHECK: store <2 x i32> [[ld]], <2 x i32>* [[sub]]
  BTex2dMs.sample[ix1 + 3][ix2 + 43] = BTex2dMs.sample[ix1 + 2][ix2 + 42];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[sax:%.*]] = add i32 [[ix1]], 4
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" undef)
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 44, i32 44>
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[][].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]], i32 [[sax]])
  // CHECK: [[ld:%.*]] = load <2 x i64>, <2 x i64>* [[sub]]
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[sax:%.*]] = add i32 [[ix1]], 5
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 0, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<vector<unsigned long long, 2>, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 517 }, %"class.RWTexture2DMS<vector<unsigned long long, 2>, 0>" undef)
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 45, i32 45>
  // CHECK: [[sub:%.*]] = call <2 x i64>* @"dx.hl.subscript.[][].rn.<2 x i64>* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]], i32 [[sax]])
  // CHECK: store <2 x i64> [[ld]], <2 x i64>* [[sub]]
  LTex2dMs.sample[ix1 + 5][ix2 + 45] = LTex2dMs.sample[ix1 + 4][ix2 + 44];

  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[sax:%.*]] = add i32 [[ix1]], 6
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<double, 0>\22)"(i32 0, %"class.RWTexture2DMS<double, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<double, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 261 }, %"class.RWTexture2DMS<double, 0>" undef)
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 46, i32 46>
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[][].rn.double* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]], i32 [[sax]])
  // CHECK: [[ld:%.*]] = load double, double* [[sub]]
  // CHECK: [[ix1:%.*]] = load i32, i32* [[ix1adr]], align 4
  // CHECK: [[sax:%.*]] = add i32 [[ix1]], 7
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWTexture2DMS<double, 0>\22)"(i32 0, %"class.RWTexture2DMS<double, 0>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWTexture2DMS<double, 0>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4099, i32 261 }, %"class.RWTexture2DMS<double, 0>" undef)
  // CHECK: [[ix2:%.*]] = load <2 x i32>, <2 x i32>* [[ix2adr]], align 4
  // CHECK: [[ix:%.*]] = add <2 x i32> [[ix2:%.*]], <i32 47, i32 47>
  // CHECK: [[sub:%.*]] = call double* @"dx.hl.subscript.[][].rn.double* (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 5, %dx.types.Handle [[anhdl]], <2 x i32> [[ix]], i32 [[sax]])
  // CHECK: store double [[ld]], double* [[sub]]
  DTex2dMs.sample[ix1 + 7][ix2 + 47] = DTex2dMs.sample[ix1 + 6][ix2 + 46];

  // CHECK: ret void

}
