// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_CONTAINERS_CONST_PROPAGATING_PTR_H_
#define SRC_TINT_UTILS_CONTAINERS_CONST_PROPAGATING_PTR_H_

namespace tint {

/// ConstPropagatingPtr is a `const` propagating pointer - if the ConstPropagatingPtr is const, then
/// you will only be able to obtain a const pointee pointer.
template <typename T>
class ConstPropagatingPtr {
  public:
    /// Constructor.
    /// The pointer is initialized to null.
    ConstPropagatingPtr() : ptr_(nullptr) {}

    /// Constructor.
    /// @param ptr the pointer value.
    ConstPropagatingPtr(T* ptr) : ptr_(ptr) {}  // NOLINT(runtime/explicit)

    /// Move constructor.
    /// @param other the ConstPropagatingPtr to move.
    ConstPropagatingPtr(ConstPropagatingPtr&& other) = default;

    /// Copy (non-const) constructor.
    /// @param other the ConstPropagatingPtr to move.
    ConstPropagatingPtr(ConstPropagatingPtr& other) : ptr_(other.ptr_) {}

    /// Assignment operator
    /// @param ptr the new pointer value
    /// @return this ConstPropagatingPtr
    ConstPropagatingPtr& operator=(T* ptr) {
        ptr_ = ptr;
        return *this;
    }

    /// Move assignment operator.
    /// @param other the ConstPropagatingPtr to move.
    /// @return this ConstPropagatingPtr
    ConstPropagatingPtr& operator=(ConstPropagatingPtr&& other) = default;

    /// Copy (non-const) assignment operator.
    /// @param other the ConstPropagatingPtr to move.
    /// @return this ConstPropagatingPtr
    ConstPropagatingPtr& operator=(ConstPropagatingPtr& other) {
        ptr_ = other.ptr_;
        return *this;
    }

    /// @returns the pointer
    operator T*() { return ptr_; }

    /// @returns the const pointer
    operator const T*() const { return ptr_; }

    /// @returns the pointer
    T* operator->() { return ptr_; }

    /// @returns the const pointer
    const T* operator->() const { return ptr_; }

    /// @returns the pointer
    T* Get() { return ptr_; }

    /// @returns the const pointer
    const T* Get() const { return ptr_; }

    /// Equality operator
    /// @param ptr the pointer to compare against
    /// @return true if the pointers are equal
    bool operator==(T* ptr) const { return ptr_ == ptr; }

    /// Inequality operator
    /// @param ptr the pointer to compare against
    /// @return true if the pointers are not equal
    bool operator!=(T* ptr) const { return ptr_ != ptr; }

  private:
    // Delete copy constructor and copy-assignment to prevent making a copy of the
    // ConstPropagatingPtr to break const propagation.
    ConstPropagatingPtr(const ConstPropagatingPtr&) = delete;
    ConstPropagatingPtr& operator=(const ConstPropagatingPtr&) = delete;

    T* ptr_ = nullptr;
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_CONST_PROPAGATING_PTR_H_
