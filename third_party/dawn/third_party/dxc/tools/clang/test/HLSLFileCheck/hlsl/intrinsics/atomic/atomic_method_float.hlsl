// RUN: %dxc -T ps_6_0 %s | FileCheck %s -check-prefix=CHECK

// Test float atomic cmp and xchg atomic methods

RWByteAddressBuffer res;

float4 main( uint a : A, uint b: B, uint c :C) : SV_Target
{
  int iv = b - c;
  int iv2 = b + c;
  float fv = b/c;
  float fv2 = b*c;
  float ofv = 0;
  int oiv = 0;
  uint ix = 0;

  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  res.InterlockedExchangeFloat( ix++, fv, ofv );
  res.InterlockedExchangeFloat( ix++, iv, oiv );
  res.InterlockedExchangeFloat( ix++, iv2, ofv );
  res.InterlockedExchangeFloat( ix++, fv2, oiv );

  fv = ofv*0.1;
  fv2 = ofv*2;
  iv = oiv*0.1;
  iv2 = oiv*2;
  int iv3 = oiv*3;
  int iv4 = oiv*4;
  float fv3 = ofv*3;
  float fv4 = ofv*4;

  // CHECK: bitcast float
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: bitcast float
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: bitcast float
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: bitcast float
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  res.InterlockedCompareStoreFloatBitwise( ix++, fv, fv2 );
  res.InterlockedCompareStoreFloatBitwise( ix++, iv, iv2 );
  res.InterlockedCompareStoreFloatBitwise( ix++, iv3, fv4 );
  res.InterlockedCompareStoreFloatBitwise( ix++, fv3, iv4 );

  fv = ofv/6;
  fv2 = ofv/2;
  iv = oiv/6;
  iv2 = oiv/2;
  iv3 = oiv/3;
  iv4 = oiv/4;
  fv3 = ofv/3;
  fv4 = ofv/4;

  // CHECK: bitcast float
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: bitcast float
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: bitcast float
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: bitcast float
  // CHECK: bitcast float
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  res.InterlockedCompareExchangeFloatBitwise( ix++, fv, fv2, ofv );
  res.InterlockedCompareExchangeFloatBitwise( ix++, iv, iv2, oiv );
  res.InterlockedCompareExchangeFloatBitwise( ix++, iv3, fv4, ofv );
  res.InterlockedCompareExchangeFloatBitwise( ix++, fv3, iv4, oiv );

  // Test literals
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle {{%?[A-Za-z0-9_]*}}, i32 {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1065353216)
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle {{%?[A-Za-z0-9_]*}}, i32 {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1073741824)
  res.InterlockedExchangeFloat( ix++, 1.0, ofv );
  res.InterlockedExchangeFloat( ix++, 2, oiv );


  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[A-Za-z0-9_]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1065353216, i32 1073741824)
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[A-Za-z0-9_]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[0-9]*}}, i32 1073741824)
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[A-Za-z0-9_]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1065353216, i32 {{%?[0-9]*}})
  res.InterlockedCompareStoreFloatBitwise( ix++, 1.0, 2.0 );
  res.InterlockedCompareStoreFloatBitwise( ix++, iv, 2 );
  res.InterlockedCompareStoreFloatBitwise( ix++, 1, fv4 );

  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[A-Za-z0-9_]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1065353216, i32 1073741824)
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[A-Za-z0-9_]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1065353216, i32 {{%?[0-9]*}})
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[A-Za-z0-9_]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[0-9]*}}, i32 1073741824)
  res.InterlockedCompareExchangeFloatBitwise( ix++, 1.0, 2.0, ofv );
  res.InterlockedCompareExchangeFloatBitwise( ix++, 1.0, iv2, oiv );
  res.InterlockedCompareExchangeFloatBitwise( ix++, iv3, 2.0, ofv );
  

  return ofv;
}
