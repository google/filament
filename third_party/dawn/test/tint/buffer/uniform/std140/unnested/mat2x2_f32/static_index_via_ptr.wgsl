@group(0) @binding(0) var<uniform> m : mat2x2<f32>;

var<private> counter = 0;
fn i() -> i32 { counter++; return counter; }

@compute @workgroup_size(1)
fn f() {
  let p_m   = &m;
  let p_m_1 = &((*p_m)[1]);

  let l_m   : mat2x2<f32> = *p_m;
  let l_m_1 : vec2<f32>   = *p_m_1;
}
