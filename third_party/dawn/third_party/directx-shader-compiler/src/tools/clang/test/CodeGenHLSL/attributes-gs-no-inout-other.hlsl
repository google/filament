// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// CHECK: 10:10: error: stream-output object must be an inout parameter

struct GsOut {
    float4 pos : SV_Position;
};

// Missing inout on param
void foo(LineStream<GsOut> param) {
    GsOut vertex;
    vertex = (GsOut)0;
    param.Append(vertex);
}

[maxvertexcount(3)]
void main(in triangle float4 pos[3] : SV_Position,
          inout LineStream<GsOut> outData) {
    GsOut vertex;
    vertex.pos = pos[0];
    outData.Append(vertex);

    foo(outData);

    outData.RestartStrip();
}
