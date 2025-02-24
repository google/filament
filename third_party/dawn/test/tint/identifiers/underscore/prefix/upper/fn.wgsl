fn A() {}
fn _A() {}

fn B() { A(); }
fn _B() { _A(); }

@compute @workgroup_size(1)
fn main() {
    B();
    _B();
}
