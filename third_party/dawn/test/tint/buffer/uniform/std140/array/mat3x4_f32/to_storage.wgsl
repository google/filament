@group(0) @binding(0) var<uniform> u : array<mat3x4<f32>, 4>;
@group(0) @binding(1) var<storage, read_write> s : array<mat3x4<f32>, 4>;

@compute @workgroup_size(1)
fn f() {
    s = u;
    s[1] = u[2];
    s[1][0] = u[0][1].ywxz;
    s[1][0].x = u[0][1].x;
}
