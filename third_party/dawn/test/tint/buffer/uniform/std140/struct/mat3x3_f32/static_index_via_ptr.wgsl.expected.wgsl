struct Inner {
  @size(64)
  m : mat3x3<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_3 = &((*(p_a))[3]);
  let p_a_3_a = &((*(p_a_3)).a);
  let p_a_3_a_2 = &((*(p_a_3_a))[2]);
  let p_a_3_a_2_m = &((*(p_a_3_a_2)).m);
  let p_a_3_a_2_m_1 = &((*(p_a_3_a_2_m))[1]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_3 : Outer = *(p_a_3);
  let l_a_3_a : array<Inner, 4> = *(p_a_3_a);
  let l_a_3_a_2 : Inner = *(p_a_3_a_2);
  let l_a_3_a_2_m : mat3x3<f32> = *(p_a_3_a_2_m);
  let l_a_3_a_2_m_1 : vec3<f32> = *(p_a_3_a_2_m_1);
  let l_a_3_a_2_m_1_0 : f32 = (*(p_a_3_a_2_m_1))[0];
}
