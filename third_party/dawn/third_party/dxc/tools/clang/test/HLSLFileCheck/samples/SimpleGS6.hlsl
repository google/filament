// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// CHECK: InputPrimitive=lineadj


struct Out
{
  float4 pos : SV_Position;
  float4 a[2] : A;
};

struct Empty{};
int i;

[maxvertexcount(3)]
void main(lineadj Empty e[4], inout PointStream<Out> OutputStream0)
{
  Out output = (Out)0;
  output.a[i] = 3;

  OutputStream0.Append(output);
  OutputStream0.RestartStrip();
}
