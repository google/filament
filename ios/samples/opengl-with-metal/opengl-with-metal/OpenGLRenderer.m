/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "OpenGLRenderer.h"

#import <QuartzCore/QuartzCore.h>

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

@interface OpenGLRenderer ()

- (void)setupContext;
- (void)destroyFramebuffer;

@end

#define GL_UNSIGNED_INT_8_8_8_8_REV 0x8367

@implementation OpenGLRenderer {
    EAGLContext* _context;
    GLuint _frameBuffer;
    CVOpenGLESTextureCacheRef _textureCache;
    CVOpenGLESTextureRef _texture;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        [self setupContext];
        _frameBuffer = 0;
        _texture = nil;
    }
    return self;
}

- (void)setPixelBuffer:(CVPixelBufferRef)pixelBuffer {
    [EAGLContext setCurrentContext:_context];

    [self destroyFramebuffer];

    if (_texture) {
        CFRelease(_texture);
        _texture = nil;
    }

    if (!pixelBuffer) {
        return;
    }

    const size_t width = CVPixelBufferGetWidth(pixelBuffer);
    const size_t height = CVPixelBufferGetHeight(pixelBuffer);
    CVReturn cvret = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
            _textureCache, pixelBuffer, nil, GL_TEXTURE_2D, GL_RGBA, (GLsizei)width,
            (GLsizei)height, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, 0, &_texture);
    assert(cvret == kCVReturnSuccess);

    glGenFramebuffers(1, &_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            CVOpenGLESTextureGetName(_texture), 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

- (void)setupContext {
    EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
    _context = [[EAGLContext alloc] initWithAPI:api];
    assert(_context);

    BOOL result = [EAGLContext setCurrentContext:_context];
    assert(result);

    CVReturn cvret =
            CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, nil, _context, nil, &_textureCache);
    assert(cvret == kCVReturnSuccess);
}

- (void)destroyFramebuffer {
    if (_frameBuffer == 0) {
        return;
    }

    glDeleteFramebuffers(1, &_frameBuffer);
    _frameBuffer = 0;
}

- (void)renderWithColor:(Color3)color {
    [EAGLContext setCurrentContext:_context];

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);

    glClearColor(color.r, color.g, color.b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // From Apple's "OpenGLMetalInteroperability" sample:
    // When rendering to a CVPixelBuffer with OpenGL, call glFlush to ensure OpenGL commands are
    // excuted on the pixel buffer before Metal reads the buffer.
    glFlush();
}

- (void)dealloc {
    [self setPixelBuffer:nil];
    CFRelease(_textureCache);
}

@end
