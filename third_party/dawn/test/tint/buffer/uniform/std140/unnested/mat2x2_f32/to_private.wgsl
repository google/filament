@group(0) @binding(0) var<uniform> u : mat2x2<f32>;
var<private> p : mat2x2<f32>;

@compute @workgroup_size(1)
fn f() {
    p = u;
    p[1] = u[0];
    p[1] = u[0].yx;
    p[0][1] = u[1][0];
}
