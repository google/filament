@group(0) @binding(0) var<storage, read_write> s: i32;

const a : i32 = 1;
const _a : i32 = 2;

@compute @workgroup_size(1)
fn f() {
    const b = a;
    const _b = _a;

    s = b + _b;
}
