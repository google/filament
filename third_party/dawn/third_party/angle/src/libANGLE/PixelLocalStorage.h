//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PixelLocalStorage.h: Defines the renderer-agnostic container classes
// gl::PixelLocalStorage and gl::PixelLocalStoragePlane for
// ANGLE_shader_pixel_local_storage.

#ifndef LIBANGLE_PIXEL_LOCAL_STORAGE_H_
#define LIBANGLE_PIXEL_LOCAL_STORAGE_H_

#include "GLSLANG/ShaderLang.h"
#include "libANGLE/Caps.h"
#include "libANGLE/ImageIndex.h"
#include "libANGLE/angletypes.h"

namespace gl
{

class Context;
class Texture;

// Holds the configuration of an ANGLE_shader_pixel_local_storage plane.
//
// Unlike normal framebuffer attachments, pixel local storage planes don't take effect until the
// application calls glBeginPixelLocalStorageANGLE, and the manner in which they take effect is
// highly dependent on the backend implementation. A PixelLocalStoragePlane is just a plain data
// description what to set up later once PLS is enabled.
class PixelLocalStoragePlane : angle::NonCopyable, public angle::ObserverInterface
{
  public:
    PixelLocalStoragePlane();
    ~PixelLocalStoragePlane() override;

    // Called when the context is lost or destroyed. Causes this class to clear its GL object
    // handles.
    void onContextObjectsLost();

    void deinitialize(Context *);
    void setMemoryless(Context *, GLenum internalformat);
    void setTextureBacked(Context *, Texture *, int level, int layer);
    void onSubjectStateChange(angle::SubjectIndex, angle::SubjectMessage) override;

    // Returns true if the plane is deinitialized, either explicitly or implicitly via deleting the
    // texture that was attached to it.
    bool isDeinitialized() const;

    GLenum getInternalformat() const { return mInternalformat; }
    bool isMemoryless() const { return mMemoryless; }
    TextureID getTextureID() const { return mTextureID; }

    // Implements glGetIntegeri_v() for GL_PIXEL_LOCAL_FORMAT_ANGLE,
    // GL_PIXEL_LOCAL_TEXTURE_NAME_ANGLE, GL_PIXEL_LOCAL_TEXTURE_LEVEL_ANGLE, and
    // GL_PIXEL_LOCAL_TEXTURE_LAYER_ANGLE
    GLint getIntegeri(GLenum target) const;

    // If this plane is texture backed, stores the bound texture image's {width, height, 0} to
    // Extents and returns true. Otherwise returns false, meaning the plane is either deinitialized
    // or memoryless.
    bool getTextureImageExtents(const Context *, Extents *extents) const;

    // Ensures we have an internal backing texture for memoryless planes. In some implementations we
    // need a backing texture even if the plane is memoryless.
    void ensureBackingTextureIfMemoryless(Context *, Extents plsSize);

    // Attaches this plane to the specified color attachment point on the current draw framebuffer.
    void attachToDrawFramebuffer(Context *, GLenum colorAttachment) const;

    // Interface for clearing typed pixel local storage planes.
    class ClearCommands
    {
      public:
        virtual ~ClearCommands() {}
        virtual void clearfv(int target, const GLfloat[]) const = 0;
        virtual void cleariv(int target, const GLint[]) const   = 0;
        virtual void clearuiv(int target, const GLuint[]) const = 0;
    };

    // Issues the approprite command from ClearCommands for this plane's internalformat. Uses the
    // clear state value that corresponds to mInternalFormat, and potentially clamps it to ensure it
    // is representable.
    void issueClearCommand(ClearCommands *, int target, GLenum loadop) const;

    // Binds this PLS plane to a texture image unit for image load/store shader operations.
    void bindToImage(Context *, GLuint unit, bool needsR32Packing) const;

    // Low-level access to the backing texture. The plane must not be memoryless or deinitialized.
    const ImageIndex &getTextureImageIndex() const { return mTextureImageIndex; }
    const Texture *getBackingTexture(const Context *context) const;

    void setClearValuef(const GLfloat value[4]) { memcpy(mClearValuef.data(), value, 4 * 4); }
    void setClearValuei(const GLint value[4]) { memcpy(mClearValuei.data(), value, 4 * 4); }
    void setClearValueui(const GLuint value[4]) { memcpy(mClearValueui.data(), value, 4 * 4); }

    void getClearValuef(GLfloat value[4]) const { memcpy(value, mClearValuef.data(), 4 * 4); }
    void getClearValuei(GLint value[4]) const { memcpy(value, mClearValuei.data(), 4 * 4); }
    void getClearValueui(GLuint value[4]) const { memcpy(value, mClearValueui.data(), 4 * 4); }

    // True if PLS is currently active and this plane is enabled.
    bool isActive() const { return mActive; }
    void markActive(bool active) { mActive = active; }

  private:
    GLenum mInternalformat = GL_NONE;  // GL_NONE if this plane is in a deinitialized state.
    bool mMemoryless       = false;
    TextureID mTextureID   = TextureID();
    ImageIndex mTextureImageIndex;

    // Clear value state.
    std::array<GLfloat, 4> mClearValuef{};
    std::array<GLint, 4> mClearValuei{};
    std::array<GLuint, 4> mClearValueui{};

    // True if PLS is currently active and this plane is enabled.
    bool mActive = false;

    angle::ObserverBinding mTextureObserver;
};

// Manages a collection of PixelLocalStoragePlanes and applies them to ANGLE's GL state.
//
// The main magic of ANGLE_shader_pixel_local_storage happens inside shaders, so we just emulate the
// client API on top of ANGLE's OpenGL ES API for simplicity.
class PixelLocalStorage
{
  public:
    static std::unique_ptr<PixelLocalStorage> Make(const Context *);

    virtual ~PixelLocalStorage();

    // Called when the owning framebuffer is being destroyed.
    void onFramebufferDestroyed(const Context *);

    // Deletes any GL objects that have been allocated for pixel local storage. These can't be
    // cleaned up in the destructor because they require a non-const Context object.
    void deleteContextObjects(Context *);

    const PixelLocalStoragePlane &getPlane(GLint plane) const
    {
        ASSERT(0 <= plane && plane < IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES);
        return mPlanes[plane];
    }

    const PixelLocalStoragePlane *getPlanes() { return mPlanes.data(); }

    size_t interruptCount() const { return mInterruptCount; }

    // ANGLE_shader_pixel_local_storage API.
    void deinitialize(Context *context, GLint plane) { mPlanes[plane].deinitialize(context); }
    void setMemoryless(Context *context, GLint plane, GLenum internalformat)
    {
        mPlanes[plane].setMemoryless(context, internalformat);
    }
    void setTextureBacked(Context *context, GLint plane, Texture *tex, int level, int layer)
    {
        mPlanes[plane].setTextureBacked(context, tex, level, layer);
    }
    void setClearValuef(GLint plane, const GLfloat val[4]) { mPlanes[plane].setClearValuef(val); }
    void setClearValuei(GLint plane, const GLint val[4]) { mPlanes[plane].setClearValuei(val); }
    void setClearValueui(GLint plane, const GLuint val[4]) { mPlanes[plane].setClearValueui(val); }
    void begin(Context *, GLsizei n, const GLenum loadops[]);
    void end(Context *, GLsizei n, const GLenum storeops[]);
    void barrier(Context *);
    void interrupt(Context *);
    void restore(Context *);

    // While pixel local storage is active, the draw buffers on and after
    // 'FirstOverriddenDrawBuffer' are blocked from the client and reserved for internal use by PLS.
    static GLint FirstOverriddenDrawBuffer(const Caps &caps, GLuint numActivePlanes)
    {
        ASSERT(numActivePlanes > 0);
        return std::min(caps.maxColorAttachmentsWithActivePixelLocalStorage,
                        caps.maxCombinedDrawBuffersAndPixelLocalStoragePlanes - numActivePlanes);
    }

  protected:
    PixelLocalStorage(const ShPixelLocalStorageOptions &, const Caps &);

    // Called when the context is lost or destroyed. Causes the subclass to clear its GL object
    // handles.
    virtual void onContextObjectsLost() = 0;

    // Called when the framebuffer is being destroyed. Causes the subclass to delete its frontend GL
    // object handles.
    virtual void onDeleteContextObjects(Context *) = 0;

    // ANGLE_shader_pixel_local_storage API.
    virtual void onBegin(Context *, GLsizei n, const GLenum loadops[], Extents plsSize) = 0;
    virtual void onEnd(Context *, GLsizei n, const GLenum storeops[])                   = 0;
    virtual void onBarrier(Context *)                                                   = 0;

    const ShPixelLocalStorageOptions mPLSOptions;

  private:
    angle::FixedVector<PixelLocalStoragePlane, IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES>
        mPlanes;
    size_t mInterruptCount           = 0;
    GLsizei mActivePlanesAtInterrupt = 0;
};

}  // namespace gl

#endif  // LIBANGLE_PIXEL_LOCAL_STORAGE_H_
