 struct Uniforms {
    u_scale : vec2<f32>,
    u_offset : vec2<f32>,
};
@binding(0) @group(0) var<uniform> uniforms : Uniforms;

struct VertexOutputs {
    @location(0) texcoords : vec2<f32>,
    @builtin(position) position : vec4<f32>,
};

@vertex fn vs_main(
    @builtin(vertex_index) VertexIndex : u32
) -> VertexOutputs {
    var texcoord = array<vec2<f32>, 3>(
        vec2<f32>(-0.5, 0.0),
        vec2<f32>( 1.5, 0.0),
        vec2<f32>( 0.5, 2.0));

    var output : VertexOutputs;
    output.position = vec4<f32>((texcoord[VertexIndex] * 2.0 - vec2<f32>(1.0, 1.0)), 0.0, 1.0);

    // Y component of scale is calculated by the copySizeHeight / textureHeight. Only
    // flipY case can get negative number.
    var flipY = uniforms.u_scale.y < 0.0;

    // Texture coordinate takes top-left as origin point. We need to map the
    // texture to triangle carefully.
    if (flipY) {
        // We need to get the mirror positions(mirrored based on y = 0.5) on flip cases.
        // Adopt transform to src texture and then mapping it to triangle coord which
        // do a +1 shift on Y dimension will help us got that mirror position perfectly.
        output.texcoords = (texcoord[VertexIndex] * uniforms.u_scale + uniforms.u_offset) *
            vec2<f32>(1.0, -1.0) + vec2<f32>(0.0, 1.0);
    } else {
        // For the normal case, we need to get the exact position.
        // So mapping texture to triangle firstly then adopt the transform.
        output.texcoords = (texcoord[VertexIndex] *
            vec2<f32>(1.0, -1.0) + vec2<f32>(0.0, 1.0)) *
            uniforms.u_scale + uniforms.u_offset;
    }

    return output;
}

@binding(1) @group(0) var mySampler: sampler;
@binding(2) @group(0) var myTexture: texture_2d<f32>;

@fragment fn fs_main(
    @location(0) texcoord : vec2<f32>
) -> @location(0) vec4<f32> {
    // Clamp the texcoord and discard the out-of-bound pixels.
    var clampedTexcoord =
        clamp(texcoord, vec2<f32>(0.0, 0.0), vec2<f32>(1.0, 1.0));
    if (!all(clampedTexcoord == texcoord)) {
        discard;
    }

    // Violates uniformity analysis:
    // var srcColor = textureSample(myTexture, mySampler, texcoord);
    var srcColor = vec4<f32>(0);
    // Swizzling of texture formats when sampling / rendering is handled by the
    // hardware so we don't need special logic in this shader. This is covered by tests.
    return srcColor;
}
