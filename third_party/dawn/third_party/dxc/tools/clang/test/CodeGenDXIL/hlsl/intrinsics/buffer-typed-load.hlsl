// RUN: %dxc -fcgl  -T vs_6_6 %s | FileCheck %s

// Source file for DxilGen IR test for typed buffer/texture load lowering

RWBuffer< bool2 > TyBuf : register(u1);
Texture2DMS< bool2 > Tex2dMs : register(t2);

Texture1D< float2 > Tex1d : register(t3);
Texture2D< float2 > Tex2d : register(t4);
Texture3D< float2 > Tex3d : register(t5);
Texture2DArray< float2 > Tex2dArr : register(t6);

RWBuffer< float2 > OutBuf : register(u7);

void main(uint ix1 : IX1, uint2 ix2 : IX2, uint3 ix3 : IX3, uint4 ix4 : IX4) {

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 1
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 0, %"class.RWBuffer<vector<bool, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<bool, 2> >" undef)
  // CHECK: call <2 x i1> @"dx.hl.op.ro.<2 x i1> (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  bool2  Tyb0  = TyBuf.Load(ix1 + 1);

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 2
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 0, %"class.RWBuffer<vector<bool, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<bool, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 517 }, %"class.RWBuffer<vector<bool, 2> >" undef)
  // CHECK: call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  bool2  Tyb1  = TyBuf[ix1 + 2];

  // CHECK: [[IX:%.*]] = add <2 x i32> {{%.*}}, <i32 3, i32 3>
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.Texture2DMS<vector<bool, 2>, 0>"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 3, i32 517 }, %"class.Texture2DMS<vector<bool, 2>, 0>" undef),
  // CHECK: call <2 x i1> @"dx.hl.op..<2 x i1> (i32, %dx.types.Handle, <2 x i32>, i32)"(i32 231, %dx.types.Handle [[ANHDL]], <2 x i32> [[IX]]
  bool2  TxMs0  = Tex2dMs.Load(ix2 + 3, ix1);

  // CHECK: [[IX:%.*]] = add <2 x i32> {{%.*}}, <i32 4, i32 4>
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2DMS<vector<bool, 2>, 0>\22)"(i32 0, %"class.Texture2DMS<vector<bool, 2>, 0>"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2DMS<vector<bool, 2>, 0>\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 3, i32 517 }, %"class.Texture2DMS<vector<bool, 2>, 0>" undef)
  // CHECK: call <2 x i32>* @"dx.hl.subscript.[].rn.<2 x i32>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[ANHDL]], <2 x i32> [[IX]])
  bool2  TxMs1  = Tex2dMs[ix2 + 4];

  // CHECK: [[IX:%.*]] = add <2 x i32> {{%.*}}, <i32 5, i32 5>
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture1D<vector<float, 2> >\22)"(i32 0, %"class.Texture1D<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture1D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 1, i32 521 }, %"class.Texture1D<vector<float, 2> >" undef)
  // CHECK: call <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <2 x i32>)"(i32 231, %dx.types.Handle [[ANHDL]], <2 x i32> [[IX]])
  float2 Tx1d0  = Tex1d.Load(ix2 + 5);

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 6
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture1D<vector<float, 2> >\22)"(i32 0, %"class.Texture1D<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture1D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 1, i32 521 }, %"class.Texture1D<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Tx1d1  = Tex1d[ix1 + 6];

  // CHECK: [[IX:%.*]] = add <3 x i32> {{%.*}}, <i32 7, i32 7, i32 7>
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<vector<float, 2> >\22)"(i32 0, %"class.Texture2D<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 2, i32 521 }, %"class.Texture2D<vector<float, 2> >" undef)
  // CHECK: call <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <3 x i32>)"(i32 231, %dx.types.Handle [[ANHDL]], <3 x i32> [[IX]])
  float2 Tx2d0  = Tex2d.Load(ix3 + 7);

  // CHECK: [[IX:%.*]] = add <2 x i32> {{%.*}}, <i32 8, i32 8>
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2D<vector<float, 2> >\22)"(i32 0, %"class.Texture2D<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 2, i32 521 }, %"class.Texture2D<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, <2 x i32>)"(i32 0, %dx.types.Handle [[ANHDL]], <2 x i32> [[IX]])
  float2 Tx2d1  = Tex2d[ix2 + 8];

  // CHECK: [[IX:%.*]] = add <4 x i32> {{%.*}}, <i32 9, i32 9, i32 9, i32 9>
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture3D<vector<float, 2> >\22)"(i32 0, %"class.Texture3D<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture3D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4, i32 521 }, %"class.Texture3D<vector<float, 2> >" undef)
  // CHECK: call <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <4 x i32>)"(i32 231, %dx.types.Handle [[ANHDL]], <4 x i32> [[IX]])
  float2 Tx3d0  = Tex3d.Load(ix4 + 9);

  // CHECK: [[IX:%.*]] = add <3 x i32> {{%.*}}, <i32 10, i32 10, i32 10>
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture3D<vector<float, 2> >\22)"(i32 0, %"class.Texture3D<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture3D<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4, i32 521 }, %"class.Texture3D<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle [[ANHDL]], <3 x i32> [[IX]])
  float2 Tx3d1  = Tex3d[ix3 + 10];

  // CHECK: [[IX:%.*]] = add <4 x i32> {{%.*}}, <i32 11, i32 11, i32 11, i32 11>
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2DArray<vector<float, 2> >\22)"(i32 0, %"class.Texture2DArray<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2DArray<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 7, i32 521 }, %"class.Texture2DArray<vector<float, 2> >" undef)
  // CHECK: call <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, <4 x i32>)"(i32 231, %dx.types.Handle [[ANHDL]], <4 x i32> [[IX]])
  float2 Tx2da0  = Tex2dArr.Load(ix4 + 11);

  // CHECK: [[IX:%.*]] = add <3 x i32> {{%.*}}, <i32 12, i32 12, i32 12>
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.Texture2DArray<vector<float, 2> >\22)"(i32 0, %"class.Texture2DArray<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.Texture2DArray<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 7, i32 521 }, %"class.Texture2DArray<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, <3 x i32>)"(i32 0, %dx.types.Handle [[ANHDL]], <3 x i32> [[IX]])
  float2 Tx2da1  = Tex2dArr[ix3 + 12];

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 13
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWBuffer<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 521 }, %"class.RWBuffer<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  OutBuf[ix1+13] = select(Tyb0, Tx1d0, Tx1d1);

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 14
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWBuffer<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 521 }, %"class.RWBuffer<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  OutBuf[ix1+14] = select(Tyb1, Tx2d0, Tx2d1);

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 15
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWBuffer<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 521 }, %"class.RWBuffer<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  OutBuf[ix1+15] = select(TxMs0, Tx3d0, Tx3d1);

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 16
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWBuffer<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4106, i32 521 }, %"class.RWBuffer<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  OutBuf[ix1+16] = select(TxMs1, Tx2da0, Tx2da1);
}
