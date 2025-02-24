// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_SUBRESOURCESTORAGE_H_
#define SRC_DAWN_NATIVE_SUBRESOURCESTORAGE_H_

#include <array>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/TypeTraits.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/Error.h"
#include "dawn/native/Subresource.h"

namespace dawn::native {

// SubresourceStorage<T> acts like a simple map from subresource (aspect, layer, level) to a
// value of type T except that it tries to compress similar subresources so that algorithms
// can act on a whole range of subresources at once if they have the same state.
//
// For example a very common case to optimize for is the tracking of the usage of texture
// subresources inside a render pass: the vast majority of texture views will select the whole
// texture while a small minority will select a sub-range. We want to optimize the common case
// by setting and checking a single "usage" value when a full subresource is used but at the
// same time allow per-subresource data when needed.
//
// Another example is barrier tracking per-subresource in the backends: it will often happen
// that during texture upload each mip level will have a different "barrier state". However
// when the texture is fully uploaded and after it is used for sampling (with a full view) for
// the first time, the barrier state will likely be the same across all the subresources.
// That's why some form of "recompression" of subresource state must be possibe.
//
// In order to keep the implementation details private and to avoid iterator-hell, this
// container uses a more functional approach of calling a closure on the interesting ranges.
// This is for example how to look at the state of all subresources.
//
//   subresources.Iterate([](const SubresourceRange& range, const T& data) {
//      // Do something with the knowledge that all the subresources in `range` have value
//      // `data`.
//   });
//
// SubresourceStorage internally tracks compression state per aspect and then per layer of each
// aspect. This means that a 2-aspect texture can have the following compression state:
//
//  - Aspect 0 is fully compressed.
//  - Aspect 1 is partially compressed:
//    - Aspect 1 layer 3 is decompressed.
//    - Aspect 1 layer 0-2 and 4-42 are compressed.
//
// A useful model to reason about SubresourceStorage is to represent is as a tree:
//
//  - SubresourceStorage is the root.
//    |-> Nodes 1 deep represent each aspect. If an aspect is compressed, its node doesn't have
//       any children because the data is constant across all of the subtree.
//      |-> Nodes 2 deep represent layers (for uncompressed aspects). If a layer is compressed,
//         its node doesn't have any children because the data is constant across all of the
//         subtree.
//        |-> Nodes 3 deep represent individial mip levels (for uncompressed layers).
//
// The concept of recompression is the removal of all child nodes of a non-leaf node when the
// data is constant across them. Decompression is the addition of child nodes to a leaf node
// and copying of its data to all its children.
//
// The choice of having secondary compression for array layers is to optimize for the cases
// where transfer operations are used to update specific layers of texture with render or
// transfer operations, while the rest is untouched. It seems much less likely that there
// would be operations that touch all Nth mips of a 2D array texture without touching the
// others.
//
// There are several hot code paths that create new SubresourceStorage like the tracking of
// resource usage per-pass. We don't want to allocate a container for the decompressed data
// unless we have to because it would dramatically lower performance. Instead
// SubresourceStorage contains an inline array that contains the per-aspect compressed data
// and only allocates a per-subresource on aspect decompression.
//
// T must be a copyable type that supports equality comparison with ==.
//
// The implementation of functions in this file can have a lot of control flow and corner cases
// so each modification should come with extensive tests and ensure 100% code coverage of the
// modified functions. See instructions at
// https://chromium.googlesource.com/chromium/src/+/main/docs/testing/code_coverage.md#local-coverage-script
// to run the test with code coverage. A command line that worked in the past (with the right
// GN args for the out/coverage directory in a Chromium checkout) is:
//
/*
   python tools/code_coverage/coverage.py dawn_unittests -b out/coverage -o out/report -c \
       "out/coverage/dawn_unittests --gtest_filter=SubresourceStorage\*" -f \
       third_party/dawn/src/dawn/native
*/
//
// TODO(crbug.com/dawn/836): Make the recompression optional, the calling code should know
// if recompression can happen or not in Update() and Merge()
template <typename T>
class SubresourceStorage {
  public:
    static_assert(std::is_copy_assignable<T>::value, "T must be copyable");
    static_assert(HasEqualityOperator<T>::value, "T requires bool operator == (T, T)");

    // Creates the storage with the given "dimensions" and all subresources starting with the
    // initial value.
    SubresourceStorage(Aspect aspects,
                       uint32_t arrayLayerCount,
                       uint32_t mipLevelCount,
                       const T& initialValue = {});

    // Returns the data for a single subresource. Note that the reference returned might be the
    // same for multiple subresources.
    const T& Get(Aspect aspect, uint32_t arrayLayer, uint32_t mipLevel) const;

    // Fill the storage with a single value for all subresources, resulting in a fully
    // compressed storage.
    void Fill(const T& value);

    // Given an iterateFunc that's a function or function-like object that can be called with
    // arguments of type (const SubresourceRange& range, const T& data) and returns either void or
    // MaybeError, calls it with aggregate ranges if possible, such that each subresource is part of
    // exactly one of the ranges iterateFunc is called with (and obviously data is the value
    // stored for that subresource). Note that for MaybeError version, Iterate will return on the
    // first error. Example usages:
    //
    //   // Returning void version:
    //   subresources.Iterate([&](const SubresourceRange& range, const T& data) {
    //       // Do something with range and data.
    //   });
    //
    //   // Return MaybeError version:
    //   DAWN_TRY(subresources.Iterate(
    //       [&](const SubresourceRange& range, const T& data) -> MaybeError {
    //           // Do something with range and data.
    //           // Return a MaybeError.
    //       })
    //   );
    template <typename F, typename R = std::invoke_result_t<F, const SubresourceRange&, const T&>>
    R Iterate(F&& iterateFunc) const;

    // Given an updateFunc that's a function or function-like objet that can be called with
    // arguments of type (const SubresourceRange& range, T* data) and returns void,
    // calls it with ranges that in aggregate form `range` and pass for each of the
    // sub-ranges a pointer to modify the value for that sub-range. For example:
    //
    //   subresources.Update(view->GetRange(), [](const SubresourceRange&, T* data) {
    //       *data |= wgpu::TextureUsage::Stuff;
    //   });
    //
    // /!\ WARNING: updateFunc should never use range to compute the update to data otherwise
    // your code is likely to break when compression happens. Range should only be used for
    // side effects like using it to compute a Vulkan pipeline barrier.
    template <typename F>
    void Update(const SubresourceRange& range, F&& updateFunc);

    // Given a mergeFunc that's a function or a function-like object that can be called with
    // arguments of type (const SubresourceRange& range, T* data, const U& otherData) and
    // returns void, calls it with ranges that in aggregate form the full resources and pass
    // for each of the sub-ranges a pointer to modify the value for that sub-range and the
    // corresponding value from other for that sub-range. For example:
    //
    //   subresources.Merge(otherUsages,
    //       [](const SubresourceRange&, T* data, const T& otherData) {
    //          *data |= otherData;
    //       });
    //
    // /!\ WARNING: mergeFunc should never use range to compute the update to data otherwise
    // your code is likely to break when compression happens. Range should only be used for
    // side effects like using it to compute a Vulkan pipeline barrier.
    template <typename U, typename F>
    void Merge(const SubresourceStorage<U>& other, F&& mergeFunc);

    // Other operations to consider:
    //
    //  - UpdateTo(Range, T) that updates the range to a constant value.

    // Methods to query the internal state of SubresourceStorage for testing.
    Aspect GetAspectsForTesting() const;
    uint32_t GetArrayLayerCountForTesting() const;
    uint32_t GetMipLevelCountForTesting() const;
    bool IsAspectCompressedForTesting(Aspect aspect) const;
    bool IsLayerCompressedForTesting(Aspect aspect, uint32_t layer) const;

  private:
    template <typename U>
    friend class SubresourceStorage;

    void DecompressAspect(uint32_t aspectIndex);
    void RecompressAspect(uint32_t aspectIndex);

    void DecompressLayer(uint32_t aspectIndex, uint32_t layer);
    void RecompressLayer(uint32_t aspectIndex, uint32_t layer);

    SubresourceRange GetFullLayerRange(Aspect aspect, uint32_t layer) const;

    // LayerCompressed should never be called when the aspect is compressed otherwise it would
    // need to check that mLayerCompressed is not null before indexing it.
    bool& LayerCompressed(uint32_t aspectIndex, uint32_t layerIndex);
    bool LayerCompressed(uint32_t aspectIndex, uint32_t layerIndex) const;

    // Return references to the data for a compressed plane / layer or subresource.
    // Each variant should be called exactly under the correct compression level.
    T& DataInline(uint32_t aspectIndex);
    T& Data(uint32_t aspectIndex, uint32_t layer, uint32_t level = 0);
    const T& DataInline(uint32_t aspectIndex) const;
    const T& Data(uint32_t aspectIndex, uint32_t layer, uint32_t level = 0) const;

    Aspect mAspects;
    uint8_t mMipLevelCount;
    uint16_t mArrayLayerCount;

    // Invariant: if an aspect is marked compressed, then all it's layers are marked as
    // compressed.
    static constexpr size_t kMaxAspects = 3;
    std::array<bool, kMaxAspects> mAspectCompressed;
    std::array<T, kMaxAspects> mInlineAspectData;

    // Indexed as mLayerCompressed[aspectIndex * mArrayLayerCount + layer].
    std::unique_ptr<bool[]> mLayerCompressed;

    // Indexed as mData[(aspectIndex * mArrayLayerCount + layer) * mMipLevelCount + level].
    // The data for a compressed aspect is stored in the slot for (aspect, 0, 0). Similarly
    // the data for a compressed layer of aspect if in the slot for (aspect, layer, 0).
    std::unique_ptr<T[]> mData;
};

template <typename T>
SubresourceStorage<T>::SubresourceStorage(Aspect aspects,
                                          uint32_t arrayLayerCount,
                                          uint32_t mipLevelCount,
                                          const T& initialValue)
    : mAspects(aspects), mMipLevelCount(mipLevelCount), mArrayLayerCount(arrayLayerCount) {
    DAWN_ASSERT(arrayLayerCount <= std::numeric_limits<decltype(mArrayLayerCount)>::max());
    DAWN_ASSERT(mipLevelCount <= std::numeric_limits<decltype(mMipLevelCount)>::max());

    Fill(initialValue);
}

template <typename T>
void SubresourceStorage<T>::Fill(const T& value) {
    uint32_t aspectCount = GetAspectCount(mAspects);
    DAWN_ASSERT(aspectCount <= kMaxAspects);

    for (uint32_t aspectIndex = 0; aspectIndex < aspectCount; aspectIndex++) {
        mAspectCompressed[aspectIndex] = true;
        DataInline(aspectIndex) = value;
    }
}

template <typename T>
template <typename F>
void SubresourceStorage<T>::Update(const SubresourceRange& range, F&& updateFunc) {
    DAWN_ASSERT(range.baseArrayLayer < mArrayLayerCount &&
                range.baseArrayLayer + range.layerCount <= mArrayLayerCount);
    DAWN_ASSERT(range.baseMipLevel < mMipLevelCount &&
                range.baseMipLevel + range.levelCount <= mMipLevelCount);

    bool fullLayers = range.baseMipLevel == 0 && range.levelCount == mMipLevelCount;
    bool fullAspects =
        range.baseArrayLayer == 0 && range.layerCount == mArrayLayerCount && fullLayers;

    for (Aspect aspect : IterateEnumMask(range.aspects)) {
        uint32_t aspectIndex = GetAspectIndex(aspect);

        // Call the updateFunc once for the whole aspect if possible or decompress and fallback
        // to per-layer handling.
        if (mAspectCompressed[aspectIndex]) {
            if (fullAspects) {
                SubresourceRange updateRange =
                    SubresourceRange::MakeFull(aspect, mArrayLayerCount, mMipLevelCount);
                updateFunc(updateRange, &DataInline(aspectIndex));
                continue;
            }
            DecompressAspect(aspectIndex);
        }

        uint32_t layerEnd = range.baseArrayLayer + range.layerCount;
        for (uint32_t layer = range.baseArrayLayer; layer < layerEnd; layer++) {
            // Call the updateFunc once for the whole layer if possible or decompress and
            // fallback to per-level handling.
            if (LayerCompressed(aspectIndex, layer)) {
                if (fullLayers) {
                    SubresourceRange updateRange = GetFullLayerRange(aspect, layer);
                    updateFunc(updateRange, &Data(aspectIndex, layer));
                    continue;
                }
                DecompressLayer(aspectIndex, layer);
            }

            // Worst case: call updateFunc per level.
            uint32_t levelEnd = range.baseMipLevel + range.levelCount;
            for (uint32_t level = range.baseMipLevel; level < levelEnd; level++) {
                SubresourceRange updateRange = SubresourceRange::MakeSingle(aspect, layer, level);
                updateFunc(updateRange, &Data(aspectIndex, layer, level));
            }

            // If the range has fullLayers then it is likely we can recompress after the calls
            // to updateFunc (this branch is skipped if updateFunc was called for the whole
            // layer).
            if (fullLayers) {
                RecompressLayer(aspectIndex, layer);
            }
        }

        // If the range has fullAspects then it is likely we can recompress after the calls to
        // updateFunc (this branch is skipped if updateFunc was called for the whole aspect).
        if (fullAspects) {
            RecompressAspect(aspectIndex);
        }
    }
}

template <typename T>
template <typename U, typename F>
void SubresourceStorage<T>::Merge(const SubresourceStorage<U>& other, F&& mergeFunc) {
    DAWN_ASSERT(mAspects == other.mAspects);
    DAWN_ASSERT(mArrayLayerCount == other.mArrayLayerCount);
    DAWN_ASSERT(mMipLevelCount == other.mMipLevelCount);

    for (Aspect aspect : IterateEnumMask(mAspects)) {
        uint32_t aspectIndex = GetAspectIndex(aspect);

        // If the other storage's aspect is compressed we don't need to decompress anything
        // in `this` and can just iterate through it, merging with `other`'s constant value for
        // the aspect. For code simplicity this can be done with a call to Update().
        if (other.mAspectCompressed[aspectIndex]) {
            const U& otherData = other.DataInline(aspectIndex);
            Update(SubresourceRange::MakeFull(aspect, mArrayLayerCount, mMipLevelCount),
                   [&](const SubresourceRange& subrange, T* data) {
                       mergeFunc(subrange, data, otherData);
                   });
            continue;
        }

        // Other doesn't have the aspect compressed so we must do at least per-layer merging.
        if (mAspectCompressed[aspectIndex]) {
            DecompressAspect(aspectIndex);
        }

        for (uint32_t layer = 0; layer < mArrayLayerCount; layer++) {
            // Similarly to above, use a fast path if other's layer is compressed.
            if (other.LayerCompressed(aspectIndex, layer)) {
                const U& otherData = other.Data(aspectIndex, layer);
                Update(GetFullLayerRange(aspect, layer),
                       [&](const SubresourceRange& subrange, T* data) {
                           mergeFunc(subrange, data, otherData);
                       });
                continue;
            }

            // Sad case, other is decompressed for this layer, do per-level merging.
            if (LayerCompressed(aspectIndex, layer)) {
                DecompressLayer(aspectIndex, layer);
            }

            for (uint32_t level = 0; level < mMipLevelCount; level++) {
                SubresourceRange updateRange = SubresourceRange::MakeSingle(aspect, layer, level);
                mergeFunc(updateRange, &Data(aspectIndex, layer, level),
                          other.Data(aspectIndex, layer, level));
            }

            RecompressLayer(aspectIndex, layer);
        }

        RecompressAspect(aspectIndex);
    }
}

template <typename T>
template <typename F, typename R>
R SubresourceStorage<T>::Iterate(F&& iterateFunc) const {
    static_assert(std::is_same_v<R, MaybeError> || std::is_same_v<R, void>,
                  "R must be either void or MaybeError");
    constexpr bool mayError = std::is_same_v<R, MaybeError>;

    for (Aspect aspect : IterateEnumMask(mAspects)) {
        uint32_t aspectIndex = GetAspectIndex(aspect);

        // Fastest path, call iterateFunc on the whole aspect at once.
        if (mAspectCompressed[aspectIndex]) {
            SubresourceRange range =
                SubresourceRange::MakeFull(aspect, mArrayLayerCount, mMipLevelCount);
            if constexpr (mayError) {
                DAWN_TRY(iterateFunc(range, DataInline(aspectIndex)));
            } else {
                iterateFunc(range, DataInline(aspectIndex));
            }
            continue;
        }

        for (uint32_t layer = 0; layer < mArrayLayerCount; layer++) {
            // Fast path, call iterateFunc on the whole array layer at once.
            if (LayerCompressed(aspectIndex, layer)) {
                SubresourceRange range = GetFullLayerRange(aspect, layer);
                if constexpr (mayError) {
                    DAWN_TRY(iterateFunc(range, Data(aspectIndex, layer)));
                } else {
                    iterateFunc(range, Data(aspectIndex, layer));
                }
                continue;
            }

            // Slow path, call iterateFunc for each mip level.
            for (uint32_t level = 0; level < mMipLevelCount; level++) {
                SubresourceRange range = SubresourceRange::MakeSingle(aspect, layer, level);
                if constexpr (mayError) {
                    DAWN_TRY(iterateFunc(range, Data(aspectIndex, layer, level)));
                } else {
                    iterateFunc(range, Data(aspectIndex, layer, level));
                }
            }
        }
    }
    if constexpr (mayError) {
        return {};
    }
}

template <typename T>
const T& SubresourceStorage<T>::Get(Aspect aspect, uint32_t arrayLayer, uint32_t mipLevel) const {
    uint32_t aspectIndex = GetAspectIndex(aspect);
    DAWN_ASSERT(aspectIndex < GetAspectCount(mAspects));
    DAWN_ASSERT(arrayLayer < mArrayLayerCount);
    DAWN_ASSERT(mipLevel < mMipLevelCount);

    // Fastest path, the aspect is compressed!
    if (mAspectCompressed[aspectIndex]) {
        return DataInline(aspectIndex);
    }

    // Fast path, the array layer is compressed.
    if (LayerCompressed(aspectIndex, arrayLayer)) {
        return Data(aspectIndex, arrayLayer);
    }

    return Data(aspectIndex, arrayLayer, mipLevel);
}

template <typename T>
Aspect SubresourceStorage<T>::GetAspectsForTesting() const {
    return mAspects;
}

template <typename T>
uint32_t SubresourceStorage<T>::GetArrayLayerCountForTesting() const {
    return mArrayLayerCount;
}

template <typename T>
uint32_t SubresourceStorage<T>::GetMipLevelCountForTesting() const {
    return mMipLevelCount;
}

template <typename T>
bool SubresourceStorage<T>::IsAspectCompressedForTesting(Aspect aspect) const {
    return mAspectCompressed[GetAspectIndex(aspect)];
}

template <typename T>
bool SubresourceStorage<T>::IsLayerCompressedForTesting(Aspect aspect, uint32_t layer) const {
    return mAspectCompressed[GetAspectIndex(aspect)] ||
           mLayerCompressed[GetAspectIndex(aspect) * mArrayLayerCount + layer];
}

template <typename T>
void SubresourceStorage<T>::DecompressAspect(uint32_t aspectIndex) {
    DAWN_ASSERT(mAspectCompressed[aspectIndex]);
    const T& aspectData = DataInline(aspectIndex);
    mAspectCompressed[aspectIndex] = false;

    // Extra allocations are only needed when aspects are decompressed. Create them lazily.
    if (mData == nullptr) {
        DAWN_ASSERT(mLayerCompressed == nullptr);

        uint32_t aspectCount = GetAspectCount(mAspects);
        mLayerCompressed = std::make_unique<bool[]>(aspectCount * mArrayLayerCount);
        mData = std::make_unique<T[]>(aspectCount * mArrayLayerCount * mMipLevelCount);

        for (uint32_t layerIndex = 0; layerIndex < aspectCount * mArrayLayerCount; layerIndex++) {
            mLayerCompressed[layerIndex] = true;
        }
    }

    DAWN_ASSERT(LayerCompressed(aspectIndex, 0));
    for (uint32_t layer = 0; layer < mArrayLayerCount; layer++) {
        Data(aspectIndex, layer) = aspectData;
        DAWN_ASSERT(LayerCompressed(aspectIndex, layer));
    }
}

template <typename T>
void SubresourceStorage<T>::RecompressAspect(uint32_t aspectIndex) {
    DAWN_ASSERT(!mAspectCompressed[aspectIndex]);
    // All layers of the aspect must be compressed for the aspect to possibly recompress.
    for (uint32_t layer = 0; layer < mArrayLayerCount; layer++) {
        if (!LayerCompressed(aspectIndex, layer)) {
            return;
        }
    }

    T layer0Data = Data(aspectIndex, 0);
    for (uint32_t layer = 1; layer < mArrayLayerCount; layer++) {
        if (!(Data(aspectIndex, layer) == layer0Data)) {
            return;
        }
    }

    mAspectCompressed[aspectIndex] = true;
    DataInline(aspectIndex) = layer0Data;
}

template <typename T>
void SubresourceStorage<T>::DecompressLayer(uint32_t aspectIndex, uint32_t layer) {
    DAWN_ASSERT(LayerCompressed(aspectIndex, layer));
    DAWN_ASSERT(!mAspectCompressed[aspectIndex]);
    const T& layerData = Data(aspectIndex, layer);
    LayerCompressed(aspectIndex, layer) = false;

    // We assume that (aspect, layer, 0) is stored at the same place as (aspect, layer) which
    // allows starting the iteration at level 1.
    for (uint32_t level = 1; level < mMipLevelCount; level++) {
        Data(aspectIndex, layer, level) = layerData;
    }
}

template <typename T>
void SubresourceStorage<T>::RecompressLayer(uint32_t aspectIndex, uint32_t layer) {
    DAWN_ASSERT(!LayerCompressed(aspectIndex, layer));
    DAWN_ASSERT(!mAspectCompressed[aspectIndex]);
    const T& level0Data = Data(aspectIndex, layer, 0);

    for (uint32_t level = 1; level < mMipLevelCount; level++) {
        if (!(Data(aspectIndex, layer, level) == level0Data)) {
            return;
        }
    }

    LayerCompressed(aspectIndex, layer) = true;
}

template <typename T>
SubresourceRange SubresourceStorage<T>::GetFullLayerRange(Aspect aspect, uint32_t layer) const {
    return {aspect, {layer, 1}, {0, mMipLevelCount}};
}

template <typename T>
bool& SubresourceStorage<T>::LayerCompressed(uint32_t aspectIndex, uint32_t layer) {
    DAWN_ASSERT(!mAspectCompressed[aspectIndex]);
    return mLayerCompressed[aspectIndex * mArrayLayerCount + layer];
}

template <typename T>
bool SubresourceStorage<T>::LayerCompressed(uint32_t aspectIndex, uint32_t layer) const {
    DAWN_ASSERT(!mAspectCompressed[aspectIndex]);
    return mLayerCompressed[aspectIndex * mArrayLayerCount + layer];
}

template <typename T>
T& SubresourceStorage<T>::DataInline(uint32_t aspectIndex) {
    DAWN_ASSERT(mAspectCompressed[aspectIndex]);
    return mInlineAspectData[aspectIndex];
}
template <typename T>
T& SubresourceStorage<T>::Data(uint32_t aspectIndex, uint32_t layer, uint32_t level) {
    DAWN_ASSERT(level == 0 || !LayerCompressed(aspectIndex, layer));
    DAWN_ASSERT(!mAspectCompressed[aspectIndex]);
    return mData[(aspectIndex * mArrayLayerCount + layer) * mMipLevelCount + level];
}
template <typename T>
const T& SubresourceStorage<T>::DataInline(uint32_t aspectIndex) const {
    DAWN_ASSERT(mAspectCompressed[aspectIndex]);
    return mInlineAspectData[aspectIndex];
}
template <typename T>
const T& SubresourceStorage<T>::Data(uint32_t aspectIndex, uint32_t layer, uint32_t level) const {
    DAWN_ASSERT(level == 0 || !LayerCompressed(aspectIndex, layer));
    DAWN_ASSERT(!mAspectCompressed[aspectIndex]);
    return mData[(aspectIndex * mArrayLayerCount + layer) * mMipLevelCount + level];
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SUBRESOURCESTORAGE_H_
