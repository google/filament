@group(0) @binding(0) var<uniform> u : array<mat4x2<f32>, 4>;

@group(0) @binding(1) var<storage, read_write> s : f32;

@compute @workgroup_size(1)
fn f() {
  let t = transpose(u[2]);
  let l = length(u[0][1].yx);
  let a = abs(u[0][1].yx.x);
  s = ((t[0].x + f32(l)) + f32(a));
}
