// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// CHECK: :16:6: error: GS entry point must have a valid maxvertexcount attribute

struct GsOut {
    float4 pos : SV_Position;
};

void foo(inout LineStream<GsOut> param) {
    GsOut vertex;
    vertex = (GsOut)0;
    param.Append(vertex);
}

// Missing maxvertexcount attribute
void main(in triangle float4 pos[3] : SV_Position,
          inout LineStream<GsOut> outData) {
    GsOut vertex;
    vertex.pos = pos[0];
    outData.Append(vertex);

    foo(outData);

    outData.RestartStrip();
}
