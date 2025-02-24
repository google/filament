var<workgroup> zero : array<i32, 3>;

@compute @workgroup_size(1)
fn main() {
    var v = zero;
}
