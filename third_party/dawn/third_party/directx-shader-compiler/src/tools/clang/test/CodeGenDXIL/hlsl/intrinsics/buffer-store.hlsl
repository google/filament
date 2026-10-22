// RUN: %dxc -fcgl  -T vs_6_6 %s | FileCheck %s

// Source file for DxilGen IR test for buffer store lowering

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
RWStructuredBuffer< float[2] >             ArrBuf : register(u3);
RWStructuredBuffer< Vector<float, 2> >    SVecBuf : register(u4);
RWStructuredBuffer< float2x2 >             MatBuf : register(u5);
RWStructuredBuffer< Matrix<float, 2, 2> > SMatBuf : register(u6);

ConsumeStructuredBuffer< float2 >               CVecBuf : register(u7);
ConsumeStructuredBuffer< float[2] >             CArrBuf : register(u8);
ConsumeStructuredBuffer< Vector<float, 2> >    CSVecBuf : register(u9);
ConsumeStructuredBuffer< float2x2 >             CMatBuf : register(u10);
ConsumeStructuredBuffer< Matrix<float, 2, 2> > CSMatBuf : register(u11);

AppendStructuredBuffer< float2 >               AVecBuf : register(u12);
AppendStructuredBuffer< float[2] >             AArrBuf : register(u13);
AppendStructuredBuffer< Vector<float, 2> >    ASVecBuf : register(u14);
AppendStructuredBuffer< float2x2 >             AMatBuf : register(u15);
AppendStructuredBuffer< Matrix<float, 2, 2> > ASMatBuf : register(u16);

void main(uint ix0 : IX0) {

  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 0
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %struct.RWByteAddressBuffer
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call <2 x i1> @"dx.hl.op.ro.<2 x i1> (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 1
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %struct.RWByteAddressBuffer
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, <2 x i1>)"(i32 277, %dx.types.Handle [[anhdl]], i32 [[ix]], <2 x i1>
  BabBuf.Store<bool2>(ix0 + 1, BabBuf.Load< bool2 >(ix0 + 0));

  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 1
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %struct.RWByteAddressBuffer
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call [2 x float]* @"dx.hl.op.ro.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 2
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %struct.RWByteAddressBuffer
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, [2 x float]*)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]], [2 x float]
  BabBuf.Store<float[2]>(ix0 + 2, BabBuf.Load< float[2] >(ix0 + 1));

  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 2
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %struct.RWByteAddressBuffer
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call %"struct.Vector<float, 2>"* @"dx.hl.op.ro.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 3
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %struct.RWByteAddressBuffer
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, %\22struct.Vector<float, 2>\22*)"(i32 277, %dx.types.Handle [[anhdl]], i32 [[ix]], %"struct.Vector<float, 2>"
  BabBuf.Store<Vector<float,2> >(ix0 + 3, BabBuf.Load< Vector<float,2> >(ix0 + 2));

  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 3
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %struct.RWByteAddressBuffer
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call %class.matrix.float.2.2 @"dx.hl.op.ro.%class.matrix.float.2.2 (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 4
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %struct.RWByteAddressBuffer
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, %class.matrix.float.2.2)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]], %class.matrix.float.2.2
  BabBuf.Store<float2x2>(ix0 + 4, BabBuf.Load< float2x2 >(ix0 + 3));

  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 4
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %struct.RWByteAddressBuffer
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: [[MSS:%.*]] = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.op.ro.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 5
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %struct.RWByteAddressBuffer
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %struct.RWByteAddressBuffer)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4107, i32 0 }, %struct.RWByteAddressBuffer undef)
  // CHECK: call void @"dx.hl.op..void (i32, %dx.types.Handle, i32, %\22struct.Matrix<float, 2, 2>\22*)"(i32 277, %dx.types.Handle [[anhdl]], i32 [[ix]], %"struct.Matrix<float, 2, 2>"
  BabBuf.Store<Matrix<float,2,2> >(ix0 + 5, BabBuf.Load< Matrix<float,2,2> >(ix0 + 4));


  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 0
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 {{[0-9]*}}, %"class.RWStructuredBuffer<vector<float, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]]
  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 1
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 {{[0-9]*}}, %"class.RWStructuredBuffer<vector<float, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<vector<float, 2> >\22)"(i32 {{[0-9]*}}, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<vector<float, 2> >" undef)
  // CHECK: call <2 x float>* @"dx.hl.subscript.[].rn.<2 x float>* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  VecBuf[ix0 + 1] = VecBuf[ix0 + 0];

  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 2
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<float [2]>\22)"(i32 {{[0-9]*}}, %"class.RWStructuredBuffer<float [2]>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<float [2]>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<float [2]>" undef)
  // CHECK: call [2 x float]* @"dx.hl.subscript.[].rn.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 1
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<float [2]>\22)"(i32 {{[0-9]*}}, %"class.RWStructuredBuffer<float [2]>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<float [2]>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.RWStructuredBuffer<float [2]>" undef)
  // CHECK: call [2 x float]* @"dx.hl.subscript.[].rn.[2 x float]* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  ArrBuf[ix0 + 2] = ArrBuf[ix0 + 1];

  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 3
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Vector<float, 2> >\22)"(i32 {{[0-9]*}}, %"class.RWStructuredBuffer<Vector<float, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4876, i32 32 }, %"class.RWStructuredBuffer<Vector<float, 2> >" undef)
  // CHECK: call %"struct.Vector<float, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 2
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Vector<float, 2> >\22)"(i32 {{[0-9]*}}, %"class.RWStructuredBuffer<Vector<float, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4876, i32 32 }, %"class.RWStructuredBuffer<Vector<float, 2> >" undef)
  // CHECK: call %"struct.Vector<float, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle, i32)"(i32 0, %dx.types.Handle [[anhdl]], i32 [[ix]])
  SVecBuf[ix0 + 3] = SVecBuf[ix0 + 2];

  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 4
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 {{[0-9]*}}, %"class.RWStructuredBuffer<matrix<float, 2, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 16 }, %"class.RWStructuredBuffer<matrix<float, 2, 2> >" undef)
  // CHECK: [[SS:%.*]] = call %class.matrix.float.2.2* @"dx.hl.subscript.[].rn.%class.matrix.float.2.2* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 3
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 {{[0-9]*}}, %"class.RWStructuredBuffer<matrix<float, 2, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 16 }, %"class.RWStructuredBuffer<matrix<float, 2, 2> >" undef)
  // CHECK: call %class.matrix.float.2.2* @"dx.hl.subscript.[].rn.%class.matrix.float.2.2* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  MatBuf[ix0 + 4] = MatBuf[ix0 + 3];

  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 5
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 {{[0-9]*}}, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 32 }, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >" undef)
  // CHECK: [[MSS:%.*]] = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  // CHECK: [[ix:%.*]] = add i32 {{%.*}}, 4
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.RWStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 {{[0-9]*}}, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.RWStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 32 }, %"class.RWStructuredBuffer<Matrix<float, 2, 2> >" undef)
  // CHECK: call %"struct.Matrix<float, 2, 2>"* @"dx.hl.subscript.[].rn.%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle, i32)"(i32 {{[0-9]*}}, %dx.types.Handle [[anhdl]], i32 [[ix]])
  SMatBuf[ix0 + 5] = SMatBuf[ix0 + 4];

  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<vector<float, 2> >\22)"(i32 0, %"class.ConsumeStructuredBuffer<vector<float, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.ConsumeStructuredBuffer<vector<float, 2> >" undef)
  // CHECK: [[cn:%.*]] = call <2 x float> @"dx.hl.op..consume<2 x float> (i32, %dx.types.Handle)"(i32 283, %dx.types.Handle [[anhdl]])
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<vector<float, 2> >\22)"(i32 0, %"class.AppendStructuredBuffer<vector<float, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.AppendStructuredBuffer<vector<float, 2> >" undef)
  // CHECK: call void @"dx.hl.op..appendvoid (i32, %dx.types.Handle, <2 x float>)"(i32 226, %dx.types.Handle [[anhdl]], <2 x float> [[cn]])
  AVecBuf.Append(CVecBuf.Consume());

  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<float [2]>\22)"(i32 0, %"class.ConsumeStructuredBuffer<float [2]>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<float [2]>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.ConsumeStructuredBuffer<float [2]>" undef)
  // CHECK: [[cn:%.*]] = call [2 x float]* @"dx.hl.op..consume[2 x float]* (i32, %dx.types.Handle)"(i32 283, %dx.types.Handle [[anhdl]])
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<float [2]>\22)"(i32 0, %"class.AppendStructuredBuffer<float [2]>"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<float [2]>\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4108, i32 8 }, %"class.AppendStructuredBuffer<float [2]>" undef)
  // CHECK: call void @"dx.hl.op..appendvoid (i32, %dx.types.Handle, [2 x float]*)"(i32 226, %dx.types.Handle [[anhdl]], [2 x float]*
  AArrBuf.Append(CArrBuf.Consume());

  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<Vector<float, 2> >\22)"(i32 0, %"class.ConsumeStructuredBuffer<Vector<float, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<Vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4876, i32 32 }, %"class.ConsumeStructuredBuffer<Vector<float, 2> >" undef)
  // CHECK: [[cn:%.*]] = call %"struct.Vector<float, 2>"* @"dx.hl.op..consume%\22struct.Vector<float, 2>\22* (i32, %dx.types.Handle)"(i32 283, %dx.types.Handle [[anhdl]])
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<Vector<float, 2> >\22)"(i32 0, %"class.AppendStructuredBuffer<Vector<float, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<Vector<float, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4876, i32 32 }, %"class.AppendStructuredBuffer<Vector<float, 2> >" undef)
  // CHECK: call void @"dx.hl.op..appendvoid (i32, %dx.types.Handle, %\22struct.Vector<float, 2>\22*)"(i32 226, %dx.types.Handle [[anhdl]], %"struct.Vector<float, 2>"*
  ASVecBuf.Append(CSVecBuf.Consume());

  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 0, %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 16 }, %"class.ConsumeStructuredBuffer<matrix<float, 2, 2> >" undef)
  // CHECK: [[cn:%.*]] = call %class.matrix.float.2.2 @"dx.hl.op..consume%class.matrix.float.2.2 (i32, %dx.types.Handle)"(i32 283, %dx.types.Handle [[anhdl]])
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 0, %"class.AppendStructuredBuffer<matrix<float, 2, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 16 }, %"class.AppendStructuredBuffer<matrix<float, 2, 2> >" undef)
  // CHECK: call void @"dx.hl.op..appendvoid (i32, %dx.types.Handle, %class.matrix.float.2.2)"(i32 226, %dx.types.Handle [[anhdl]], %class.matrix.float.2.2 [[cn]])
  AMatBuf.Append(CMatBuf.Consume());

  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 0, %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 32 }, %"class.ConsumeStructuredBuffer<Matrix<float, 2, 2> >" undef)
  // CHECK: [[cn:%.*]] = call %"struct.Matrix<float, 2, 2>"* @"dx.hl.op..consume%\22struct.Matrix<float, 2, 2>\22* (i32, %dx.types.Handle)"(i32 283, %dx.types.Handle [[anhdl]])
  // CHECK: [[hdl:%.*]] = call %dx.types.Handle @"dx.hl.createhandle..%dx.types.Handle (i32, %\22class.AppendStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 0, %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >"
  // CHECK: [[anhdl:%.*]] = call %dx.types.Handle @"dx.hl.annotatehandle..%dx.types.Handle (i32, %dx.types.Handle, %dx.types.ResourceProperties, %\22class.AppendStructuredBuffer<Matrix<float, 2, 2> >\22)"(i32 14, %dx.types.Handle [[hdl]], %dx.types.ResourceProperties { i32 4620, i32 32 }, %"class.AppendStructuredBuffer<Matrix<float, 2, 2> >" undef)
  // CHECK: call void @"dx.hl.op..appendvoid (i32, %dx.types.Handle, %\22struct.Matrix<float, 2, 2>\22*)"(i32 226, %dx.types.Handle [[anhdl]], %"struct.Matrix<float, 2, 2>"*
  ASMatBuf.Append(CSMatBuf.Consume());

}
