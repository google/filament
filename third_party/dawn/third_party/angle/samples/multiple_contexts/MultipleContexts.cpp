//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Demonstrates GLES usage with two contexts, one uploading texture data.

#include "SampleApplication.h"
#include "util/random_utils.h"
#include "util/shader_utils.h"
#include "util/test_utils.h"

#include <array>
#include <mutex>
#include <queue>
#include <thread>

struct TextureAndFence
{
    GLuint textureID;
    GLsync fenceSync;
};

constexpr GLuint64 kTimeout         = 10000000;
static constexpr GLint kTextureSize = 2;
constexpr size_t kWindowWidth       = 400;
constexpr size_t kWindowHeight      = 300;

void UpdateThreadLoop(EGLDisplay display,
                      EGLConfig config,
                      EGLContext shareContext,
                      std::mutex *updateThreadMutex,
                      std::queue<TextureAndFence> *updateThreadQueue,
                      std::mutex *mainThreadMutex,
                      std::queue<TextureAndFence> *mainThreadQueue)
{
    angle::RNG rng;

    EGLint contextAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE};
    EGLContext context      = eglCreateContext(display, config, shareContext, contextAttribs);

    EGLint surfaceAttribs[] = {EGL_NONE};
    EGLSurface surface      = eglCreatePbufferSurface(display, config, surfaceAttribs);

    eglMakeCurrent(display, surface, surface, context);

    for (;;)
    {
        bool hasUpdate = false;
        TextureAndFence textureAndFence;

        {
            std::lock_guard<std::mutex> lock(*updateThreadMutex);
            if (!updateThreadQueue->empty())
            {
                textureAndFence = updateThreadQueue->back();
                hasUpdate       = true;
                updateThreadQueue->pop();
            }
        }

        if (hasUpdate)
        {
            if (textureAndFence.textureID == 0)
            {
                // Signal from the main thread to stop execution.
                break;
            }

            glClientWaitSync(textureAndFence.fenceSync, 0, kTimeout);
            glDeleteSync(textureAndFence.fenceSync);
            glBindTexture(GL_TEXTURE_2D, textureAndFence.textureID);

            std::vector<uint8_t> bytes(3, 0);
            FillVectorWithRandomUBytes(&rng, &bytes);
            bytes.push_back(255);

            std::vector<uint8_t> textureData;
            for (GLint x = 0; x < kTextureSize; ++x)
            {
                for (GLint y = 0; y < kTextureSize; ++y)
                {
                    textureData.insert(textureData.end(), bytes.begin(), bytes.end());
                }
            }

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTextureSize, kTextureSize, GL_RGBA,
                            GL_UNSIGNED_BYTE, textureData.data());

            GLsync mainThreadSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

            {
                std::lock_guard<std::mutex> lock(*mainThreadMutex);
                mainThreadQueue->push({textureAndFence.textureID, mainThreadSync});
            }
        }

        angle::Sleep(200);
    }

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(display, surface);
    eglDestroyContext(display, context);
}

class MultipleContextsSample : public SampleApplication
{
  public:
    MultipleContextsSample(int argc, char **argv)
        : SampleApplication("MultipleContexts",
                            argc,
                            argv,
                            ClientType::ES3_0,
                            kWindowWidth,
                            kWindowHeight)
    {}

    bool initialize() override
    {
        // Initialize some textures and send them to the update thread.
        glGenTextures(kNumTextures, mTextures.data());

        for (GLuint texture : mTextures)
        {
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);
            GLsync fenceSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

            mUpdateThreadQueue.push({texture, fenceSync});
        }

        mUpdateThread.reset(new std::thread(UpdateThreadLoop, getDisplay(), getConfig(),
                                            getContext(), &mUpdateThreadMutex, &mUpdateThreadQueue,
                                            &mMainThreadMutex, &mMainThreadQueue));

        mProgram = CompileProgram(angle::essl1_shaders::vs::Texture2D(),
                                  angle::essl1_shaders::fs::Texture2D());
        glUseProgram(mProgram);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 6, nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        return true;
    }

    void destroy() override
    {
        {
            // Signal the worker thread to stop execution.
            std::lock_guard<std::mutex> lock(mUpdateThreadMutex);
            mUpdateThreadQueue.push({0, 0});
        }

        for (;;)
        {
            {
                std::lock_guard<std::mutex> mainLock(mMainThreadMutex);
                if (mMainThreadQueue.empty())
                {
                    std::lock_guard<std::mutex> updateLock(mUpdateThreadMutex);
                    if (mUpdateThreadQueue.empty())
                    {
                        break;
                    }
                }
                else
                {
                    TextureAndFence textureAndFence = mMainThreadQueue.back();
                    mMainThreadQueue.pop();
                    glClientWaitSync(textureAndFence.fenceSync, 0, kTimeout);
                    glDeleteSync(textureAndFence.fenceSync);
                }
            }
        }

        glDeleteTextures(kNumTextures, mTextures.data());
        glDeleteProgram(mProgram);
        glDeleteBuffers(1, &mVertexBuffer);
    }

    void draw() override
    {
        bool hasUpdate = false;
        TextureAndFence textureAndFence;
        while (!hasUpdate)
        {
            {
                std::lock_guard<std::mutex> lock(mMainThreadMutex);
                if (!mMainThreadQueue.empty())
                {
                    hasUpdate       = true;
                    textureAndFence = mMainThreadQueue.back();
                    mMainThreadQueue.pop();
                }
            }
        }

        glClientWaitSync(textureAndFence.fenceSync, 0, kTimeout);
        glDeleteSync(textureAndFence.fenceSync);
        glBindTexture(GL_TEXTURE_2D, textureAndFence.textureID);

        constexpr size_t kNumRows    = 3;
        constexpr size_t kNumCols    = 4;
        constexpr size_t kTileHeight = kWindowHeight / kNumRows;
        constexpr size_t kTileWidth  = kWindowWidth / kNumCols;

        size_t tileX = mDrawCount % kNumCols;
        size_t tileY = (mDrawCount / kNumCols) % kNumRows;

        mDrawCount++;

        GLfloat tileX0 = static_cast<float>(tileX) / static_cast<float>(kNumCols);
        GLfloat tileY0 = static_cast<float>(tileY) / static_cast<float>(kNumRows);
        GLfloat tileX1 = tileX0 + static_cast<float>(kTileWidth) / static_cast<float>(kWindowWidth);
        GLfloat tileY1 =
            tileY0 + static_cast<float>(kTileHeight) / static_cast<float>(kWindowHeight);

        tileX0 = tileX0 * 2.0f - 1.0f;
        tileX1 = tileX1 * 2.0f - 1.0f;
        tileY0 = tileY0 * 2.0f - 1.0f;
        tileY1 = tileY1 * 2.0f - 1.0f;

        std::vector<GLfloat> vertices = {tileX0, tileY0, tileX0, tileY1, tileX1, tileY0,
                                         tileX1, tileY0, tileX1, tileY1, tileX0, tileY1};

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices[0]) * vertices.size(), vertices.data());

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        GLsync drawSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        {
            std::lock_guard<std::mutex> lock(mUpdateThreadMutex);
            mUpdateThreadQueue.push({textureAndFence.textureID, drawSync});
        }
    }

  private:
    std::unique_ptr<std::thread> mUpdateThread;

    static constexpr GLuint kNumTextures = 4;
    std::array<GLuint, kNumTextures> mTextures;

    GLuint mProgram      = 0;
    GLuint mVertexBuffer = 0;

    std::mutex mUpdateThreadMutex;
    std::queue<TextureAndFence> mUpdateThreadQueue;
    std::mutex mMainThreadMutex;
    std::queue<TextureAndFence> mMainThreadQueue;
    size_t mDrawCount = 0;
};

int main(int argc, char **argv)
{
    MultipleContextsSample app(argc, argv);
    return app.run();
}
