@group(0) @binding(0) var<uniform> m : mat2x3<f32>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_m = &(m);
  let p_m_i = &((*(p_m))[i()]);
  let l_m : mat2x3<f32> = *(p_m);
  let l_m_i : vec3<f32> = *(p_m_i);
}
