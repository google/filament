// RUN: %dxilver 1.7 | %dxc -E main -T gs_6_1 %s | FileCheck %s

// dxilver 1.7 because PSV0 data was incorrectly filled in before this point,
// making this test fail if running against prior validator versions.

// CHECK: Number of inputs: 12, outputs per stream: { 4, 4, 0, 0 }
// CHECK: Outputs for Stream 0 dependent on ViewId: { 1, 2, 3 }
// CHECK: Outputs for Stream 1 dependent on ViewId: { 0, 2 }
// CHECK: Inputs contributing to computation of Outputs for Stream 0:
// CHECK:   output 1 depends on inputs: { 0 }
// CHECK:   output 2 depends on inputs: { 1 }
// CHECK:   output 3 depends on inputs: { 2 }
// CHECK: Inputs contributing to computation of Outputs for Stream 1:
// CHECK:   output 0 depends on inputs: { 8 }
// CHECK:   output 1 depends on inputs: { 9 }
// CHECK:   output 2 depends on inputs: { 10 }
// CHECK:   output 3 depends on inputs: { 11 }

struct InVertex {
  float4 a1 : SV_Position;
  float2 a2 : AAA2;
  float4 a3 : AAA3;
};

struct OutVertex1 {
  float4 b : BBB;
};

struct OutVertex2 {
  float4 c : CCC;
};

[maxvertexcount(3)]
void main( 
  triangle InVertex verts[3], 
  uint vid  : SV_ViewID,
  inout PointStream<OutVertex1> myStream1, 
  inout PointStream<OutVertex2> myStream2 )
{
    OutVertex1 myVert1;
    OutVertex2 myVert2;
    myVert1.b.yzw = verts[0].a1.xyz + vid;
    myVert2.c = verts[1].a3;
    myVert2.c.x += vid;
    [branch]
    if (vid == 2) {
     myVert2.c.z = 5;
    }
    myStream1.Append( myVert1 );
    myStream2.Append( myVert2 );
}