// RUN: %dxc -fcgl  -T vs_6_6 %s | FileCheck %s

// Source file for DxilGen IR test for buffer load lowering
// Much of this mirrors buffer-load-store and buffer-agg-load-store

template<typename T, int N>
struct Vector {
  float4 pad1;
  double pad2;
  vector<T, N> v;
  Vector operator+(Vector vec) {
    Vector ret;
    ret.pad1 = 0.0;
    ret.pad2 = 0.0;
    ret.v = v + vec.v;
    return ret;
  }
};

template<typename T, int N, int M>
struct Matrix {
  float4 pad1;
  matrix<T, N, M> m;
  Matrix operator+(Matrix mat) {
    Matrix ret;
    ret.m = m + mat.m;
    return ret;
  }
};

RWByteAddressBuffer                        BabBuf : register(u1);
RWStructuredBuffer< float2 >               VecBuf : register(u2);
  StructuredBuffer< float[2] >             ArrBuf : register(t3);
  StructuredBuffer< Vector<float, 2> >    SVecBuf : register(t4);
  StructuredBuffer< float2x2 >             MatBuf : register(t5);
  StructuredBuffer< Matrix<float, 2, 2> > SMatBuf : register(t6);

void main(uint ix0 : IX0) {

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 0
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call <2 x i1> @"dx.hl.op.ro.<2 x i1> (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  bool2  Bab0  = BabBuf.Load< bool2 >(ix0 + 0);

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 1
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call [2 x float]* @"dx.hl.op.ro.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Bab1  = (float2)BabBuf.Load< float[2] >(ix0 + 1);

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 2
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call %"struct.Vector<float, 2>"* @"dx.hl.op.ro.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Bab2  = BabBuf.Load< Vector<float,2> >(ix0 + 2).v;

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 3
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call %class.matrix.float.2.2 @"dx.hl.op.ro.%class.matrix.float.2.2 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Bab3  = BabBuf.Load< float2x2 >(ix0 + 3)[1];

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 4
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: [[MSS:%.*]] = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.op.ro.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Bab4  = BabBuf.Load< Matrix<float,2,2> >(ix0 + 4).m[1];

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 5
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 0, %struct.RWByteAddressBuffer
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <2 x float>)"(i32 277, %dx.types.Handle [[ANHDL]], i32 [[IX]], <2 x float>
  BabBuf.Store< float2 >(ix0+5, select(Bab0, Bab1+Bab2, Bab3+Bab4));

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 0
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<vector<float, 2> >" undef)
  // CHECK: call <2 x float> @"dx.hl.op.ro.<2 x float> (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Sld0 = VecBuf.Load(ix0 + 0);

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 1
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<float [2]>\22)"(i32 0, %"class.StructuredBuffer<float [2]>"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<float [2]>\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 12, i32 8 }, %"class.StructuredBuffer<float [2]>" undef)
  // CHECK: call [2 x float]* @"dx.hl.op.ro.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Sld1 = (float2)ArrBuf.Load(ix0 + 1);

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 2
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<Vector<float, 2> >\22)"(i32 0, %"class.StructuredBuffer<Vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<Vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 780, i32 32 }, %"class.StructuredBuffer<Vector<float, 2> >" undef)
  // CHECK: call %"struct.Vector<float, 2>"* @"dx.hl.op.ro.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Sld2 = SVecBuf.Load(ix0 + 2).v;

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 3
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<matrix<float, 2, 2> >\22)"(i32 0, %"class.StructuredBuffer<matrix<float, 2, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 524, i32 16 }, %"class.StructuredBuffer<matrix<float, 2, 2> >" undef)
  // CHECK: call %class.matrix.float.2.2 @"dx.hl.op.ro.%class.matrix.float.2.2 (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Sld3 = MatBuf.Load(ix0 + 3)[1];

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 4
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 0, %"class.StructuredBuffer<Matrix<float, 2, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 524, i32 32 }, %"class.StructuredBuffer<Matrix<float, 2, 2> >" undef)
  // CHECK: [[MSS:%.*]] = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.op.ro.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 231, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  // CHECK: [[GEP:%.*]] = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* [[MSS]], i32 0, i32 1
  // CHECK: call <2 x float>* @"dx.hl.subscript.colMajor[].rn.<2 x float>* (i32, %class.matrix.float.2.2*, i32, i32)"(i32 1, %class.matrix.float.2.2* [[GEP]], i32 1, i32 3)
  float2 Sld4 = SMatBuf.Load(ix0 + 4).m[1];

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 5
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  VecBuf[ix0+5] = select(Sld0, Sld1+Sld2, Sld3+Sld4);
  
  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 6
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]]
  float2 Sss0 = VecBuf[ix0 + 6];

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 7
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<float [2]>\22)"(i32 0, %"class.StructuredBuffer<float [2]>"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<float [2]>\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 12, i32 8 }, %"class.StructuredBuffer<float [2]>" undef)
  // CHECK: call [2 x float]* @"dx.hl.subscript.[].rn.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Sss1 = (float2)ArrBuf[ix0 + 7];

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 8
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<Vector<float, 2> >\22)"(i32 0, %"class.StructuredBuffer<Vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<Vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 780, i32 32 }, %"class.StructuredBuffer<Vector<float, 2> >" undef)
  // CHECK: call %"struct.Vector<float, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  float2 Sss2 = SVecBuf[ix0 + 8].v;

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 9
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<matrix<float, 2, 2> >\22)"(i32 0, %"class.StructuredBuffer<matrix<float, 2, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 524, i32 16 }, %"class.StructuredBuffer<matrix<float, 2, 2> >" undef)
  // CHECK: [[SS:%.*]] = call %class.matrix.float.2.2* @"dx.hl.subscript.[].rn.%class.matrix.float.2.2* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  // CHECK: call <2 x float>* @"dx.hl.subscript.colMajor[].rn.<2 x float>* (i32, %class.matrix.float.2.2*, i32, i32)"(i32 1, %class.matrix.float.2.2* [[SS]], i32 1, i32 3)
  float2 Sss3 = MatBuf[ix0 + 9][1];

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 10
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.StructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 0, %"class.StructuredBuffer<Matrix<float, 2, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.StructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 524, i32 32 }, %"class.StructuredBuffer<Matrix<float, 2, 2> >" undef)
  // CHECK: [[MSS:%.*]] = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  // CHECK: [[GEP:%.*]] = getelementptr inbounds %"struct.Matrix<float, 2, 2>", %"struct.Matrix<float, 2, 2>"* [[MSS]], i32 0, i32 1
  // CHECK: call <2 x float>* @"dx.hl.subscript.colMajor[].rn.<2 x float>* (i32, %class.matrix.float.2.2*, i32, i32)"(i32 1, %class.matrix.float.2.2* [[GEP]], i32 1, i32 3)
  float2 Sss4 = SMatBuf[ix0 + 10].m[1];

  // CHECK: [[IX:%.*]] = add i32 {{%.*}}, 11
  // CHECK: [[HDL:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 0, %"class.RWStructuredBuffer<vector<float, 2> >"
  // CHECK: [[ANHDL:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[HDL]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[ANHDL]], i32 [[IX]])
  VecBuf[ix0+11] = select(Sss0, Sss1+Sss2, Sss3+Sss4);
}
