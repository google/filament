#version 450
#if defined(GL_ARB_post_depth_coverge)
#extension GL_ARB_post_depth_coverage : require
#else
#extension GL_EXT_post_depth_coverage : require
#endif
layout(early_fragment_tests, post_depth_coverage) in;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(float(gl_SampleMaskIn[0]));
}

