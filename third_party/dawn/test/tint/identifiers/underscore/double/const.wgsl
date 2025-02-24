@group(0) @binding(0) var<storage, read_write> s: i32;

const a : i32 = 1;
const a__ : i32 = 2;

@compute @workgroup_size(1)
fn f() {
    const b = a;
    const b__ = a__;

    s = b + b__;
}
