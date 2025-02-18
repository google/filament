fn a() {}
fn a__() {}

fn b() { a(); }
fn b__() { a__(); }

@compute @workgroup_size(1)
fn main() {
    b();
    b__();
}
