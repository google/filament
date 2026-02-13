// RUN: %dxc -E main -DTYPE=uint -T cs_6_0 %s | FileCheck %s
// RUN: %dxc -E main -DTYPE=int -T cs_6_0 %s | FileCheck %s
// RUN: %dxc -E main -DTYPE=uint64_t -T cs_6_6 %s | FileCheck %s
// RUN: %dxc -E main -DTYPE=int64_t -T cs_6_6 %s | FileCheck %s
// RUN: %dxc -E main -DTYPE=int -T cs_6_0 -ast-dump-implicit %s | FileCheck %s -check-prefix=AST

// CHECK: Compute Shader
// CHECK: NumThreads=(8,8,1)
// CHECK: atomicrmw add
// CHECK: atomicrmw add
// CHECK: cmpxchg
// CHECK: cmpxchg
// CHECK: atomicBinOp
// CHECK: atomicBinOp
// CHECK: atomicCompareExchange
// CHECK: atomicCompareExchange
// CHECK: atomicBinOp
// CHECK: atomicBinOp
// CHECK: atomicCompareExchange
// CHECK: atomicCompareExchange
// CHECK: atomicBinOp
// CHECK: atomicCompareExchange
// CHECK: atomicCompareExchange
// CHECK: AtomicAdd


RWByteAddressBuffer rawBuf0 : register( u0 );

#define _TOTUPLE(type) type##2
#define TOTUPLE(type) _TOTUPLE(type)

struct Foo
{
  float2 a;
  float3 b;
  TYPE   u;
  TOTUPLE(TYPE) c[4];
  TYPE d[4];
};
RWStructuredBuffer<Foo> structBuf1 : register( u1 );
RWTexture2D<TYPE> rwTex2: register( u2 );

groupshared TYPE shareMem[256];

[numthreads( 8, 8, 1 )]
void main( uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID )
{
    shareMem[GI] = 0;

    GroupMemoryBarrierWithGroupSync();
    TYPE v;

    InterlockedAdd( shareMem[DTid.x], 1 );
    InterlockedAdd( shareMem[DTid.x], 1, v );
    InterlockedCompareStore( shareMem[DTid.x], 1, v );
    InterlockedCompareExchange( shareMem[DTid.x], 1, 2, v );

    InterlockedAdd( rwTex2[DTid.xy], v );
    InterlockedAdd( rwTex2[DTid.xy], 1, v );
    InterlockedCompareStore( rwTex2[DTid.xy], 1, v );
    InterlockedCompareExchange( rwTex2[DTid.xy], 1, 2, v );

    InterlockedAdd( structBuf1[DTid.z].u, v);
    InterlockedAdd( structBuf1[DTid.z].u, 1, v);
    InterlockedCompareStore( structBuf1[DTid.z].u, 1, v);
    InterlockedCompareExchange( structBuf1[DTid.z].u, 1, 2, v);

    GroupMemoryBarrierWithGroupSync();

    rawBuf0.InterlockedAdd( GI * 4, shareMem[GI], v );
    rawBuf0.InterlockedCompareStore( GI * 4, shareMem[GI], v );
    rawBuf0.InterlockedCompareExchange( GI * 4, shareMem[GI], 2, v );
    rawBuf0.InterlockedAdd( GI * 4, v );

	// Special case: vector component access operation on scalars; Github issue# 2434
	// CHECK: atomicBinOp
	InterlockedAdd(structBuf1[DTid.z].u.x, 1);

	// CHECK: atomicBinOp
	InterlockedAdd(structBuf1[DTid.z].d[1].r, 1);

	// CHECK: atomicBinOp
	InterlockedAdd(structBuf1[DTid.z].d[1].r, 1);

	// CHECK: atomicBinOp
	InterlockedAdd(rwTex2[DTid.xy].x, 1);

	// CHECK: atomicrmw
	InterlockedAdd(shareMem[DTid.z].r, 1);
}

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit InterlockedAdd 'void (unsigned long long &, unsigned long long)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'unsigned long long &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'unsigned long long'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit used InterlockedAdd 'void (int &, unsigned int)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'int &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'unsigned int'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit InterlockedAdd 'void (unsigned long long &, unsigned long long, long long &)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'unsigned long long &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'unsigned long long'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} original 'long long &&__restrict'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit used InterlockedAdd 'void (int &, unsigned int, int &)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'int &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'unsigned int'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} original 'int &&__restrict'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit InterlockedCompareStore 'void (unsigned long long &, unsigned long long, unsigned long long)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'unsigned long long &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} compare 'unsigned long long'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'unsigned long long'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit used InterlockedCompareStore 'void (int &, unsigned int, unsigned int)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'int &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} compare 'unsigned int'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'unsigned int'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit InterlockedCompareExchange 'void (unsigned long long &, unsigned long long, unsigned long long, long long &)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'unsigned long long &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} compare 'unsigned long long'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'unsigned long long'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} original 'long long &&__restrict'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit

// AST: FunctionDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} implicit used InterlockedCompareExchange 'void (int &, unsigned int, unsigned int, int &)' extern
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} result 'int &'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} compare 'unsigned int'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} value 'unsigned int'
// AST-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} original 'int &&__restrict'
// AST-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}} Implicit "op" ""
// AST-NEXT: HLSLBuiltinCallAttr {{0x[0-9a-fA-F]+}} {{[<>a-z ]+}}> Implicit
