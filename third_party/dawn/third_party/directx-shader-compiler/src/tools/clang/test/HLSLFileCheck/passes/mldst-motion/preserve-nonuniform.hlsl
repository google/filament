// RUN: %dxc -T vs_6_0 %s | FileCheck %s

// Make sure we preserve non-uniform through mldst-motion

// CHECK-NOT: call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 {{.*}}, i1 false)
// CHECK: call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 {{.*}}, i1 true)

StructuredBuffer<uint>      g_ObjectIndices[]   : register( t0, space2 );

uint getIndex16Bit( int iInstance, uint uIdx )
{
  uint uIndexToAccess = uIdx;

  if ( ( uIdx & 1 ) == 0 )
  {
    uint uReadIdx = g_ObjectIndices[ NonUniformResourceIndex( iInstance ) ][ NonUniformResourceIndex( uIndexToAccess ) ];
    return uReadIdx & 0xffff;
  }
  else
  {
    uint uReadIdx = g_ObjectIndices[ NonUniformResourceIndex( iInstance ) ][ NonUniformResourceIndex( uIndexToAccess ) ];
    uReadIdx = uReadIdx >> 16;
    return uReadIdx & 0xffff;
  }
}

uint3 GetIndex16Bit( int instance, int primitiveIndex )
{
  uint idx = 3 * primitiveIndex;
  uint uIdx0 = getIndex16Bit( instance, idx );
  uint uIdx1 = getIndex16Bit( instance, idx + 1 );
  uint uIdx2 = getIndex16Bit( instance, idx + 2 );
  return uint3( uIdx0, uIdx1, uIdx2 );
}

uint3 main( int instance : INSTANCE, int primitiveIndex : PRIMINDEX ) : OUT {
  return GetIndex16Bit(instance, primitiveIndex);
}
