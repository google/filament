// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s



// Make sure no store is generated.
// CHECK-NOT:store {{.*}},

struct ST
{
    float4 a;
    float4 b;
    float4 c;
};


cbuffer cbModelSkinningConstants : register ( b4 )
{
    float4 v[ 2 * 256 * 3 ];

    static const float4 v2d[ 512 ] [ 3 ] = v ;
    static const ST vst[ 512 ] = v;
} ;


float4 main(int i:I) : SV_Target {
  return v2d[i][1] + vst[i].b;
}