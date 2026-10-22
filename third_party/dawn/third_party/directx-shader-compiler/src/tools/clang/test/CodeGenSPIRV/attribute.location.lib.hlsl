// RUN: %dxc -T lib_6_6 %s -spirv | FileCheck %s


// The inputs to the vertex shader should be at location 0 and 1 to match
// the location attributes.
// CHECK-DAG: OpDecorate %in_var_Position Location 0
// CHECK-DAG: OpDecorate %in_var_Color Location 1
struct Vertex {
    [[vk::location(0)]] float3 position : Position;
    [[vk::location(1)]] float4 color : Color;
};

// The "color" output to the vertex shader should match the "color" input to the
// pixel shader. The should both be at location 1 to match the attribute.
// CHECK-DAG: OpDecorate %out_var_Color Location 1
// CHECK-DAG: OpDecorate %in_var_Color_0 Location 1
struct Fragment {
    [[vk::location(0)]] float4 sv_position : SV_Position;
    [[vk::location(1)]] float4 color : Color;
};

[shader("vertex")]
Fragment vsmain(in Vertex vertex) {
    Fragment o;
    
    o.sv_position = float4(vertex.position, 1);
    o.color = vertex.color;

    return o;
}

// This location should set the location of the pixel shader output to location 0.
// CHECK-DAG: OpDecorate %out_var_SV_Target0 Location 0
[[vk::location(0)]]
[shader("pixel")]
float4 psmain(in Fragment fragment) : SV_Target0 {
    return fragment.color;
}
