alias RTArr = array<i32>;

struct S {
  field0 : RTArr,
}

var<private> x_3 : vec3<u32>;

@group(0) @binding(0) var<storage, read_write> x_6 : S;

@group(0) @binding(1) var<storage, read_write> x_7 : S;

@group(0) @binding(2) var<storage, read_write> x_8 : S;

@group(0) @binding(3) var<storage, read_write> x_9 : S;

fn main_1() {
  let x_26 : u32 = x_3.x;
  let x_28 : i32 = x_6.field0[x_26];
  let x_30 : i32 = x_7.field0[x_26];
  let x_32 : i32 = x_8.field0[x_26];
  x_9.field0[x_26] = bitcast<i32>(clamp(bitcast<u32>(x_28), bitcast<u32>(x_30), bitcast<u32>(x_32)));
  return;
}

@compute @workgroup_size(1, 1, 1)
fn main(@builtin(global_invocation_id) x_3_param : vec3<u32>) {
  x_3 = x_3_param;
  main_1();
}
