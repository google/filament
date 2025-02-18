struct S {
  /* @offset(0) */
  a : vec4f,
  /* @offset(16) */
  b : i32,
}

alias RTArr = array<S>;

struct sb_block {
  /* @offset(0) */
  inner : RTArr,
}

@group(0) @binding(0) var<storage, read> sb : sb_block;

fn main_1() {
  let x_18 = sb.inner[1i];
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
