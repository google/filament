 struct Uniforms {
    dstTextureFlipY : u32,
    channelCount    : u32,
    srcCopyOrigin   : vec2<u32>,
    dstCopyOrigin   : vec2<u32>,
    copySize        : vec2<u32>,
};
 struct OutputBuf {
    result : array<u32>,
};
@group(0) @binding(0) var src : texture_2d<f32>;
@group(0) @binding(1) var dst : texture_2d<f32>;
@group(0) @binding(2) var<storage, read_write> output : OutputBuf;
@group(0) @binding(3) var<uniform> uniforms : Uniforms;
fn aboutEqual(value : f32, expect : f32) -> bool {
    // The value diff should be smaller than the hard coded tolerance.
    return abs(value - expect) < 0.001;
}
@compute @workgroup_size(1, 1, 1)
fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3<u32>) {
    let srcSize = textureDimensions(src);
    let dstSize = textureDimensions(dst);
    let dstTexCoord : vec2<u32> = vec2<u32>(GlobalInvocationID.xy);
    let nonCoveredColor : vec4<f32> =
        vec4<f32>(0.0, 1.0, 0.0, 1.0); // should be green

    var success : bool = true;
    if (dstTexCoord.x < uniforms.dstCopyOrigin.x ||
        dstTexCoord.y < uniforms.dstCopyOrigin.y ||
        dstTexCoord.x >= uniforms.dstCopyOrigin.x + uniforms.copySize.x ||
        dstTexCoord.y >= uniforms.dstCopyOrigin.y + uniforms.copySize.y) {
        success = success &&
                    all(textureLoad(dst, vec2<i32>(dstTexCoord), 0) == nonCoveredColor);
    } else {
        // Calculate source texture coord.
        var srcTexCoord : vec2<u32> = dstTexCoord - uniforms.dstCopyOrigin +
                                        uniforms.srcCopyOrigin;
        // Note that |flipY| equals flip src texture firstly and then do copy from src
        // subrect to dst subrect. This helps on blink part to handle some input texture
        // which is flipped and need to unpack flip during the copy.
        // We need to calculate the expect y coord based on this rule.
        if (uniforms.dstTextureFlipY == 1u) {
            srcTexCoord.y = srcSize.y - srcTexCoord.y - 1u;
        }

        let srcColor : vec4<f32> = textureLoad(src, vec2<i32>(srcTexCoord), 0);
        let dstColor : vec4<f32> = textureLoad(dst, vec2<i32>(dstTexCoord), 0);

        // Not use loop and variable index format to workaround
        // crbug.com/tint/638.
        if (uniforms.channelCount == 2u) { // All have rg components.
            success = success &&
                        aboutEqual(dstColor.r, srcColor.r) &&
                        aboutEqual(dstColor.g, srcColor.g);
        } else {
            success = success &&
                        aboutEqual(dstColor.r, srcColor.r) &&
                        aboutEqual(dstColor.g, srcColor.g) &&
                        aboutEqual(dstColor.b, srcColor.b) &&
                        aboutEqual(dstColor.a, srcColor.a);
        }
    }
    let outputIndex : u32 = GlobalInvocationID.y * dstSize.x +
                            GlobalInvocationID.x;
    if (success) {
        output.result[outputIndex] = 1u;
    } else {
        output.result[outputIndex] = 0u;
    }
}
