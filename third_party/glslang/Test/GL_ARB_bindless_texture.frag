#version 460 compatibility

#extension GL_ARB_bindless_texture: require

#if !defined GL_ARB_bindless_texture
#  error GL_ARB_bindless_texture is not defined
#elif GL_ARB_bindless_texture != 1
#  error GL_ARB_bindless_texture is not equal to 1
#endif

// Valid usage cases
layout(bindless_sampler) uniform sampler2D s0;      //  case0:  bindless layout
in sampler2D s1;                                    //  case1:  sampler as an input
uniform uvec2 s2;                                   //  case2:  uvec2 as sampler constructor
uniform ivec2 s3;                                   //  case3:  ivec2 as sampler constructor
uniform int index;
in sampler2D s4[2][3];                              //  case4:  sampler arrays of arrays
uniform BB {sampler2D s5;} bbs5[2];                 //  case5:  uniform block member as a sampler
in samplerBuffer s6;                                //  case6:  samplerBuffer input
uniform UBO9 {samplerBuffer s7;};                   //  case7:  samplerBuffer as an uniform block member
buffer SSBO10 {samplerBuffer s8;};                  //  case8:  samplerBuffer as an ssbo member
layout(rgba8, bindless_image) in image2D i9;        //  case9:  bindless image as an input


uniform vec2 coord;                                 //  bindless coord 2-D
uniform int icoord;                                 //  bindless coord 1-D
out vec4 color0;
out vec4 color1;
out vec4 color2;
out vec4 color3;
out vec4 color4;
out vec4 color5;
out vec4 color6;
out vec4 color7;
out vec4 color8;
out vec4 color9;

void main()
{
    color0 = texture(s0, coord);
    color1 = texture(s1, coord);
    color2 = texture(sampler2D(s2), coord);
    color3 = texture(sampler2D(s3), coord);
    color4 = texture(s4[index][index], coord);
    color5 = texture(bbs5[index].s5, coord);
    color6 = texelFetch(s6, icoord);
    color7 = texelFetch(s7, icoord);
    color8 = texelFetch(s8, icoord);
    color9 = imageLoad(i9, ivec2(0,0));
}