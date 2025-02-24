@group(0) @binding(0) var<uniform> a : array<mat2x3<f32>, 4>;
@group(0) @binding(1) var<storage, read_write> s: f32;

@compute @workgroup_size(1)
fn f() {
  let p_a       = &a;
  let p_a_2     = &((*p_a)[2]);
  let p_a_2_1   = &((*p_a_2)[1]);

  let l_a       : array<mat2x3<f32>, 4> = *p_a;
  let l_a_i     : mat2x3<f32>           = *p_a_2;
  let l_a_i_i   : vec3<f32>             = *p_a_2_1;

  s = (*p_a_2_1).x + l_a[0][0].x + l_a_i[0].x + l_a_i_i.x;
}
