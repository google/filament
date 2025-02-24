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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_DATA_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_DATA_H_

#include <memory>
#include <unordered_map>
#include <utility>

#include "src/tint/utils/rtti/castable.h"

namespace tint::ast::transform {

/// Data is the base class for transforms that accept extra input or emit extra output information.
class Data : public Castable<Data> {
  public:
    /// Constructor
    Data();

    /// Copy constructor
    Data(const Data&);

    /// Destructor
    ~Data() override;

    /// Assignment operator
    /// @returns this Data
    Data& operator=(const Data&);
};

/// DataMap is a map of Data unique pointers keyed by the Data's ClassID.
class DataMap {
  public:
    /// Constructor
    DataMap();

    /// Move constructor
    DataMap(DataMap&&);

    /// Constructor
    /// @param data_unique_ptrs a variadic list of additional data unique_ptrs produced by the
    /// transform
    template <typename... DATA>
    explicit DataMap(DATA... data_unique_ptrs) {
        PutAll(std::forward<DATA>(data_unique_ptrs)...);
    }

    /// Destructor
    ~DataMap();

    /// Move assignment operator
    /// @param rhs the DataMap to move into this DataMap
    /// @return this DataMap
    DataMap& operator=(DataMap&& rhs);

    /// Adds the data into DataMap keyed by the ClassID of type T.
    /// @param data the data to add to the DataMap
    template <typename T>
    void Put(std::unique_ptr<T>&& data) {
        static_assert(std::is_base_of<Data, T>::value, "T does not derive from Data");
        map_[&tint::TypeInfo::Of<T>()] = std::move(data);
    }

    /// Creates the data of type `T` with the provided arguments and adds it into DataMap keyed by
    /// the ClassID of type T.
    /// @param args the arguments forwarded to the initializer for type T
    template <typename T, typename... ARGS>
    void Add(ARGS&&... args) {
        Put(std::make_unique<T>(std::forward<ARGS>(args)...));
    }

    /// @returns a pointer to the Data placed into the DataMap with a call to Put()
    template <typename T>
    T const* Get() const {
        return const_cast<DataMap*>(this)->Get<T>();
    }

    /// @returns a pointer to the Data placed into the DataMap with a call to Put()
    template <typename T>
    T* Get() {
        auto it = map_.find(&tint::TypeInfo::Of<T>());
        if (it == map_.end()) {
            return nullptr;
        }
        return static_cast<T*>(it->second.get());
    }

    /// Add moves all the data from other into this DataMap
    /// @param other the DataMap to move into this DataMap
    void Add(DataMap&& other) {
        for (auto& it : other.map_) {
            map_.emplace(it.first, std::move(it.second));
        }
        other.map_.clear();
    }

  private:
    template <typename T0>
    void PutAll(T0&& first) {
        Put(std::forward<T0>(first));
    }

    template <typename T0, typename... Tn>
    void PutAll(T0&& first, Tn&&... remainder) {
        Put(std::forward<T0>(first));
        PutAll(std::forward<Tn>(remainder)...);
    }

    std::unordered_map<const tint::TypeInfo*, std::unique_ptr<Data>> map_;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_DATA_H_
