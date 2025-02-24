//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SynchronizedValue.h:
//   A class that ensures that the correct mutex is locked when the encapsulated data is accessed.
//   Based on boost::synchronized_value, which probably becomes part of the next C++ standard.
// https://www.boost.org/doc/libs/1_76_0/doc/html/thread/sds.html#thread.sds.synchronized_valuesxxx

#ifndef COMMON_SYNCHRONIZEDVALUE_H_
#define COMMON_SYNCHRONIZEDVALUE_H_

#include "common/SimpleMutex.h"
#include "common/debug.h"

#include <mutex>
#include <type_traits>

namespace angle
{

template <typename T, typename Lockable = angle::SimpleMutex>
class ConstStrictLockPtr
{
  public:
    using value_type = T;
    using mutex_type = Lockable;

    ConstStrictLockPtr(const T &value, Lockable &mutex) : mLock(mutex), mValue(value) {}
    ConstStrictLockPtr(const T &value, Lockable &mutex, std::adopt_lock_t) noexcept
        : mLock(mutex, std::adopt_lock), mValue(value)
    {}

    ConstStrictLockPtr(ConstStrictLockPtr &&other) noexcept
        : mLock(std::move(other.mLock)), mValue(other.mValue)
    {}

    ConstStrictLockPtr(const ConstStrictLockPtr &)            = delete;
    ConstStrictLockPtr &operator=(const ConstStrictLockPtr &) = delete;

    ~ConstStrictLockPtr() = default;

    const T *operator->() const { return &mValue; }
    const T &operator*() const { return mValue; }

  protected:
    std::unique_lock<Lockable> mLock;
    T const &mValue;
};

template <typename T, typename Lockable = angle::SimpleMutex>
class StrictLockPtr : public ConstStrictLockPtr<T, Lockable>
{
  private:
    using BaseType = ConstStrictLockPtr<T, Lockable>;

  public:
    StrictLockPtr(T &value, Lockable &mutex) : BaseType(value, mutex) {}
    StrictLockPtr(T &value, Lockable &mutex, std::adopt_lock_t) noexcept
        : BaseType(value, mutex, std::adopt_lock)
    {}

    StrictLockPtr(StrictLockPtr &&other) noexcept
        : BaseType(std::move(static_cast<BaseType &&>(other)))
    {}

    StrictLockPtr(const StrictLockPtr &)            = delete;
    StrictLockPtr &operator=(const StrictLockPtr &) = delete;

    ~StrictLockPtr() = default;

    T *operator->() { return const_cast<T *>(&this->mValue); }
    T &operator*() { return const_cast<T &>(this->mValue); }
};

template <typename SV>
struct SynchronizedValueStrictLockPtr
{
    using type = StrictLockPtr<typename SV::value_type, typename SV::mutex_type>;
};

template <typename SV>
struct SynchronizedValueStrictLockPtr<const SV>
{
    using type = ConstStrictLockPtr<typename SV::value_type, typename SV::mutex_type>;
};

template <typename T, typename Lockable = angle::SimpleMutex>
class ConstUniqueLockPtr : public std::unique_lock<Lockable>
{
  private:
    using BaseType = std::unique_lock<Lockable>;

  public:
    using value_type = T;
    using mutex_type = Lockable;

    ConstUniqueLockPtr(const T &value, Lockable &mutex) : BaseType(mutex), mValue(value) {}
    ConstUniqueLockPtr(const T &value, Lockable &mutex, std::adopt_lock_t) noexcept
        : BaseType(mutex, std::adopt_lock), mValue(value)
    {}
    ConstUniqueLockPtr(const T &value, Lockable &mutex, std::defer_lock_t) noexcept
        : BaseType(mutex, std::defer_lock), mValue(value)
    {}
    ConstUniqueLockPtr(const T &value, Lockable &mutex, std::try_to_lock_t) noexcept
        : BaseType(mutex, std::try_to_lock), mValue(value)
    {}

    ConstUniqueLockPtr(ConstUniqueLockPtr &&other) noexcept
        : BaseType(std::move(static_cast<BaseType &&>(other))), mValue(other.mValue)
    {}

    ConstUniqueLockPtr(const ConstUniqueLockPtr &)            = delete;
    ConstUniqueLockPtr &operator=(const ConstUniqueLockPtr &) = delete;

    ~ConstUniqueLockPtr() = default;

    const T *operator->() const
    {
        ASSERT(this->owns_lock());
        return &mValue;
    }
    const T &operator*() const
    {
        ASSERT(this->owns_lock());
        return mValue;
    }

  protected:
    T const &mValue;
};

template <typename T, typename Lockable = angle::SimpleMutex>
class UniqueLockPtr : public ConstUniqueLockPtr<T, Lockable>
{
  private:
    using BaseType = ConstUniqueLockPtr<T, Lockable>;

  public:
    UniqueLockPtr(T &value, Lockable &mutex) : BaseType(value, mutex) {}
    UniqueLockPtr(T &value, Lockable &mutex, std::adopt_lock_t) noexcept
        : BaseType(value, mutex, std::adopt_lock)
    {}
    UniqueLockPtr(T &value, Lockable &mutex, std::defer_lock_t) noexcept
        : BaseType(value, mutex, std::defer_lock)
    {}
    UniqueLockPtr(T &value, Lockable &mutex, std::try_to_lock_t) noexcept
        : BaseType(value, mutex, std::try_to_lock)
    {}

    UniqueLockPtr(UniqueLockPtr &&other) noexcept
        : BaseType(std::move(static_cast<BaseType &&>(other)))
    {}

    UniqueLockPtr(const UniqueLockPtr &)            = delete;
    UniqueLockPtr &operator=(const UniqueLockPtr &) = delete;

    ~UniqueLockPtr() = default;

    T *operator->()
    {
        ASSERT(this->owns_lock());
        return const_cast<T *>(&this->mValue);
    }
    T &operator*()
    {
        ASSERT(this->owns_lock());
        return const_cast<T &>(this->mValue);
    }
};

template <typename SV>
struct SynchronizedValueUniqueLockPtr
{
    using type = UniqueLockPtr<typename SV::value_type, typename SV::mutex_type>;
};

template <typename SV>
struct SynchronizedValueUniqueLockPtr<const SV>
{
    using type = ConstUniqueLockPtr<typename SV::value_type, typename SV::mutex_type>;
};

template <typename T, typename Lockable = angle::SimpleMutex>
class SynchronizedValue
{
  public:
    using value_type = T;
    using mutex_type = Lockable;

    SynchronizedValue() noexcept(std::is_nothrow_default_constructible<T>::value) : mValue() {}

    SynchronizedValue(const T &other) noexcept(std::is_nothrow_copy_constructible<T>::value)
        : mValue(other)
    {}

    SynchronizedValue(T &&other) noexcept(std::is_nothrow_move_constructible<T>::value)
        : mValue(std::move(other))
    {}

    template <typename... Args>
    SynchronizedValue(Args &&...args) noexcept(noexcept(T(std::forward<Args>(args)...)))
        : mValue(std::forward<Args>(args)...)
    {}

    SynchronizedValue(const SynchronizedValue &other)
    {
        std::lock_guard<Lockable> lock(other.mMutex);
        mValue = other.mValue;
    }

    SynchronizedValue(SynchronizedValue &&other)
    {
        std::lock_guard<Lockable> lock(other.mMutex);
        mValue = std::move(other.mValue);
    }

    SynchronizedValue &operator=(const SynchronizedValue &other)
    {
        if (&other != this)
        {
            std::unique_lock<Lockable> lock1(mMutex, std::defer_lock);
            std::unique_lock<Lockable> lock2(other.mMutex, std::defer_lock);
            std::lock(lock1, lock2);
            mValue = other.mValue;
        }
        return *this;
    }

    SynchronizedValue &operator=(SynchronizedValue &&other)
    {
        if (&other != this)
        {
            std::unique_lock<Lockable> lock1(mMutex, std::defer_lock);
            std::unique_lock<Lockable> lock2(other.mMutex, std::defer_lock);
            std::lock(lock1, lock2);
            mValue = std::move(other.mValue);
        }
        return *this;
    }

    SynchronizedValue &operator=(const T &value)
    {
        {
            std::lock_guard<Lockable> lock(mMutex);
            mValue = value;
        }
        return *this;
    }

    SynchronizedValue &operator=(T &&value)
    {
        {
            std::lock_guard<Lockable> lock(mMutex);
            mValue = std::move(value);
        }
        return *this;
    }

    T get() const
    {
        std::lock_guard<Lockable> lock(mMutex);
        return mValue;
    }

    explicit operator T() const { return get(); }

    void swap(SynchronizedValue &other)
    {
        if (this == &other)
        {
            return;
        }
        std::unique_lock<Lockable> lock1(mMutex, std::defer_lock);
        std::unique_lock<Lockable> lock2(other.mMutex, std::defer_lock);
        std::lock(lock1, lock2);
        std::swap(mValue, other.mValue);
    }

    void swap(T &other)
    {
        std::lock_guard<Lockable> lock(mMutex);
        std::swap(mValue, other);
    }

    StrictLockPtr<T, Lockable> operator->() { return StrictLockPtr<T, Lockable>(mValue, mMutex); }
    ConstStrictLockPtr<T, Lockable> operator->() const
    {
        return ConstStrictLockPtr<T, Lockable>(mValue, mMutex);
    }

    StrictLockPtr<T, Lockable> synchronize() { return StrictLockPtr<T, Lockable>(mValue, mMutex); }
    ConstStrictLockPtr<T, Lockable> synchronize() const
    {
        return ConstStrictLockPtr<T, Lockable>(mValue, mMutex);
    }

    UniqueLockPtr<T, Lockable> unique_synchronize()
    {
        return UniqueLockPtr<T, Lockable>(mValue, mMutex);
    }
    ConstUniqueLockPtr<T, Lockable> unique_synchronize() const
    {
        return ConstUniqueLockPtr<T, Lockable>(mValue, mMutex);
    }

    UniqueLockPtr<T, Lockable> defer_synchronize() noexcept
    {
        return UniqueLockPtr<T, Lockable>(mValue, mMutex, std::defer_lock);
    }
    ConstUniqueLockPtr<T, Lockable> defer_synchronize() const noexcept
    {
        return ConstUniqueLockPtr<T, Lockable>(mValue, mMutex, std::defer_lock);
    }

    UniqueLockPtr<T, Lockable> try_to_synchronize() noexcept
    {
        return UniqueLockPtr<T, Lockable>(mValue, mMutex, std::try_to_lock);
    }
    ConstUniqueLockPtr<T, Lockable> try_to_synchronize() const noexcept
    {
        return ConstUniqueLockPtr<T, Lockable>(mValue, mMutex, std::try_to_lock);
    }

    UniqueLockPtr<T, Lockable> adopt_synchronize() noexcept
    {
        return UniqueLockPtr<T, Lockable>(mValue, mMutex, std::adopt_lock);
    }
    ConstUniqueLockPtr<T, Lockable> adopt_synchronize() const noexcept
    {
        return ConstUniqueLockPtr<T, Lockable>(mValue, mMutex, std::adopt_lock);
    }

    class DerefValue
    {
      public:
        DerefValue(DerefValue &&other) : mLock(std::move(other.mLock)), mValue(other.mValue) {}

        DerefValue(const DerefValue &)            = delete;
        DerefValue &operator=(const DerefValue &) = delete;

        operator T &() { return mValue; }

        DerefValue &operator=(const T &other)
        {
            mValue = other;
            return *this;
        }

      private:
        explicit DerefValue(SynchronizedValue &outer) : mLock(outer.mMutex), mValue(outer.mValue) {}

        std::unique_lock<Lockable> mLock;
        T &mValue;

        friend class SynchronizedValue;
    };

    class ConstDerefValue
    {
      public:
        ConstDerefValue(ConstDerefValue &&other)
            : mLock(std::move(other.mLock)), mValue(other.mValue)
        {}

        ConstDerefValue(const ConstDerefValue &)            = delete;
        ConstDerefValue &operator=(const ConstDerefValue &) = delete;

        operator const T &() { return mValue; }

      private:
        explicit ConstDerefValue(const SynchronizedValue &outer)
            : mLock(outer.mMutex), mValue(outer.mValue)
        {}

        std::unique_lock<Lockable> mLock;
        const T &mValue;

        friend class SynchronizedValue;
    };

    DerefValue operator*() { return DerefValue(*this); }
    ConstDerefValue operator*() const { return ConstDerefValue(*this); }

    template <typename OStream>
    void save(OStream &os) const
    {
        std::lock_guard<Lockable> lock(mMutex);
        os << mValue;
    }

    template <typename IStream>
    void load(IStream &is)
    {
        std::lock_guard<Lockable> lock(mMutex);
        is >> mValue;
    }

    bool operator==(const SynchronizedValue &other) const
    {
        std::unique_lock<Lockable> lock1(mMutex, std::defer_lock);
        std::unique_lock<Lockable> lock2(other.mMutex, std::defer_lock);
        std::lock(lock1, lock2);
        return mValue == other.mValue;
    }

    bool operator!=(const SynchronizedValue &other) const
    {
        std::unique_lock<Lockable> lock1(mMutex, std::defer_lock);
        std::unique_lock<Lockable> lock2(other.mMutex, std::defer_lock);
        std::lock(lock1, lock2);
        return mValue != other.mValue;
    }

    bool operator<(const SynchronizedValue &other) const
    {
        std::unique_lock<Lockable> lock1(mMutex, std::defer_lock);
        std::unique_lock<Lockable> lock2(other.mMutex, std::defer_lock);
        std::lock(lock1, lock2);
        return mValue < other.mValue;
    }

    bool operator>(const SynchronizedValue &other) const
    {
        std::unique_lock<Lockable> lock1(mMutex, std::defer_lock);
        std::unique_lock<Lockable> lock2(other.mMutex, std::defer_lock);
        std::lock(lock1, lock2);
        return mValue > other.mValue;
    }

    bool operator<=(const SynchronizedValue &other) const
    {
        std::unique_lock<Lockable> lock1(mMutex, std::defer_lock);
        std::unique_lock<Lockable> lock2(other.mMutex, std::defer_lock);
        std::lock(lock1, lock2);
        return mValue <= other.mValue;
    }

    bool operator>=(const SynchronizedValue &other) const
    {
        std::unique_lock<Lockable> lock1(mMutex, std::defer_lock);
        std::unique_lock<Lockable> lock2(other.mMutex, std::defer_lock);
        std::lock(lock1, lock2);
        return mValue >= other.mValue;
    }

    bool operator==(const T &other) const
    {
        std::lock_guard<Lockable> lock(mMutex);
        return mValue == other;
    }

    bool operator!=(const T &other) const
    {
        std::lock_guard<Lockable> lock(mMutex);
        return mValue != other;
    }

    bool operator<(const T &other) const
    {
        std::lock_guard<Lockable> lock(mMutex);
        return mValue < other;
    }

    bool operator>(const T &other) const
    {
        std::lock_guard<Lockable> lock(mMutex);
        return mValue > other;
    }

    bool operator<=(const T &other) const
    {
        std::lock_guard<Lockable> lock(mMutex);
        return mValue <= other;
    }

    bool operator>=(const T &other) const
    {
        std::lock_guard<Lockable> lock(mMutex);
        return mValue >= other;
    }

  private:
    T mValue;
    mutable Lockable mMutex;
};

template <typename OStream, typename T, typename L>
inline OStream &operator<<(OStream &os, SynchronizedValue<T, L> const &sv)
{
    sv.save(os);
    return os;
}

template <typename IStream, typename T, typename L>
inline IStream &operator>>(IStream &is, SynchronizedValue<T, L> &sv)
{
    sv.load(is);
    return is;
}

template <typename T, typename L>
bool operator==(const T &lhs, const SynchronizedValue<T, L> &rhs)
{
    return rhs == lhs;
}

template <typename T, typename L>
bool operator!=(const T &lhs, const SynchronizedValue<T, L> &rhs)
{
    return rhs != lhs;
}

template <typename T, typename L>
bool operator<(const T &lhs, const SynchronizedValue<T, L> &rhs)
{
    return rhs < lhs;
}

template <typename T, typename L>
bool operator>(const T &lhs, const SynchronizedValue<T, L> &rhs)
{
    return rhs > lhs;
}

template <typename T, typename L>
bool operator<=(const T &lhs, const SynchronizedValue<T, L> &rhs)
{
    return rhs <= lhs;
}

template <typename T, typename L>
bool operator>=(const T &lhs, const SynchronizedValue<T, L> &rhs)
{
    return rhs >= lhs;
}

}  // namespace angle

#endif  // COMMON_SYNCHRONIZEDVALUE_H_
