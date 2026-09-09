// RUN: %dxc -E main  -T cs_6_0 %s  | FileCheck %s

RWBuffer<double> uav1;
RWTexture2D<uint64_t> uav2;

RWTexture1D<double2> uav3;

struct Foo
{
  double2 a;
  int64_t b;
  uint64_t4 c;
};

StructuredBuffer<Foo> buf1;
RWStructuredBuffer<Foo> buf2;

[ numthreads( 64, 2, 2 ) ]
void main( uint GI : SV_GroupIndex)
{
    // CHECK: splitdouble
    uav1[GI] = GI;

    uav2[GI.xx] = GI+1;
    // CHECK: splitDouble
    uav3[GI] = GI+2;

    // CHECK: makeDouble
    buf2[GI] = buf1[GI];

    // CHECK: zext
    // CHECK: zext
    // CHECK: shl
    // CHECK: or
    // CHECK: 6
    // CHECK: trunc
    // CHECK: lshr
    // CHECK: trunc
    buf2[GI+1].b = buf1[GI].b + 6;
    // CHECK: makeDouble
    // CHECK: splitdouble
    buf2[GI+2].a = buf1[GI].a;
}
