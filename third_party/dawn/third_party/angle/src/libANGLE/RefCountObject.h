//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RefCountObject.h: Defines the gl::RefCountObject base class that provides
// lifecycle support for GL objects using the traditional BindObject scheme, but
// that need to be reference counted for correct cross-context deletion.
// (Concretely, textures, buffers and renderbuffers.)

#ifndef LIBANGLE_REFCOUNTOBJECT_H_
#define LIBANGLE_REFCOUNTOBJECT_H_

#include "angle_gl.h"
#include "common/PackedEnums.h"
#include "common/debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/Observer.h"
#include "libANGLE/renderer/serial_utils.h"

#include <cstddef>

namespace angle
{

template <typename ContextT, typename ErrorT>
class RefCountObject : angle::NonCopyable
{
  public:
    using ContextType = ContextT;
    using ErrorType   = ErrorT;

    RefCountObject() : mRefCount(0) {}

    virtual void onDestroy(const ContextType *context) {}

    void addRef() const { ++mRefCount; }

    ANGLE_INLINE void release(const ContextType *context)
    {
        ASSERT(mRefCount > 0);
        if (--mRefCount == 0)
        {
            onDestroy(context);
            delete this;
        }
    }

    size_t getRefCount() const { return mRefCount; }

  protected:
    virtual ~RefCountObject() { ASSERT(mRefCount == 0); }

    mutable size_t mRefCount;
};

// Alternative base class to RefCountObject using atomics to track the reference count.
// Interchangable with RefCountObject for usage with BindingPointer and other wrappers.
template <typename ContextT, typename ErrorT>
class ThreadSafeRefCountObject : angle::NonCopyable
{
  public:
    using ContextType = ContextT;
    using ErrorType   = ErrorT;

    ThreadSafeRefCountObject() : mRefCount(0) {}

    virtual void onDestroy(const ContextType *context) {}

    void addRef() const { mRefCount.fetch_add(1, std::memory_order_relaxed); }

    ANGLE_INLINE void release(const ContextType *context)
    {
        ASSERT(mRefCount > 0);
        if (mRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
        {
            onDestroy(context);
            delete this;
        }
    }

    size_t getRefCount() const { return mRefCount; }

  protected:
    virtual ~ThreadSafeRefCountObject() { ASSERT(mRefCount == 0); }

    mutable std::atomic<size_t> mRefCount;
};

template <class ObjectType, typename ContextT, typename ErrorT = angle::Result>
class RefCountObjectReleaser : angle::NonCopyable
{
  public:
    using ContextType = ContextT;
    using ErrorType   = ErrorT;

    RefCountObjectReleaser() {}
    RefCountObjectReleaser(const ContextType *context, ObjectType *object)
        : mContext(context), mObject(object)
    {}

    RefCountObjectReleaser(RefCountObjectReleaser &&other)
        : mContext(other.mContext), mObject(other.mObject)
    {
        other.mContext = nullptr;
        other.mObject  = nullptr;
    }

    RefCountObjectReleaser &operator=(RefCountObjectReleaser &&other)
    {
        std::swap(mContext, other.mContext);
        std::swap(mObject, other.mObject);
        return *this;
    }

    ~RefCountObjectReleaser()
    {
        if (mObject)
        {
            mObject->release(mContext);
            mObject = nullptr;
        }
    }

  private:
    const ContextType *mContext = nullptr;
    ObjectType *mObject         = nullptr;
};

template <class ObjectType, typename ContextT, typename ErrorT = angle::Result>
class BindingPointer
{
  public:
    using ContextType = ContextT;
    using ErrorType   = ErrorT;

    BindingPointer() : mObject(nullptr) {}

    BindingPointer(ObjectType *object) : mObject(object)
    {
        if (mObject)
        {
            mObject->addRef();
        }
    }

    BindingPointer(const BindingPointer &other) : mObject(other.mObject)
    {
        if (mObject)
        {
            mObject->addRef();
        }
    }

    BindingPointer &operator=(BindingPointer &&other)
    {
        std::swap(mObject, other.mObject);
        return *this;
    }

    virtual ~BindingPointer()
    {
        // Objects have to be released before the resource manager is destroyed, so they must be
        // explicitly cleaned up.
        ASSERT(mObject == nullptr);
    }

    RefCountObjectReleaser<ObjectType, ContextType, ErrorT> set(const ContextType *context,
                                                                ObjectType *newObject)
    {
        // addRef first in case newObject == mObject and this is the last reference to it.
        if (newObject != nullptr)
        {
            newObject->addRef();
        }

        // Store the old pointer in a temporary so we can set the pointer before calling release.
        // Otherwise the object could still be referenced when its destructor is called.
        ObjectType *oldObject = mObject;
        mObject               = newObject;
        return RefCountObjectReleaser<ObjectType, ContextType, ErrorT>(context, oldObject);
    }

    void assign(ObjectType *object) { mObject = object; }

    ObjectType *get() const { return mObject; }
    ObjectType *operator->() const { return mObject; }

    bool operator==(const BindingPointer &other) const { return mObject == other.mObject; }

    bool operator!=(const BindingPointer &other) const { return !(*this == other); }

  protected:
    ANGLE_INLINE void setImpl(ObjectType *obj) { mObject = obj; }

  private:
    ObjectType *mObject;
};
}  // namespace angle

namespace gl
{
class Context;

template <class ObjectType>
class BindingPointer;

using RefCountObjectNoID           = angle::RefCountObject<Context, angle::Result>;
using ThreadSafeRefCountObjectNoID = angle::ThreadSafeRefCountObject<Context, angle::Result>;

template <typename IDType, typename RC = RefCountObjectNoID>
class RefCountObject : public RC
{
  public:
    explicit RefCountObject(rx::UniqueSerial serial, IDType id) : mSerial(serial), mId(id) {}

    rx::UniqueSerial serial() const { return mSerial; }
    IDType id() const { return mId; }

  protected:
    ~RefCountObject() override {}

  private:
    // Unique serials are used to identify resources for frame capture.
    rx::UniqueSerial mSerial;
    IDType mId;
};
template <typename IDType>
using ThreadSafeRefCountObject = RefCountObject<IDType, ThreadSafeRefCountObjectNoID>;

template <class ObjectType>
class BindingPointer : public angle::BindingPointer<ObjectType, Context>
{
  public:
    using ContextType = typename angle::BindingPointer<ObjectType, Context>::ContextType;
    using ErrorType   = typename angle::BindingPointer<ObjectType, Context>::ErrorType;

    BindingPointer() {}

    BindingPointer(ObjectType *object) : angle::BindingPointer<ObjectType, Context>(object) {}

    typename ResourceTypeToID<ObjectType>::IDType id() const
    {
        ObjectType *obj = this->get();
        if (obj)
            return obj->id();
        return {0};
    }
};

template <class ObjectType>
class OffsetBindingPointer : public BindingPointer<ObjectType>
{
  public:
    using ContextType = typename BindingPointer<ObjectType>::ContextType;
    using ErrorType   = typename BindingPointer<ObjectType>::ErrorType;

    OffsetBindingPointer() : mOffset(0), mSize(0) {}

    void set(const ContextType *context, ObjectType *newObject, GLintptr offset, GLsizeiptr size)
    {
        set(context, newObject);
        updateOffsetAndSize(newObject, offset, size);
    }

    GLintptr getOffset() const { return mOffset; }
    GLsizeiptr getSize() const { return mSize; }

    bool operator==(const OffsetBindingPointer<ObjectType> &other) const
    {
        return this->get() == other.get() && mOffset == other.mOffset && mSize == other.mSize;
    }

    bool operator!=(const OffsetBindingPointer<ObjectType> &other) const
    {
        return !(*this == other);
    }

    void assign(ObjectType *newObject, GLintptr offset, GLsizeiptr size)
    {
        assign(newObject);
        updateOffsetAndSize(newObject, offset, size);
    }

  private:
    ANGLE_INLINE void updateOffsetAndSize(ObjectType *newObject, GLintptr offset, GLsizeiptr size)
    {
        if (newObject)
        {
            mOffset = offset;
            mSize   = size;
        }
        else
        {
            mOffset = 0;
            mSize   = 0;
        }
    }

    // Delete the unparameterized functions. This forces an explicit offset and size.
    using BindingPointer<ObjectType>::set;
    using BindingPointer<ObjectType>::assign;

    GLintptr mOffset;
    GLsizeiptr mSize;
};

template <typename SubjectT>
class SubjectBindingPointer : protected BindingPointer<SubjectT>, public angle::ObserverBindingBase
{
  public:
    SubjectBindingPointer(angle::ObserverInterface *observer, angle::SubjectIndex index)
        : ObserverBindingBase(observer, index)
    {}
    ~SubjectBindingPointer() override {}
    SubjectBindingPointer(const SubjectBindingPointer &other)            = default;
    SubjectBindingPointer &operator=(const SubjectBindingPointer &other) = default;

    void bind(const Context *context, SubjectT *subject)
    {
        // AddRef first in case subject == get()
        if (subject)
        {
            subject->addObserver(this);
            subject->addRef();
        }

        if (get())
        {
            get()->removeObserver(this);
            get()->release(context);
        }

        this->setImpl(subject);
    }

    using BindingPointer<SubjectT>::get;
    using BindingPointer<SubjectT>::operator->;

    friend class State;
};
}  // namespace gl

namespace egl
{
class Display;

using RefCountObject           = angle::RefCountObject<Display, Error>;
using ThreadSafeRefCountObject = angle::ThreadSafeRefCountObject<Display, Error>;

template <class ObjectType>
using RefCountObjectReleaser = angle::RefCountObjectReleaser<ObjectType, Display, Error>;

template <class ObjectType>
using BindingPointer = angle::BindingPointer<ObjectType, Display, Error>;

}  // namespace egl

#endif  // LIBANGLE_REFCOUNTOBJECT_H_
