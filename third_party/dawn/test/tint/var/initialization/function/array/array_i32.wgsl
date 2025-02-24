@compute @workgroup_size(1)
fn main() {
    var zero : array<array<i32, 3>, 2>;
    var init : array<array<i32, 3>, 2> = array<array<i32, 3>, 2>(array<i32, 3>(1, 2, 3), array<i32, 3>(4, 5, 6));
}
