@group(0) @binding(0) var<storage, read> G : array<i32>;

@compute @workgroup_size(1)
fn main() {
    let l1 : u32 = arrayLength(&G);
}
