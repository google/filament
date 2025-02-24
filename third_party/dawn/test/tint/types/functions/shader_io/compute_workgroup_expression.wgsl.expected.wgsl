@id(0) override x_dim = 2;

@compute @workgroup_size((1 + 2), x_dim, clamp(((1 - 2) + 4), 0, 5))
fn main() {
}
