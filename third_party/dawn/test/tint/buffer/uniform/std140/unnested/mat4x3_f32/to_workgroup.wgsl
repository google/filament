@group(0) @binding(0) var<uniform> u : mat4x3<f32>;
var<workgroup> w : mat4x3<f32>;

@compute @workgroup_size(1)
fn f() {
    w = u;
    w[1] = u[0];
    w[1] = u[0].zxy;
    w[0][1] = u[1][0];
}
