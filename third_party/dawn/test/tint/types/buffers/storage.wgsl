@group(0) @binding(0)
var<storage> weights: array<f32>;

@fragment
fn main() {
    var a = weights[0];
}
