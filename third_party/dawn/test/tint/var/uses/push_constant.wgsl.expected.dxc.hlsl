SKIP: INVALID


enable chromium_experimental_push_constant;

var<push_constant> a : i32;

var<push_constant> b : i32;

var<push_constant> c : i32;

fn uses_a() {
  let foo = a;
}

fn uses_uses_a() {
  uses_a();
}

fn uses_b() {
  let foo = b;
}

@compute @workgroup_size(1)
fn main1() {
  uses_a();
}

@compute @workgroup_size(1)
fn main2() {
  uses_uses_a();
}

@compute @workgroup_size(1)
fn main3() {
  uses_b();
}

@compute @workgroup_size(1)
fn main4() {
}

Failed to generate: error: unhandled address space push_constant

tint executable returned error: exit status 1
