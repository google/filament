alias RTArr = array<u32>;

struct S {
  field0 : RTArr,
}

var<private> x_3 : vec3<u32>;

@group(0) @binding(0) var<storage, read_write> x_6 : S;

@group(0) @binding(1) var<storage, read_write> x_7 : S;

@group(0) @binding(2) var<storage, read_write> x_8 : S;

fn main_1() {
  let x_21 : u32 = x_3.x;
  let x_23 : u32 = x_6.field0[x_21];
  let x_25 : u32 = x_7.field0[x_21];
  x_8.field0[x_21] = bitcast<u32>(min(bitcast<i32>(x_23), bitcast<i32>(x_25)));
  return;
}

@compute @workgroup_size(1, 1, 1)
fn main(@builtin(global_invocation_id) x_3_param : vec3<u32>) {
  x_3 = x_3_param;
  main_1();
}
