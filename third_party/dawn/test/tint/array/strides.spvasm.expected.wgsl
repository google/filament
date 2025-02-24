struct strided_arr {
  @size(8)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

alias Arr_1 = array<Arr, 3u>;

struct strided_arr_1 {
  @size(128)
  el : Arr_1,
}

alias Arr_2 = array<strided_arr_1, 4u>;

struct S {
  /* @offset(0) */
  a : Arr_2,
}

@group(0) @binding(0) var<storage, read_write> s : S;

fn f_1() {
  let x_19 = s.a;
  let x_24 = s.a[3i].el;
  let x_28 = s.a[3i].el[2i];
  let x_32 = s.a[3i].el[2i][1i].el;
  s.a = array<strided_arr_1, 4u>();
  s.a[3i].el[2i][1i].el = 5.0f;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn f() {
  f_1();
}
