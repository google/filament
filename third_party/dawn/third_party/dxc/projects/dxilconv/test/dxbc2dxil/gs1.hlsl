// FXC command line: fxc /T gs_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




struct MyStructIn
{
//  float4 pos : SV_Position;
  float4 a : AAA;
  float2 b : BBB;
  float4 c[3] : CCC;
  //uint d : SV_RenderTargetIndex;
  float4 pos : SV_Position;
};

struct MyStructOut
{
  float4 pos : SV_Position;
  float2 out_a : OUT_AAA;
  uint d : SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void main(triangleadj MyStructIn array[6], inout TriangleStream<MyStructOut> OutputStream0)
{
  float4 r = array[1].a + array[2].b.x + array[3].pos;
  r += array[r.x].c[r.y].w;
  MyStructOut output = (MyStructOut)0;
  output.pos = array[r.x].a;
  output.out_a = array[r.y].b;
  output.d = array[r.x].a + 3;
  OutputStream0.Append(output);
  OutputStream0.RestartStrip();
}

