@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

@group(0) @binding(1) var<storage, read_write> output : f32;

fn foo() {
  if (non_uniform_global < 0) {
    discard;
  }
}

fn bar() {
   output = dpdx(1.0);
}

@fragment
fn main() {
  foo();
  bar();
}
