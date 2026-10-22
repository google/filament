#version 310 es
#extension GL_QCOM_tile_shading : enable 

precision highp int;
precision highp float;

layout (non_coherent_attachment_readQCOM) in;

layout (set=0, binding=0, tile_attachmentQCOM, rgba32f) uniform readonly highp image2D input0;
layout (set=0, binding=1, tile_attachmentQCOM, rgba32f) uniform readonly highp image2D  color0;
layout (set=0, binding=2, tile_attachmentQCOM, rgba32i) uniform writeonly highp iimage2D color1;


// depth/stencil attachments
layout (set=0, binding=3, tile_attachmentQCOM, rgba32f) uniform readonly highp image2D  depth;
layout (set=0, binding=4, tile_attachmentQCOM, rgba32ui) uniform writeonly highp uimage2D stencil;

layout (location=0) out vec4 fragColor; 

void main() 
{ 
	// integer coordinates of the center of tile.
    uvec2 o2     = uvec2(8, 4);
          o2     = o2 / uvec2(2);
    ivec2 offset = ivec2(gl_TileOffsetQCOM + (gl_TileDimensionQCOM/uvec3(2)).xy);

  // read from attachments 
  vec4  colorA   = imageLoad( color0, offset );
  vec4  colorB   = vec4(colorA);
  vec4  colorC   = imageLoad( input0, offset );
  float d        = imageLoad( depth, offset ).x;
  uint  s        = uint(d);
	  
    // compute output value
    ivec4 outColor  = ivec4( colorB + colorB + colorC + vec4(d) + vec4(s) );
  
    // write to attachments
    imageStore( color1,  offset, outColor );
    imageStore( stencil, offset, uvec4(outColor) );
  
    // write to color attachment 0 via fragment output 
    fragColor = vec4(outColor) + vec4(1.0, 0.0, 0.0, 1.0); 
}
