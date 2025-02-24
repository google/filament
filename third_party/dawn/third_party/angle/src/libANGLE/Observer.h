//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Observer:
//   Implements the Observer pattern for sending state change notifications
//   from Subject objects to dependent Observer objects.
//
//   See design document:
//   https://docs.google.com/document/d/15Edfotqg6_l1skTEL8ADQudF_oIdNa7i8Po43k6jMd4/

#ifndef LIBANGLE_OBSERVER_H_
#define LIBANGLE_OBSERVER_H_

#include "common/FastVector.h"
#include "common/angleutils.h"
#include "libANGLE/Constants.h"

namespace angle
{
template <typename HaystackT, typename NeedleT>
bool IsInContainer(const HaystackT &haystack, const NeedleT &needle)
{
    return std::find(haystack.begin(), haystack.end(), needle) != haystack.end();
}

using SubjectIndex = size_t;

// Messages are used to distinguish different Subject events that get sent to a single Observer.
// It could be possible to improve the handling by using different callback functions instead
// of a single handler function. But in some cases we want to share a single binding between
// Observer and Subject and handle different types of events.
enum class SubjectMessage
{
    // Used by gl::VertexArray to notify gl::Context of a gl::Buffer binding count change. Triggers
    // a validation cache update. Also used by gl::Texture to notify gl::Framebuffer of loops.
    BindingChanged,

    // Only the contents (pixels, bytes, etc) changed in this Subject. Distinct from the object
    // storage.
    ContentsChanged,

    // Sent by gl::Sampler, gl::Texture, gl::Framebuffer and others to notifiy gl::Context. This
    // flag indicates to call syncState before next use.
    DirtyBitsFlagged,

    // Generic state change message. Used in multiple places for different purposes.
    SubjectChanged,

    // Indicates a bound gl::Buffer is now mapped or unmapped. Passed from gl::Buffer, through
    // gl::VertexArray, into gl::Context. Used to track validation.
    SubjectMapped,
    SubjectUnmapped,
    // Indicates a bound buffer's storage was reallocated due to glBufferData call or optimizations
    // to prevent having to flush pending commands and waiting for the GPU to become idle.
    InternalMemoryAllocationChanged,

    // Indicates an external change to the default framebuffer.
    SurfaceChanged,
    // Indicates the system framebuffer's swapchain changed, i.e. color buffer changed but no
    // depth/stencil buffer change.
    SwapchainImageChanged,

    // Indicates a separable program's textures or images changed in the ProgramExecutable.
    ProgramTextureOrImageBindingChanged,
    // Indicates a program or pipeline is being re-linked.  This is used to make sure the Context or
    // ProgramPipeline that reference the program/pipeline wait for it to finish linking.
    ProgramUnlinked,
    // Indicates a program or pipeline was successfully re-linked.
    ProgramRelinked,
    // Indicates a separable program's sampler uniforms were updated.
    SamplerUniformsUpdated,
    // Indicates a program's uniform block binding has changed (one message per binding)
    ProgramUniformBlockBindingZeroUpdated,
    ProgramUniformBlockBindingLastUpdated = ProgramUniformBlockBindingZeroUpdated +
                                            gl::IMPLEMENTATION_MAX_COMBINED_SHADER_UNIFORM_BUFFERS -
                                            1,

    // Indicates a Storage of back-end in gl::Texture has been released.
    StorageReleased,

    // Sent when the GLuint ID for a gl::Texture is being deleted via glDeleteTextures. The
    // texture may stay alive due to orphaning, but will no longer be directly accessible by the GL
    // API.
    TextureIDDeleted,

    // Indicates that all pending updates are complete in the subject.
    InitializationComplete,

    // Indicates a change in foveated rendering state in the subject.
    FoveatedRenderingStateChanged,
};

inline bool IsProgramUniformBlockBindingUpdatedMessage(SubjectMessage message)
{
    return message >= SubjectMessage::ProgramUniformBlockBindingZeroUpdated &&
           message <= SubjectMessage::ProgramUniformBlockBindingLastUpdated;
}
inline SubjectMessage ProgramUniformBlockBindingUpdatedMessageFromIndex(uint32_t blockIndex)
{
    return static_cast<SubjectMessage>(
        static_cast<uint32_t>(SubjectMessage::ProgramUniformBlockBindingZeroUpdated) + blockIndex);
}
inline uint32_t ProgramUniformBlockBindingUpdatedMessageToIndex(SubjectMessage message)
{
    return static_cast<uint32_t>(message) -
           static_cast<uint32_t>(SubjectMessage::ProgramUniformBlockBindingZeroUpdated);
}

// The observing class inherits from this interface class.
class ObserverInterface
{
  public:
    virtual ~ObserverInterface();
    virtual void onSubjectStateChange(SubjectIndex index, SubjectMessage message) = 0;
};

class ObserverBindingBase
{
  public:
    ObserverBindingBase(ObserverInterface *observer, SubjectIndex subjectIndex)
        : mObserver(observer), mIndex(subjectIndex)
    {}
    virtual ~ObserverBindingBase() {}

    ObserverBindingBase(const ObserverBindingBase &other)            = default;
    ObserverBindingBase &operator=(const ObserverBindingBase &other) = default;

    ObserverInterface *getObserver() const { return mObserver; }
    SubjectIndex getSubjectIndex() const { return mIndex; }

    virtual void onSubjectReset() {}

  private:
    ObserverInterface *mObserver;
    SubjectIndex mIndex;
};

constexpr size_t kMaxFixedObservers = 8;

// Maintains a list of observer bindings. Sends update messages to the observer.
class Subject : NonCopyable
{
  public:
    Subject();
    virtual ~Subject();

    void onStateChange(SubjectMessage message) const;
    bool hasObservers() const;
    void resetObservers();
    ANGLE_INLINE size_t getObserversCount() const { return mObservers.size(); }

    ANGLE_INLINE void addObserver(ObserverBindingBase *observer)
    {
        ASSERT(!IsInContainer(mObservers, observer));
        mObservers.push_back(observer);
    }
    ANGLE_INLINE void removeObserver(ObserverBindingBase *observer)
    {
        ASSERT(IsInContainer(mObservers, observer));
        mObservers.remove_and_permute(observer);
    }

  private:
    // Keep a short list of observers so we can allocate/free them quickly. But since we support
    // unlimited bindings, have a spill-over list of that uses dynamic allocation.
    angle::FastVector<ObserverBindingBase *, kMaxFixedObservers> mObservers;
};

// Keeps a binding between a Subject and Observer, with a specific subject index.
class ObserverBinding final : public ObserverBindingBase
{
  public:
    ObserverBinding();
    ObserverBinding(ObserverInterface *observer, SubjectIndex index);
    ~ObserverBinding() override;
    ObserverBinding(const ObserverBinding &other);
    ObserverBinding &operator=(const ObserverBinding &other);

    void bind(Subject *subject);

    ANGLE_INLINE void reset() { bind(nullptr); }

    void onStateChange(SubjectMessage message) const;
    void onSubjectReset() override;

    ANGLE_INLINE const Subject *getSubject() const { return mSubject; }

    ANGLE_INLINE void assignSubject(Subject *subject) { mSubject = subject; }

  private:
    Subject *mSubject;
};

}  // namespace angle

#endif  // LIBANGLE_OBSERVER_H_
