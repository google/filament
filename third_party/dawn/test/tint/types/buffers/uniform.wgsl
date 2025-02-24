@group(0) @binding(0)
var<uniform> weights: vec2f;

@fragment
fn main() {
    var a = weights[0];
}
