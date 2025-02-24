/*!
\brief The shaders for UIRenderer as c-style strings, glsl200es and glsl300es versions.
\file PVRUtils/OpenGLES/UIRendererShaders_ES.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

/// <summary>Source for the OpenGL ES 2 vertex shader for the UIRenderer</summary>
static const char _print3DShader_glsles200_vsh[] = "attribute highp vec4 myVertex;\n"
												   "attribute mediump vec2 myUV;\n"
												   "uniform mat4 myMVPMatrix;\n"
												   "uniform mat4 myUVMatrix;\n"
												   "varying mediump vec2 texCoord;\n"
												   "\n"
												   "void main()\n"
												   "{\n"
												   "\tgl_Position = myMVPMatrix * myVertex;\n"
												   "\ttexCoord = (myUVMatrix * vec4(myUV.st,1.0,1.0)).xy;\n"
												   "}\n";
static const int _print3DShader_glsles200_vsh_size = sizeof(_print3DShader_glsles200_vsh) / sizeof(_print3DShader_glsles200_vsh[0]);

/// <summary>Source for the OpenGL ES 2 fragment shader of the UIRenderer</summary>
static const char _print3DShader_glsles200_fsh[] = "uniform sampler2D fontTexture;\n"
												   "uniform highp vec4 varColor;\n"
												   "uniform bool alphaMode;\n"
												   "varying mediump vec2 texCoord;\n"
												   "void main()\n"
												   "{\n"
												   "\tmediump vec4 vTex = texture2D(fontTexture, texCoord);\n"
												   "\tif (alphaMode)\n"
												   "\t{\n"
												   "\t\tgl_FragColor = vec4(varColor.rgb, varColor.a * vTex.a);\n"
												   "\t}\n"
												   "\telse\n"
												   "\t{\n"
												   "\t\tgl_FragColor = vec4(varColor * vTex);\n"
												   "\t}\n"

												   "\t#ifndef FRAMEBUFFER_SRGB\n"
												   "\t\tgl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(0.4545454545));// Gamma correction   (0.4545454545 = 1.0/ 2.2)\n"
												   "\t#endif\n"
												   "}\n";
static const int _print3DShader_glsles200_fsh_size = sizeof(_print3DShader_glsles200_fsh) / sizeof(_print3DShader_glsles200_fsh[0]);
