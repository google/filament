#include "CapturedTest_ActiveTextures_ES3_Vulkan.h"
#include "angle_trace_gl.h"

// Private Functions

void SetupReplayContext3(void)
{
    eglMakeCurrent(gEGLDisplay, gSurfaceMap2[0], gSurfaceMap2[0], gContextMap2[3]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[2]);
    glUseProgram(gShaderProgramMap[1]);
    UpdateCurrentProgram(1);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gTransformFeedbackMap[0]);
    glViewport(0, 0, 128, 128);
    glScissor(0, 0, 128, 128);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void ReplayFrame1(void)
{
    eglGetError();
    glGetError();
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *)gReadBuffer);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint *)gReadBuffer);
    glGetAttribLocation(gShaderProgramMap[1], "a_position");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, gClientArrays[0]);
    glEnableVertexAttribArray(0);
    UpdateClientArrayPointer(0, (const GLubyte *)&gBinaryData[0], 72);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glGetError();
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)gReadBuffer);
    glGetError();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[2]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[3]);
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *)gReadBuffer);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint *)gReadBuffer);
    glGetAttribLocation(gShaderProgramMap[1], "a_position");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, gClientArrays[0]);
    glEnableVertexAttribArray(0);
    UpdateClientArrayPointer(0, (const GLubyte *)&gBinaryData[80], 72);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glGetError();
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)gReadBuffer);
    glGetError();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, 6408, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte *)&gBinaryData[160]);
}

void ReplayFrame2(void)
{
    eglGetError();
}

void ReplayFrame3(void)
{
    eglGetError();
}

void ResetReplayContextShared(void)
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 9728);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 9728);
    glTexImage2D(GL_TEXTURE_2D, 0, 6408, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const GLubyte *)&gBinaryData[224]);
}

void ResetReplayContext3(void)
{
}

void ReplayFrame4(void)
{
    eglGetError();
}

// Public Functions

void SetupReplay(void)
{
    InitReplay();
    SetupReplayContextShared();
    if (gReplayResourceMode == angle::ReplayResourceMode::All)
    {
        SetupReplayContextSharedInactive();
    }
    SetCurrentContextID(3);
    SetupReplayContext3();

}

void ResetReplay(void)
{
    ResetReplayContextShared();
    ResetReplayContext3();

    // Reset main context state
    glBindTexture(GL_TEXTURE_2D, gTextureMap[1]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureMap[2]);
}

