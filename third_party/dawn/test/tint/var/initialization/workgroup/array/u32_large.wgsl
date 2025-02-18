var<workgroup> zero : array<i32, 23>;

@compute @workgroup_size(13)
fn main() {
    var v = zero;
}
