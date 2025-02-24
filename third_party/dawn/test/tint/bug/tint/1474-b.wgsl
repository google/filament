// Example taken from https://github.com/gpuweb/gpuweb/pull/2622
@group(0) @binding(0) var<storage, read_write> non_uniform_value : i32;

@compute @workgroup_size(1,1,1)
fn main() {
    return;
    let non_uniform_cond = non_uniform_value == 0;
    if (non_uniform_cond) {
        workgroupBarrier(); // valid, unreachable code does not contribute
                            // to the uniformity analysis
    }
}
