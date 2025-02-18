alias RTArr = array<u32>;

struct S {
  field0 : RTArr,
}

var<private> x_2 : vec3<u32>;

@group(0) @binding(0) var<storage, read_write> x_5 : S;

@group(0) @binding(1) var<storage, read_write> x_6 : S;

@group(0) @binding(2) var<storage, read_write> x_7 : S;

fn main_1() {
  let x_21 : u32 = x_2.x;
  let x_23 : u32 = x_5.field0[x_21];
  let x_25 : u32 = x_6.field0[x_21];
  x_7.field0[x_21] = select(0u, 1u, (bitcast<i32>(x_23) > bitcast<i32>(x_25)));
  return;
}

@compute @workgroup_size(1, 1, 1)
fn main(@builtin(global_invocation_id) x_2_param : vec3<u32>) {
  x_2 = x_2_param;
  main_1();
}
