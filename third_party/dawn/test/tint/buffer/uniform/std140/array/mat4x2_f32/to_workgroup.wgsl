@group(0) @binding(0) var<uniform> u : array<mat4x2<f32>, 4>;
var<workgroup> w : array<mat4x2<f32>, 4>;

@compute @workgroup_size(1)
fn f() {
    w = u;
    w[1] = u[2];
    w[1][0] = u[0][1].yx;
    w[1][0].x = u[0][1].x;
}
