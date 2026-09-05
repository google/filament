// RUN: %dxc -E CSMain -T cs_6_6 %s | FileCheck %s -check-prefix=GSCHECK
// RUN: %dxc -T ps_6_6 -DMEMTYPE=RWBuffer %s | FileCheck %s -check-prefixes=CHECK,TYCHECK
// RUN: %dxc -T ps_6_6 -DMEMTYPE=RWStructuredBuffer %s | FileCheck %s -check-prefix=CHECK
// RUN: %dxc -E CSMain -T cs_6_6 -ast-dump-implicit %s | FileCheck %s -check-prefix=AST

#ifdef MEMTYPE
MEMTYPE<float>     resF;
MEMTYPE<int>       resI;
MEMTYPE<uint64_t>  resI64;
#else
groupshared float    resF[256];
groupshared int      resI[256];
groupshared int64_t  resI64[256];
#endif

float4 dotest( uint a, uint b, uint c)
{
  float fv = b - c;
  float fv2 = b + c;
  float ofv = 0;
  int iv = b / c;
  int iv2 = b * c;
  int oiv = 0;
  uint64_t bb = b;
  uint64_t cc = c;
  uint64_t lv = bb * cc;
  uint64_t lv2 = bb / cc;
  uint64_t olv = 0;

  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i64
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  InterlockedExchange( resF[a], fv, ofv);
  InterlockedExchange( resI[a], iv, iv2 );
  InterlockedExchange( resI64[a], lv, lv2);

  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i32
  // GSCHECK: atomicrmw xchg i64
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i32 @dx.op.atomicBinOp.i32
  // CHECK: call i64 @dx.op.atomicBinOp.i64
  InterlockedExchange( resF[a], iv, iv2 );
  InterlockedExchange( resF[a], fv, iv2 );
  InterlockedExchange( resF[a], iv, fv2 );
  InterlockedExchange( resI[a], fv, fv2 );
  InterlockedExchange( resI64[a], fv, fv2 );

  // GSCHECK: cmpxchg i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  InterlockedCompareStoreFloatBitwise( resF[a], fv, fv2);

  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  InterlockedCompareStoreFloatBitwise( resF[a], iv, iv2 );
  InterlockedCompareStoreFloatBitwise( resF[a], fv, iv2 );
  InterlockedCompareStoreFloatBitwise( resF[a], iv, fv2 );

  // GSCHECK: cmpxchg i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  InterlockedCompareExchangeFloatBitwise( resF[a], fv, fv2, ofv);

  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // GSCHECK: cmpxchg i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32
  InterlockedCompareExchangeFloatBitwise( resF[a], iv, iv2, ofv );
  InterlockedCompareExchangeFloatBitwise( resF[a], fv, iv2, ofv );
  InterlockedCompareExchangeFloatBitwise( resF[a], iv, fv2, ofv );

  // Test literals
  // GSCHECK: atomicrmw xchg i32 addrspace(3)* {{%?[0-9]*}}, i32 1065353216
  // GSCHECK: atomicrmw xchg i32 addrspace(3)* {{%?[0-9]*}}, i32 1073741824
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1065353216)
  // CHECK: call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1073741824)
  InterlockedExchange( resF[a], 1.0, ofv );
  InterlockedExchange( resF[a], 2, oiv );


  // GSCHECK: cmpxchg i32 addrspace(3)* {{%?[0-9]*}}, i32 1065353216, i32 1073741824
  // GSCHECK: cmpxchg i32 addrspace(3)* {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 1073741824
  // GSCHECK: cmpxchg i32 addrspace(3)* {{%?[0-9]*}}, i32 1065353216, i32 {{%?[0-9]*}}
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1065353216, i32 1073741824)
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[0-9]*}}, i32 1073741824)
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1065353216, i32 {{%?[0-9]*}})
  InterlockedCompareStoreFloatBitwise( resF[a], 1.0, 2.0 );
  InterlockedCompareStoreFloatBitwise( resF[a], iv, 2 );
  InterlockedCompareStoreFloatBitwise( resF[a], 1, fv2 );

  // GSCHECK: cmpxchg i32 addrspace(3)* {{%?[0-9]*}}, i32 1065353216, i32 1073741824
  // GSCHECK: cmpxchg i32 addrspace(3)* {{%?[0-9]*}}, i32 1065353216, i32 {{%?[0-9]*}}
  // GSCHECK: cmpxchg i32 addrspace(3)* {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 1073741824
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1065353216, i32 1073741824)
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 1065353216, i32 {{%?[0-9]*}})
  // CHECK: call i32 @dx.op.atomicCompareExchange.i32(i32 79, %dx.types.Handle {{%?[0-9]*}}, i32 {{%?[0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[A-Za-z0-9]*}}, i32 {{%?[0-9]*}}, i32 1073741824)
  InterlockedCompareExchangeFloatBitwise( resF[a], 1.0, 2.0, ofv );
  InterlockedCompareExchangeFloatBitwise( resF[a], 1.0, iv2, oiv );
  InterlockedCompareExchangeFloatBitwise( resF[a], iv2, 2.0, ofv );

  return ofv;
}

float4 main( uint a : A, uint b: B, uint c :C) : SV_Target
{
  return dotest(a,b,c);
}

RWStructuredBuffer<float4> output;
[numthreads(1,1,1)]
void CSMain( uint3 gtid : SV_GroupThreadID, uint ix : SV_GroupIndex)
{
  output[ix] = dotest(gtid.x, gtid.y, gtid.z);
}


// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit InterlockedExchange 'void (unsigned long long &, long long, long long &)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'unsigned long long &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'long long'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} original 'long long &&__restrict'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit used InterlockedExchange 'void (float &, float, float &)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'float &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'float'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} original 'float &&__restrict'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit used InterlockedExchange 'void (int &, unsigned int, int &)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'int &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'unsigned int'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} original 'int &&__restrict'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit used InterlockedExchange 'void (long long &, unsigned long long, unsigned long long &)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'long long &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'unsigned long long'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} original 'unsigned long long &&__restrict'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit used InterlockedExchange 'void (long long &, long long, long long &)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'long long &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'long long'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} original 'long long &&__restrict'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit used InterlockedCompareStoreFloatBitwise 'void (float &, float, float)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'float &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} compare 'float'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'float'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit used InterlockedCompareExchangeFloatBitwise 'void (float &, float, float, float &)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'float &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} compare 'float'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'float'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} original 'float &&__restrict'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit
