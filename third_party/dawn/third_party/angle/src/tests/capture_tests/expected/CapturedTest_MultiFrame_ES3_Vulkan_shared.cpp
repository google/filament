#include "angle_trace_gl.h"
#include "CapturedTest_MultiFrame_ES3_Vulkan.h"

const char *const glShaderSource_string_4[] = { 
"precision mediump float;\n"
"varying vec2 v_texCoord;\n"
"uniform sampler2D s_texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(0.4, 0.4, 0.4, 1.0);\n"
"    gl_FragColor = texture2D(s_texture, v_texCoord);\n"
"}",
};
const char *const glShaderSource_string_5[] = { 
"precision highp float;\n"
"void main(void) {\n"
"   gl_Position = vec4(0.5, 0.5, 0.5, 1.0);\n"
"}",
};

// Private Functions

void SetupReplayContextShared(void)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, (GLuint *)gReadBuffer);
    UpdateTextureID(2, 0);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 9728);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 9728);
    glTexImage2D(GL_TEXTURE_2D, 0, 6407, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, (const GLubyte *)&gBinaryData[624]);
    CreateProgram(5);
    CreateProgram(6);
    CreateProgram(8);
    CreateShader(GL_VERTEX_SHADER, 11);
    glShaderSource(gShaderProgramMap[11], 1, glShaderSource_string_0, 0);
    glCompileShader(gShaderProgramMap[11]);
    glAttachShader(gShaderProgramMap[8], gShaderProgramMap[11]);
    CreateShader(GL_FRAGMENT_SHADER, 12);
    glShaderSource(gShaderProgramMap[12], 1, glShaderSource_string_1, 0);
    glCompileShader(gShaderProgramMap[12]);
    glAttachShader(gShaderProgramMap[8], gShaderProgramMap[12]);
    glBindAttribLocation(gShaderProgramMap[8], 0, "a_position");
    glBindAttribLocation(gShaderProgramMap[8], 1, "a_texCoord");
    glLinkProgram(gShaderProgramMap[8]);
    UpdateUniformLocation(8, "s_texture", 0, 1);
    glUseProgram(gShaderProgramMap[8]);
    UpdateCurrentProgram(8);
    glUniform1iv(gUniformLocations[gCurrentProgram][0], 1, (const GLint *)&gBinaryData[640]);
    glDeleteShader(gShaderProgramMap[11]);
    glDeleteShader(gShaderProgramMap[12]);
    CreateShader(GL_FRAGMENT_SHADER, 2);
    glAttachShader(gShaderProgramMap[5], gShaderProgramMap[2]);
    glShaderSource(gShaderProgramMap[2], 1, glShaderSource_string_4, 0);
    glCompileShader(gShaderProgramMap[2]);
    CreateShader(GL_VERTEX_SHADER, 4);
    glAttachShader(gShaderProgramMap[5], gShaderProgramMap[4]);
    glShaderSource(gShaderProgramMap[4], 1, glShaderSource_string_0, 0);
    CreateShader(GL_VERTEX_SHADER, 7);
    glAttachShader(gShaderProgramMap[6], gShaderProgramMap[7]);
    glShaderSource(gShaderProgramMap[7], 1, glShaderSource_string_5, 0);
    glCompileShader(gShaderProgramMap[7]);
}

void SetupReplayContextSharedInactive(void)
{
    CreateProgram(3);
    CreateShader(GL_VERTEX_SHADER, 11);
    glShaderSource(gShaderProgramMap[11], 1, glShaderSource_string_0, 0);
    glCompileShader(gShaderProgramMap[11]);
    glAttachShader(gShaderProgramMap[3], gShaderProgramMap[11]);
    CreateShader(GL_FRAGMENT_SHADER, 12);
    glShaderSource(gShaderProgramMap[12], 1, glShaderSource_string_4, 0);
    glCompileShader(gShaderProgramMap[12]);
    glAttachShader(gShaderProgramMap[3], gShaderProgramMap[12]);
    glBindAttribLocation(gShaderProgramMap[3], 0, "a_position");
    glBindAttribLocation(gShaderProgramMap[3], 1, "a_texCoord");
    glLinkProgram(gShaderProgramMap[3]);
    UpdateUniformLocation(3, "s_texture", 0, 1);
    glUseProgram(gShaderProgramMap[3]);
    UpdateCurrentProgram(3);
    glUniform1iv(gUniformLocations[gCurrentProgram][0], 1, (const GLint *)&gBinaryData[656]);
    glDeleteShader(gShaderProgramMap[11]);
    glDeleteShader(gShaderProgramMap[12]);
    CreateShader(GL_VERTEX_SHADER, 1);
    glShaderSource(gShaderProgramMap[1], 1, glShaderSource_string_0, 0);
    glCompileShader(gShaderProgramMap[1]);
    CreateShader(GL_VERTEX_SHADER, 9);
    glShaderSource(gShaderProgramMap[9], 1, glShaderSource_string_0, 0);
    glCompileShader(gShaderProgramMap[9]);
    CreateShader(GL_FRAGMENT_SHADER, 10);
    glShaderSource(gShaderProgramMap[10], 1, glShaderSource_string_1, 0);
    glCompileShader(gShaderProgramMap[10]);
}

// Public Functions

