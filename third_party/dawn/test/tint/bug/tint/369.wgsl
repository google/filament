 struct S {
    m : mat2x2<f32>,
};

@group(0) @binding(0) var<storage> SSBO : S;
@group(0) @binding(0) var<uniform> UBO : S;
