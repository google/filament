enable f16;

@group(0) @binding(0) var<uniform> a : array<mat4x4<f16>, 4>;

@group(0) @binding(1) var<storage, read_write> s : f16;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_i = &((*(p_a_i))[i()]);
  let l_a : array<mat4x4<f16>, 4> = *(p_a);
  let l_a_i : mat4x4<f16> = *(p_a_i);
  let l_a_i_i : vec4<f16> = *(p_a_i_i);
  s = ((((*(p_a_i_i)).x + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x);
}
