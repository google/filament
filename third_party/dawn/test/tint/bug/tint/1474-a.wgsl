// Example taken from https://github.com/gpuweb/gpuweb/pull/2622
@compute @workgroup_size(1,1,1)
fn main() {
    const cond = true;
    while (cond) {
        if cond {
            break;
        } else {
            return;
        } // Overall behavior is {Break, Return}
    }
    let x = 5; // valid, but behavior is {Break, Return}
}
