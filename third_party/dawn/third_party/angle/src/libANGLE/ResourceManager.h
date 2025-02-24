//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ResourceManager.h : Defines the ResourceManager classes, which handle allocation and lifetime of
// GL objects.

#ifndef LIBANGLE_RESOURCEMANAGER_H_
#define LIBANGLE_RESOURCEMANAGER_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/HandleAllocator.h"
#include "libANGLE/ResourceMap.h"

namespace rx
{
class GLImplFactory;
}  // namespace rx

namespace egl
{
class ShareGroup;
}  // namespace egl

namespace gl
{
class Buffer;
struct Caps;
class Context;
class Framebuffer;
struct Limitations;
class MemoryObject;
class Path;
class Program;
class ProgramPipeline;
class Renderbuffer;
class Sampler;
class Semaphore;
class Shader;
class Sync;
class Texture;

class ResourceManagerBase : angle::NonCopyable
{
  public:
    ResourceManagerBase();

    void addRef();
    void release(const Context *context);

  protected:
    virtual void reset(const Context *context) = 0;
    virtual ~ResourceManagerBase();

    HandleAllocator mHandleAllocator;

  private:
    size_t mRefCount;
};

template <typename ResourceType, typename ImplT, typename IDType>
class TypedResourceManager : public ResourceManagerBase
{
  public:
    TypedResourceManager() {}

    void deleteObject(const Context *context, IDType handle);
    ANGLE_INLINE bool isHandleGenerated(IDType handle) const
    {
        // Zero is always assumed to have been generated implicitly.
        return GetIDValue(handle) == 0 || mObjectMap.contains(handle);
    }

    const ResourceMap<ResourceType, IDType> &getResourcesForCapture() const { return mObjectMap; }

  protected:
    ~TypedResourceManager() override;

    // Inlined in the header for performance.
    template <typename... ArgTypes>
    ANGLE_INLINE ResourceType *checkObjectAllocation(rx::GLImplFactory *factory,
                                                     IDType handle,
                                                     ArgTypes... args)
    {
        ResourceType *value = mObjectMap.query(handle);
        if (value)
        {
            return value;
        }

        if (GetIDValue(handle) == 0)
        {
            return nullptr;
        }

        return checkObjectAllocationImpl(factory, handle, args...);
    }

    void reset(const Context *context) override;

    ResourceMap<ResourceType, IDType> mObjectMap;

  private:
    template <typename... ArgTypes>
    ResourceType *checkObjectAllocationImpl(rx::GLImplFactory *factory,
                                            IDType handle,
                                            ArgTypes... args)
    {
        ResourceType *object = ImplT::AllocateNewObject(factory, handle, args...);

        if (!mObjectMap.contains(handle))
        {
            this->mHandleAllocator.reserve(GetIDValue(handle));
        }
        mObjectMap.assign(handle, object);

        return object;
    }
};

template <typename ResourceType, typename ImplT, typename IDType>
class TypedResourceManagerWithTotalMemorySize
    : public TypedResourceManager<ResourceType, ImplT, IDType>
{
  public:
    size_t getTotalMemorySize() const
    {
        size_t totalBytes = 0;

        for (const auto &rb : UnsafeResourceMapIter(this->mObjectMap))
        {
            totalBytes += static_cast<size_t>(rb.second->getMemorySize());
        }
        return totalBytes;
    }
};

class BufferManager
    : public TypedResourceManagerWithTotalMemorySize<Buffer, BufferManager, BufferID>
{
  public:
    BufferID createBuffer();
    Buffer *getBuffer(BufferID handle) const;

    ANGLE_INLINE Buffer *checkBufferAllocation(rx::GLImplFactory *factory, BufferID handle)
    {
        return checkObjectAllocation(factory, handle);
    }

    // TODO(jmadill): Investigate design which doesn't expose these methods publicly.
    static Buffer *AllocateNewObject(rx::GLImplFactory *factory, BufferID handle);
    static void DeleteObject(const Context *context, Buffer *buffer);

  protected:
    ~BufferManager() override;
};

class ShaderProgramManager : public ResourceManagerBase
{
  public:
    ShaderProgramManager();

    ShaderProgramID createShader(rx::GLImplFactory *factory,
                                 const Limitations &rendererLimitations,
                                 ShaderType type);
    void deleteShader(const Context *context, ShaderProgramID shader);
    Shader *getShader(ShaderProgramID handle) const;

    ShaderProgramID createProgram(rx::GLImplFactory *factory);
    void deleteProgram(const Context *context, ShaderProgramID program);

    ANGLE_INLINE Program *getProgram(ShaderProgramID handle) const
    {
        return mPrograms.query(handle);
    }

    // For capture and performance counters only.
    const ResourceMap<Shader, ShaderProgramID> &getShadersForCapture() const { return mShaders; }
    const ResourceMap<Program, ShaderProgramID> &getProgramsForCaptureAndPerf() const
    {
        return mPrograms;
    }

  protected:
    ~ShaderProgramManager() override;

  private:
    template <typename ObjectType, typename IDType>
    void deleteObject(const Context *context,
                      ResourceMap<ObjectType, IDType> *objectMap,
                      IDType id);

    void reset(const Context *context) override;

    ResourceMap<Shader, ShaderProgramID> mShaders;
    ResourceMap<Program, ShaderProgramID> mPrograms;
};

class TextureManager : public TypedResourceManager<Texture, TextureManager, TextureID>
{
  public:
    TextureID createTexture();
    ANGLE_INLINE Texture *getTexture(TextureID handle) const
    {
        ASSERT(mObjectMap.query({0}) == nullptr);
        return mObjectMap.query(handle);
    }

    void signalAllTexturesDirty() const;

    ANGLE_INLINE Texture *checkTextureAllocation(rx::GLImplFactory *factory,
                                                 TextureID handle,
                                                 TextureType type)
    {
        return checkObjectAllocation(factory, handle, type);
    }

    static Texture *AllocateNewObject(rx::GLImplFactory *factory,
                                      TextureID handle,
                                      TextureType type);
    static void DeleteObject(const Context *context, Texture *texture);

    void enableHandleAllocatorLogging();

    size_t getTotalMemorySize() const;

  protected:
    ~TextureManager() override;
};

class RenderbufferManager : public TypedResourceManagerWithTotalMemorySize<Renderbuffer,
                                                                           RenderbufferManager,
                                                                           RenderbufferID>
{
  public:
    RenderbufferID createRenderbuffer();
    Renderbuffer *getRenderbuffer(RenderbufferID handle) const;

    Renderbuffer *checkRenderbufferAllocation(rx::GLImplFactory *factory, RenderbufferID handle)
    {
        return checkObjectAllocation(factory, handle);
    }

    static Renderbuffer *AllocateNewObject(rx::GLImplFactory *factory, RenderbufferID handle);
    static void DeleteObject(const Context *context, Renderbuffer *renderbuffer);

  protected:
    ~RenderbufferManager() override;
};

class SamplerManager : public TypedResourceManager<Sampler, SamplerManager, SamplerID>
{
  public:
    SamplerID createSampler();
    Sampler *getSampler(SamplerID handle) const { return mObjectMap.query(handle); }
    bool isSampler(SamplerID sampler) const { return mObjectMap.contains(sampler); }

    Sampler *checkSamplerAllocation(rx::GLImplFactory *factory, SamplerID handle)
    {
        return checkObjectAllocation(factory, handle);
    }

    static Sampler *AllocateNewObject(rx::GLImplFactory *factory, SamplerID handle);
    static void DeleteObject(const Context *context, Sampler *sampler);

  protected:
    ~SamplerManager() override;
};

class SyncManager : public TypedResourceManager<Sync, SyncManager, SyncID>
{
  public:
    SyncID createSync(rx::GLImplFactory *factory);
    Sync *getSync(SyncID handle) const;

    static void DeleteObject(const Context *context, Sync *sync);

  protected:
    ~SyncManager() override;
};

class FramebufferManager
    : public TypedResourceManager<Framebuffer, FramebufferManager, FramebufferID>
{
  public:
    FramebufferID createFramebuffer();
    Framebuffer *getFramebuffer(FramebufferID handle) const;
    void setDefaultFramebuffer(Framebuffer *framebuffer);
    Framebuffer *getDefaultFramebuffer() const;

    void invalidateFramebufferCompletenessCache() const;

    Framebuffer *checkFramebufferAllocation(rx::GLImplFactory *factory,
                                            const Context *context,
                                            FramebufferID handle)
    {
        return checkObjectAllocation(factory, handle, context);
    }

    static Framebuffer *AllocateNewObject(rx::GLImplFactory *factory,
                                          FramebufferID handle,
                                          const Context *context);
    static void DeleteObject(const Context *context, Framebuffer *framebuffer);

  protected:
    ~FramebufferManager() override;
};

class ProgramPipelineManager
    : public TypedResourceManager<ProgramPipeline, ProgramPipelineManager, ProgramPipelineID>
{
  public:
    ProgramPipelineID createProgramPipeline();
    ProgramPipeline *getProgramPipeline(ProgramPipelineID handle) const;

    ProgramPipeline *checkProgramPipelineAllocation(rx::GLImplFactory *factory,
                                                    ProgramPipelineID handle)
    {
        return checkObjectAllocation(factory, handle);
    }

    static ProgramPipeline *AllocateNewObject(rx::GLImplFactory *factory, ProgramPipelineID handle);
    static void DeleteObject(const Context *context, ProgramPipeline *pipeline);

  protected:
    ~ProgramPipelineManager() override;
};

class MemoryObjectManager : public ResourceManagerBase
{
  public:
    MemoryObjectManager();

    MemoryObjectID createMemoryObject(rx::GLImplFactory *factory);
    void deleteMemoryObject(const Context *context, MemoryObjectID handle);
    MemoryObject *getMemoryObject(MemoryObjectID handle) const;

  protected:
    ~MemoryObjectManager() override;

  private:
    void reset(const Context *context) override;

    ResourceMap<MemoryObject, MemoryObjectID> mMemoryObjects;
};

class SemaphoreManager : public ResourceManagerBase
{
  public:
    SemaphoreManager();

    SemaphoreID createSemaphore(rx::GLImplFactory *factory);
    void deleteSemaphore(const Context *context, SemaphoreID handle);
    Semaphore *getSemaphore(SemaphoreID handle) const;

  protected:
    ~SemaphoreManager() override;

  private:
    void reset(const Context *context) override;

    ResourceMap<Semaphore, SemaphoreID> mSemaphores;
};
}  // namespace gl

#endif  // LIBANGLE_RESOURCEMANAGER_H_
