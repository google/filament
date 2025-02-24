@group(0) @binding(0) var<uniform> u : mat4x4<f32>;
var<private> p : mat4x4<f32>;

@compute @workgroup_size(1)
fn f() {
    p = u;
    p[1] = u[0];
    p[1] = u[0].ywxz;
    p[0][1] = u[1][0];
}
