// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// CHECK: 18:11: error: stream-output object must be an inout parameter

struct GsOut {
    float4 pos : SV_Position;
};

void foo(inout LineStream<GsOut> param) {
    GsOut vertex;
    vertex = (GsOut)0;
    param.Append(vertex);
}

// Missing inout on outData
[maxvertexcount(3)]
void main(in triangle float4 pos[3] : SV_Position,
          LineStream<GsOut> outData) {
    GsOut vertex;
    vertex.pos = pos[0];
    outData.Append(vertex);

    foo(outData);

    outData.RestartStrip();
}
