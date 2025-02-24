#include "angle_trace_gl.h"
#include "CapturedTest_ActiveTextures_ES3_Vulkan.h"

const char *const glShaderSource_string_0[] = { 
"attribute vec4 a_position;\n"
"attribute vec2 a_texCoord;\n"
"varying vec2 v_texCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = a_position;\n"
"    v_texCoord = a_texCoord;\n"
"}",
};
const char *const glShaderSource_string_1[] = { 
"precision mediump float;\n"
"varying vec2 v_texCoord;\n"
"uniform sampler2D s_texture1;\n"
"uniform sampler2D s_texture2;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(s_texture1, v_texCoord) + texture2D(s_texture2, v_texCoord);\n"
"}",
};

// Private Functions

void SetupReplayContextShared(void)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, (GLuint *)gReadBuffer);
    UpdateTextureID(1, 0);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 9728);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 9728);
    glTexImage2D(GL_TEXTURE_2D, 0, 6408, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte *)&gBinaryData[288]);
    glGenTextures(1, (GLuint *)gReadBuffer);
    UpdateTextureID(2, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 9728);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 9728);
    glTexImage2D(GL_TEXTURE_2D, 0, 6408, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte *)&gBinaryData[352]);
    glGenTextures(1, (GLuint *)gReadBuffer);
    UpdateTextureID(3, 0);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[3]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 9728);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 9728);
    glTexImage2D(GL_TEXTURE_2D, 0, 6408, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte *)&gBinaryData[416]);
    CreateProgram(1);
    CreateShader(GL_VERTEX_SHADER, 4);
    glShaderSource(gShaderProgramMap[4], 1, glShaderSource_string_0, 0);
    glCompileShader(gShaderProgramMap[4]);
    glAttachShader(gShaderProgramMap[1], gShaderProgramMap[4]);
    CreateShader(GL_FRAGMENT_SHADER, 5);
    glShaderSource(gShaderProgramMap[5], 1, glShaderSource_string_1, 0);
    glCompileShader(gShaderProgramMap[5]);
    glAttachShader(gShaderProgramMap[1], gShaderProgramMap[5]);
    glBindAttribLocation(gShaderProgramMap[1], 0, "a_position");
    glBindAttribLocation(gShaderProgramMap[1], 1, "a_texCoord");
    glLinkProgram(gShaderProgramMap[1]);
    UpdateUniformLocation(1, "s_texture1", 0, 1);
    UpdateUniformLocation(1, "s_texture2", 1, 1);
    glUseProgram(gShaderProgramMap[1]);
    UpdateCurrentProgram(1);
    glUniform1iv(gUniformLocations[gCurrentProgram][0], 1, (const GLint *)&gBinaryData[480]);
    glUniform1iv(gUniformLocations[gCurrentProgram][1], 1, (const GLint *)&gBinaryData[496]);
    glDeleteShader(gShaderProgramMap[4]);
    glDeleteShader(gShaderProgramMap[5]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void SetupReplayContextSharedInactive(void)
{
}

// Public Functions

