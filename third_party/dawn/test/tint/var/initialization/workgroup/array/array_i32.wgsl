var<workgroup> zero : array<array<i32, 3>, 2>;

@compute @workgroup_size(1)
fn main() {
    var v = zero;
}
