//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_MSL_REFERENCE_H_
#define COMPILER_TRANSLATOR_MSL_REFERENCE_H_

namespace sh
{

// Similar to std::reference_wrapper, but also lifts comparison operators.
template <typename T>
class Ref
{
  public:
    Ref(const Ref &) = default;
    Ref(Ref &&)      = default;
    Ref(T &ref) : mPtr(&ref) {}

    Ref &operator=(const Ref &) = default;
    Ref &operator=(Ref &&)      = default;

    bool operator==(const Ref &other) const { return *mPtr == *other.mPtr; }
    bool operator!=(const Ref &other) const { return *mPtr != *other.mPtr; }
    bool operator<=(const Ref &other) const { return *mPtr <= *other.mPtr; }
    bool operator>=(const Ref &other) const { return *mPtr >= *other.mPtr; }
    bool operator<(const Ref &other) const { return *mPtr < *other.mPtr; }
    bool operator>(const Ref &other) const { return *mPtr > *other.mPtr; }

    T &get() { return *mPtr; }
    T const &get() const { return *mPtr; }

    operator T &() { return *mPtr; }
    operator T const &() const { return *mPtr; }

  private:
    T *mPtr;
};

template <typename T>
using CRef = Ref<T const>;

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_MSL_REFERENCE_H_
