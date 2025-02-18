// flags: --hlsl-shader-model 62
enable chromium_disable_uniformity_analysis, f16;

@fragment
fn main() -> @location(0) vec4<f32> {
    return vec4<f32>(0.1, 0.2, 0.3, 0.4);
}
