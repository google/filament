struct Inner {
  @size(64)
  m : mat4x2<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;
fn i() -> i32 { counter++; return counter; }

@compute @workgroup_size(1)
fn f() {
  let p_a           = &a;
  let p_a_i         = &((*p_a)[i()]);
  let p_a_i_a       = &((*p_a_i).a);
  let p_a_i_a_i     = &((*p_a_i_a)[i()]);
  let p_a_i_a_i_m   = &((*p_a_i_a_i).m);
  let p_a_i_a_i_m_i = &((*p_a_i_a_i_m)[i()]);


  let l_a             : array<Outer, 4> =  *p_a;
  let l_a_i           : Outer           =  *p_a_i;
  let l_a_i_a         : array<Inner, 4> =  *p_a_i_a;
  let l_a_i_a_i       : Inner           =  *p_a_i_a_i;
  let l_a_i_a_i_m     : mat4x2<f32>     =  *p_a_i_a_i_m;
  let l_a_i_a_i_m_i   : vec2<f32>       =  *p_a_i_a_i_m_i;
  let l_a_i_a_i_m_i_i : f32             = (*p_a_i_a_i_m_i)[i()];
}
