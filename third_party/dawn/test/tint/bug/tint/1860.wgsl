@vertex
fn main() -> @builtin(position) vec4<f32> {
    return vec4(declared_after_usage.f);
}

struct DeclaredAfterUsage {
    f : f32,
}

@group(0) @binding(0) var <uniform> declared_after_usage : DeclaredAfterUsage;
