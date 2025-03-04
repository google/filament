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

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "dawn/common/Log.h"
#include "dawn/native/SubresourceStorage.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dawn::native {

using ::testing::HasSubstr;

// A fake class that replicates the behavior of SubresourceStorage but without any compression
// and is used to compare the results of operations on SubresourceStorage against the "ground
// truth" of FakeStorage.
template <typename T>
struct FakeStorage {
    FakeStorage(Aspect aspects,
                uint32_t arrayLayerCount,
                uint32_t mipLevelCount,
                const T& initialValue = {})
        : mAspects(aspects),
          mArrayLayerCount(arrayLayerCount),
          mMipLevelCount(mipLevelCount),
          mData(GetAspectCount(aspects) * arrayLayerCount * mipLevelCount, initialValue) {}

    void Fill(const T& value) { std::fill(mData.begin(), mData.end(), value); }

    template <typename F>
    void Update(const SubresourceRange& range, F&& updateFunc) {
        for (Aspect aspect : IterateEnumMask(range.aspects)) {
            for (uint32_t layer = range.baseArrayLayer;
                 layer < range.baseArrayLayer + range.layerCount; layer++) {
                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; level++) {
                    SubresourceRange updateRange =
                        SubresourceRange::MakeSingle(aspect, layer, level);
                    updateFunc(updateRange, &mData[GetDataIndex(aspect, layer, level)]);
                }
            }
        }
    }

    template <typename U, typename F>
    void Merge(const SubresourceStorage<U>& other, F&& mergeFunc) {
        for (Aspect aspect : IterateEnumMask(mAspects)) {
            for (uint32_t layer = 0; layer < mArrayLayerCount; layer++) {
                for (uint32_t level = 0; level < mMipLevelCount; level++) {
                    SubresourceRange range = SubresourceRange::MakeSingle(aspect, layer, level);
                    mergeFunc(range, &mData[GetDataIndex(aspect, layer, level)],
                              other.Get(aspect, layer, level));
                }
            }
        }
    }

    const T& Get(Aspect aspect, uint32_t arrayLayer, uint32_t mipLevel) const {
        return mData[GetDataIndex(aspect, arrayLayer, mipLevel)];
    }

    size_t GetDataIndex(Aspect aspect, uint32_t layer, uint32_t level) const {
        uint32_t aspectIndex = GetAspectIndex(aspect);
        return level + mMipLevelCount * (layer + mArrayLayerCount * aspectIndex);
    }

    // Method that checks that this and real have exactly the same content. It does so via
    // looping on all subresources and calling Get() (hence testing Get()). It also calls
    // Iterate() checking that every subresource is mentioned exactly once and that its content
    // is correct (hence testing Iterate()). Its implementation requires the RangeTracker below
    // that itself needs FakeStorage<int> so it cannot be define inline with the other methods.
    void CheckSameAs(const SubresourceStorage<T>& real);

    Aspect mAspects;
    uint32_t mArrayLayerCount;
    uint32_t mMipLevelCount;

    std::vector<T> mData;
};

// Track a set of ranges that have been seen and can assert that in aggregate they make exactly
// a single range (and that each subresource was seen only once).
struct RangeTracker {
    template <typename T>
    explicit RangeTracker(const SubresourceStorage<T>& s)
        : mTracked(s.GetAspectsForTesting(),
                   s.GetArrayLayerCountForTesting(),
                   s.GetMipLevelCountForTesting(),
                   0) {}

    void Track(const SubresourceRange& range) {
        // Add +1 to the subresources tracked.
        mTracked.Update(range, [](const SubresourceRange&, uint32_t* counter) {
            ASSERT_EQ(*counter, 0u);
            *counter += 1;
        });
    }

    void CheckTrackedExactly(const SubresourceRange& range) {
        // Check that all subresources in the range were tracked once and set the counter back
        // to 0.
        mTracked.Update(range, [](const SubresourceRange&, uint32_t* counter) {
            ASSERT_EQ(*counter, 1u);
            *counter = 0;
        });

        // Now all subresources should be at 0.
        for (int counter : mTracked.mData) {
            ASSERT_EQ(counter, 0);
        }
    }

    FakeStorage<uint32_t> mTracked;
};

template <typename T>
void FakeStorage<T>::CheckSameAs(const SubresourceStorage<T>& real) {
    EXPECT_EQ(real.GetAspectsForTesting(), mAspects);
    EXPECT_EQ(real.GetArrayLayerCountForTesting(), mArrayLayerCount);
    EXPECT_EQ(real.GetMipLevelCountForTesting(), mMipLevelCount);

    RangeTracker tracker(real);
    real.Iterate([&](const SubresourceRange& range, const T& data) {
        // Check that the range is sensical.
        EXPECT_TRUE(IsSubset(range.aspects, mAspects));

        EXPECT_LT(range.baseArrayLayer, mArrayLayerCount);
        EXPECT_LE(range.baseArrayLayer + range.layerCount, mArrayLayerCount);

        EXPECT_LT(range.baseMipLevel, mMipLevelCount);
        EXPECT_LE(range.baseMipLevel + range.levelCount, mMipLevelCount);

        for (Aspect aspect : IterateEnumMask(range.aspects)) {
            for (uint32_t layer = range.baseArrayLayer;
                 layer < range.baseArrayLayer + range.layerCount; layer++) {
                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; level++) {
                    EXPECT_EQ(data, Get(aspect, layer, level));
                    EXPECT_EQ(data, real.Get(aspect, layer, level));
                }
            }
        }

        tracker.Track(range);
    });

    tracker.CheckTrackedExactly(
        SubresourceRange::MakeFull(mAspects, mArrayLayerCount, mMipLevelCount));
}

template <typename T>
void CheckAspectCompressed(const SubresourceStorage<T>& s, Aspect aspect, bool expected) {
    DAWN_ASSERT(HasOneBit(aspect));

    uint32_t levelCount = s.GetMipLevelCountForTesting();
    uint32_t layerCount = s.GetArrayLayerCountForTesting();

    bool seen = false;
    s.Iterate([&](const SubresourceRange& range, const T&) {
        if (range.aspects == aspect && range.layerCount == layerCount &&
            range.levelCount == levelCount && range.baseArrayLayer == 0 &&
            range.baseMipLevel == 0) {
            seen = true;
        }
    });

    ASSERT_EQ(seen, expected);

    // Check that the internal state of SubresourceStorage matches what we expect.
    // If an aspect is compressed, all its layers should be internally tagged as compressed.
    ASSERT_EQ(s.IsAspectCompressedForTesting(aspect), expected);
    if (expected) {
        for (uint32_t layer = 0; layer < s.GetArrayLayerCountForTesting(); layer++) {
            ASSERT_TRUE(s.IsLayerCompressedForTesting(aspect, layer));
        }
    }
}

template <typename T>
void CheckLayerCompressed(const SubresourceStorage<T>& s,
                          Aspect aspect,
                          uint32_t layer,
                          bool expected) {
    DAWN_ASSERT(HasOneBit(aspect));

    uint32_t levelCount = s.GetMipLevelCountForTesting();

    bool seen = false;
    s.Iterate([&](const SubresourceRange& range, const T&) {
        if (range.aspects == aspect && range.layerCount == 1 && range.levelCount == levelCount &&
            range.baseArrayLayer == layer && range.baseMipLevel == 0) {
            seen = true;
        }
    });

    ASSERT_EQ(seen, expected);
    ASSERT_EQ(s.IsLayerCompressedForTesting(aspect, layer), expected);
}

struct SmallData {
    uint32_t value = 0xF00;
};

bool operator==(const SmallData& a, const SmallData& b) {
    return a.value == b.value;
}

// Tests that the MaybeError version of Iterate returns the first error that it encounters.
TEST(SubresourceStorageTest, IterateMaybeError) {
    // Create a resource with multiple layers of different data so that we can ensure that the
    // iterate function runs more than once.
    constexpr uint32_t kLayers = 4;
    SubresourceStorage<uint32_t> s(Aspect::Color, kLayers, 1);
    for (uint32_t layer = 0; layer < kLayers; layer++) {
        s.Update(SubresourceRange::MakeSingle(Aspect::Color, layer, 0),
                 [&](const SubresourceRange&, uint32_t* data) { *data = layer + 1; });
    }

    // Make sure that the first error is returned.
    uint32_t errorLayer = 0;
    MaybeError maybeError =
        s.Iterate([&](const SubresourceRange& range, const uint32_t& layer) -> MaybeError {
            if (!errorLayer) {
                errorLayer = layer;
            }
            return DAWN_VALIDATION_ERROR("Errored at layer: %d", layer);
        });
    ASSERT_TRUE(maybeError.IsError());
    std::unique_ptr<ErrorData> error = maybeError.AcquireError();
    EXPECT_THAT(error->GetFormattedMessage(), HasSubstr(std::to_string(errorLayer)));
}

// Test that the default value is correctly set.
TEST(SubresourceStorageTest, DefaultValue) {
    // Test setting no default value for a primitive type.
    {
        SubresourceStorage<int> s(Aspect::Color, 3, 5);
        EXPECT_EQ(s.Get(Aspect::Color, 1, 2), 0);

        FakeStorage<int> f(Aspect::Color, 3, 5);
        f.CheckSameAs(s);
    }

    // Test setting a default value for a primitive type.
    {
        SubresourceStorage<int> s(Aspect::Color, 3, 5, 42);
        EXPECT_EQ(s.Get(Aspect::Color, 1, 2), 42);

        FakeStorage<int> f(Aspect::Color, 3, 5, 42);
        f.CheckSameAs(s);
    }

    // Test setting no default value for a type with a default constructor.
    {
        SubresourceStorage<SmallData> s(Aspect::Color, 3, 5);
        EXPECT_EQ(s.Get(Aspect::Color, 1, 2).value, 0xF00u);

        FakeStorage<SmallData> f(Aspect::Color, 3, 5);
        f.CheckSameAs(s);
    }
    // Test setting a default value for a type with a default constructor.
    {
        SubresourceStorage<SmallData> s(Aspect::Color, 3, 5, {007u});
        EXPECT_EQ(s.Get(Aspect::Color, 1, 2).value, 007u);

        FakeStorage<SmallData> f(Aspect::Color, 3, 5, {007u});
        f.CheckSameAs(s);
    }
}

// The tests for Update() all follow the same pattern of setting up a real and a fake storage
// then performing one or multiple Update()s on them and checking:
//  - They have the same content.
//  - The Update() range was correct.
//  - The aspects and layers have the expected "compressed" status.

// Calls Update both on the read storage and the fake storage but intercepts the call to
// updateFunc done by the real storage to check their ranges argument aggregate to exactly the
// update range.
template <typename T, typename F>
void CallUpdateOnBoth(SubresourceStorage<T>* s,
                      FakeStorage<T>* f,
                      const SubresourceRange& range,
                      F&& updateFunc) {
    RangeTracker tracker(*s);

    s->Update(range, [&](const SubresourceRange& range, T* data) {
        tracker.Track(range);
        updateFunc(range, data);
    });
    f->Update(range, updateFunc);

    tracker.CheckTrackedExactly(range);
    f->CheckSameAs(*s);
}

// Test updating a single subresource on a single-aspect storage.
TEST(SubresourceStorageTest, SingleSubresourceUpdateSingleAspect) {
    SubresourceStorage<int> s(Aspect::Color, 5, 7);
    FakeStorage<int> f(Aspect::Color, 5, 7);

    // Update a single subresource.
    SubresourceRange range = SubresourceRange::MakeSingle(Aspect::Color, 3, 2);
    CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data += 1; });

    CheckAspectCompressed(s, Aspect::Color, false);
    CheckLayerCompressed(s, Aspect::Color, 2, true);
    CheckLayerCompressed(s, Aspect::Color, 3, false);
    CheckLayerCompressed(s, Aspect::Color, 4, true);
}

// Test updating a single subresource on a multi-aspect storage.
TEST(SubresourceStorageTest, SingleSubresourceUpdateMultiAspect) {
    SubresourceStorage<int> s(Aspect::Depth | Aspect::Stencil, 5, 3);
    FakeStorage<int> f(Aspect::Depth | Aspect::Stencil, 5, 3);

    SubresourceRange range = SubresourceRange::MakeSingle(Aspect::Stencil, 1, 2);
    CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data += 1; });

    CheckAspectCompressed(s, Aspect::Depth, true);
    CheckAspectCompressed(s, Aspect::Stencil, false);
    CheckLayerCompressed(s, Aspect::Stencil, 0, true);
    CheckLayerCompressed(s, Aspect::Stencil, 1, false);
    CheckLayerCompressed(s, Aspect::Stencil, 2, true);
}

// Test updating as a stipple pattern on one of two aspects then updating it completely.
TEST(SubresourceStorageTest, UpdateStipple) {
    const uint32_t kLayers = 10;
    const uint32_t kLevels = 7;
    SubresourceStorage<int> s(Aspect::Depth | Aspect::Stencil, kLayers, kLevels);
    FakeStorage<int> f(Aspect::Depth | Aspect::Stencil, kLayers, kLevels);

    // Update with a stipple.
    for (uint32_t layer = 0; layer < kLayers; layer++) {
        for (uint32_t level = 0; level < kLevels; level++) {
            if ((layer + level) % 2 == 0) {
                SubresourceRange range = SubresourceRange::MakeSingle(Aspect::Depth, layer, level);
                CallUpdateOnBoth(&s, &f, range,
                                 [](const SubresourceRange&, int* data) { *data += 17; });
            }
        }
    }

    // The depth should be fully uncompressed while the stencil stayed compressed.
    CheckAspectCompressed(s, Aspect::Stencil, true);
    CheckAspectCompressed(s, Aspect::Depth, false);
    for (uint32_t layer = 0; layer < kLayers; layer++) {
        CheckLayerCompressed(s, Aspect::Depth, layer, false);
    }

    // Update completely with a single value. Recompression should happen!
    {
        SubresourceRange fullRange =
            SubresourceRange::MakeFull(Aspect::Depth | Aspect::Stencil, kLayers, kLevels);
        CallUpdateOnBoth(&s, &f, fullRange, [](const SubresourceRange&, int* data) { *data = 31; });
    }

    CheckAspectCompressed(s, Aspect::Depth, true);
    CheckAspectCompressed(s, Aspect::Stencil, true);
}

// Test updating as a crossing band pattern:
//  - The first band is full layers [2, 3] on both aspects
//  - The second band is full mips [5, 6] on one aspect.
// Then updating completely.
TEST(SubresourceStorageTest, UpdateTwoBand) {
    const uint32_t kLayers = 5;
    const uint32_t kLevels = 9;
    SubresourceStorage<int> s(Aspect::Depth | Aspect::Stencil, kLayers, kLevels);
    FakeStorage<int> f(Aspect::Depth | Aspect::Stencil, kLayers, kLevels);

    // Update the two bands
    {
        SubresourceRange range(Aspect::Depth | Aspect::Stencil, {2, 2}, {0, kLevels});
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data += 3; });
    }

    // The layers were fully updated so they should stay compressed.
    CheckLayerCompressed(s, Aspect::Depth, 2, true);
    CheckLayerCompressed(s, Aspect::Depth, 3, true);
    CheckLayerCompressed(s, Aspect::Stencil, 2, true);
    CheckLayerCompressed(s, Aspect::Stencil, 3, true);

    {
        SubresourceRange range(Aspect::Depth, {0, kLayers}, {5, 2});
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data *= 3; });
    }

    // The layers had to be decompressed in depth
    CheckLayerCompressed(s, Aspect::Depth, 2, false);
    CheckLayerCompressed(s, Aspect::Depth, 3, false);
    CheckLayerCompressed(s, Aspect::Stencil, 2, true);
    CheckLayerCompressed(s, Aspect::Stencil, 3, true);

    // Update completely. Without a single value recompression shouldn't happen.
    {
        SubresourceRange fullRange =
            SubresourceRange::MakeFull(Aspect::Depth | Aspect::Stencil, kLayers, kLevels);
        CallUpdateOnBoth(&s, &f, fullRange,
                         [](const SubresourceRange&, int* data) { *data += 12; });
    }

    CheckAspectCompressed(s, Aspect::Depth, false);
    CheckAspectCompressed(s, Aspect::Stencil, false);
}

// Test updating with extremal subresources
//    - Then half of the array layers in full.
//    - Then updating completely.
TEST(SubresourceStorageTest, UpdateExtremas) {
    const uint32_t kLayers = 6;
    const uint32_t kLevels = 4;
    SubresourceStorage<int> s(Aspect::Color, kLayers, kLevels);
    FakeStorage<int> f(Aspect::Color, kLayers, kLevels);

    // Update the two extrema
    {
        SubresourceRange range = SubresourceRange::MakeSingle(Aspect::Color, 0, kLevels - 1);
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data += 3; });
    }
    {
        SubresourceRange range = SubresourceRange::MakeSingle(Aspect::Color, kLayers - 1, 0);
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data *= 3; });
    }

    CheckLayerCompressed(s, Aspect::Color, 0, false);
    CheckLayerCompressed(s, Aspect::Color, 1, true);
    CheckLayerCompressed(s, Aspect::Color, kLayers - 2, true);
    CheckLayerCompressed(s, Aspect::Color, kLayers - 1, false);

    // Update half of the layers in full with constant values. Some recompression should happen.
    {
        SubresourceRange range(Aspect::Color, {0, kLayers / 2}, {0, kLevels});
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data = 123; });
    }

    CheckLayerCompressed(s, Aspect::Color, 0, true);
    CheckLayerCompressed(s, Aspect::Color, 1, true);
    CheckLayerCompressed(s, Aspect::Color, kLayers - 1, false);

    // Update completely. Recompression should happen!
    {
        SubresourceRange fullRange = SubresourceRange::MakeFull(Aspect::Color, kLayers, kLevels);
        CallUpdateOnBoth(&s, &f, fullRange, [](const SubresourceRange&, int* data) { *data = 35; });
    }

    CheckAspectCompressed(s, Aspect::Color, true);
}

// A regression test for an issue found while reworking the implementation where
// RecompressAspect didn't correctly check that each each layer was compressed but only that
// their 0th value was the same.
TEST(SubresourceStorageTest, UpdateLevel0sHappenToMatch) {
    SubresourceStorage<int> s(Aspect::Color, 2, 2);
    FakeStorage<int> f(Aspect::Color, 2, 2);

    // Update 0th mip levels to some value, it should decompress the aspect and both layers.
    {
        SubresourceRange range(Aspect::Color, {0, 2}, {0, 1});
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data = 17; });
    }

    CheckAspectCompressed(s, Aspect::Color, false);
    CheckLayerCompressed(s, Aspect::Color, 0, false);
    CheckLayerCompressed(s, Aspect::Color, 1, false);

    // Update the whole resource by doing +1. The aspects and layers should stay decompressed.
    {
        SubresourceRange range = SubresourceRange::MakeFull(Aspect::Color, 2, 2);
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data += 1; });
    }

    CheckAspectCompressed(s, Aspect::Color, false);
    CheckLayerCompressed(s, Aspect::Color, 0, false);
    CheckLayerCompressed(s, Aspect::Color, 1, false);
}

// The tests for Merge() all follow the same as the Update() tests except that they use Update()
// to set up the test storages.

// Similar to CallUpdateOnBoth but for Merge
template <typename T, typename U, typename F>
void CallMergeOnBoth(SubresourceStorage<T>* s,
                     FakeStorage<T>* f,
                     const SubresourceStorage<U>& other,
                     F&& mergeFunc) {
    RangeTracker tracker(*s);

    s->Merge(other, [&](const SubresourceRange& range, T* data, const U& otherData) {
        tracker.Track(range);
        mergeFunc(range, data, otherData);
    });
    f->Merge(other, mergeFunc);

    tracker.CheckTrackedExactly(
        SubresourceRange::MakeFull(f->mAspects, f->mArrayLayerCount, f->mMipLevelCount));
    f->CheckSameAs(*s);
}

// Test merging two fully compressed single-aspect resources.
TEST(SubresourceStorageTest, MergeFullWithFullSingleAspect) {
    SubresourceStorage<int> s(Aspect::Color, 4, 6);
    FakeStorage<int> f(Aspect::Color, 4, 6);

    // Merge the whole resource in a single call.
    SubresourceStorage<bool> other(Aspect::Color, 4, 6, true);
    CallMergeOnBoth(&s, &f, other, [](const SubresourceRange&, int* data, bool other) {
        if (other) {
            *data = 13;
        }
    });

    CheckAspectCompressed(s, Aspect::Color, true);
}

// Test merging two fully compressed multi-aspect resources.
TEST(SubresourceStorageTest, MergeFullWithFullMultiAspect) {
    SubresourceStorage<int> s(Aspect::Depth | Aspect::Stencil, 6, 7);
    FakeStorage<int> f(Aspect::Depth | Aspect::Stencil, 6, 7);

    // Merge the whole resource in a single call.
    SubresourceStorage<bool> other(Aspect::Depth | Aspect::Stencil, 6, 7, true);
    CallMergeOnBoth(&s, &f, other, [](const SubresourceRange&, int* data, bool other) {
        if (other) {
            *data = 13;
        }
    });

    CheckAspectCompressed(s, Aspect::Depth, true);
    CheckAspectCompressed(s, Aspect::Stencil, true);
}

// Test merging a fully compressed resource in a resource with the "cross band" pattern.
//  - The first band is full layers [2, 3] on both aspects
//  - The second band is full mips [5, 6] on one aspect.
// This provides coverage of using a single piece of data from `other` to update all of `s`
TEST(SubresourceStorageTest, MergeFullInTwoBand) {
    const uint32_t kLayers = 5;
    const uint32_t kLevels = 9;
    SubresourceStorage<int> s(Aspect::Depth | Aspect::Stencil, kLayers, kLevels);
    FakeStorage<int> f(Aspect::Depth | Aspect::Stencil, kLayers, kLevels);

    // Update the two bands
    {
        SubresourceRange range(Aspect::Depth | Aspect::Stencil, {2, 2}, {0, kLevels});
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data += 3; });
    }
    {
        SubresourceRange range(Aspect::Depth, {0, kLayers}, {5, 2});
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data += 5; });
    }

    // Merge the fully compressed resource.
    SubresourceStorage<int> other(Aspect::Depth | Aspect::Stencil, kLayers, kLevels, 17);
    CallMergeOnBoth(&s, &f, other,
                    [](const SubresourceRange&, int* data, int other) { *data += other; });

    // The layers traversed by the mip band are still uncompressed.
    CheckLayerCompressed(s, Aspect::Depth, 1, false);
    CheckLayerCompressed(s, Aspect::Depth, 2, false);
    CheckLayerCompressed(s, Aspect::Depth, 3, false);
    CheckLayerCompressed(s, Aspect::Depth, 4, false);

    // Stencil is decompressed but all its layers are still compressed because there wasn't the
    // mip band.
    CheckAspectCompressed(s, Aspect::Stencil, false);
    CheckLayerCompressed(s, Aspect::Stencil, 1, true);
    CheckLayerCompressed(s, Aspect::Stencil, 2, true);
    CheckLayerCompressed(s, Aspect::Stencil, 3, true);
    CheckLayerCompressed(s, Aspect::Stencil, 4, true);
}
// Test the reverse, mergign two-bands in a full resource. This provides coverage for
// decompressing aspects / and partilly layers to match the compression of `other`
TEST(SubresourceStorageTest, MergeTwoBandInFull) {
    const uint32_t kLayers = 5;
    const uint32_t kLevels = 9;
    SubresourceStorage<int> s(Aspect::Depth | Aspect::Stencil, kLayers, kLevels, 75);
    FakeStorage<int> f(Aspect::Depth | Aspect::Stencil, kLayers, kLevels, 75);

    // Update the two bands
    SubresourceStorage<int> other(Aspect::Depth | Aspect::Stencil, kLayers, kLevels);
    {
        SubresourceRange range(Aspect::Depth | Aspect::Stencil, {2, 2}, {0, kLevels});
        other.Update(range, [](const SubresourceRange&, int* data) { *data += 3; });
    }
    {
        SubresourceRange range(Aspect::Depth, {0, kLayers}, {5, 2});
        other.Update(range, [](const SubresourceRange&, int* data) { *data += 5; });
    }

    // Merge the fully compressed resource.
    CallMergeOnBoth(&s, &f, other,
                    [](const SubresourceRange&, int* data, int other) { *data += other; });

    // The layers traversed by the mip band are still uncompressed.
    CheckLayerCompressed(s, Aspect::Depth, 1, false);
    CheckLayerCompressed(s, Aspect::Depth, 2, false);
    CheckLayerCompressed(s, Aspect::Depth, 3, false);
    CheckLayerCompressed(s, Aspect::Depth, 4, false);

    // Stencil is decompressed but all its layers are still compressed because there wasn't the
    // mip band.
    CheckAspectCompressed(s, Aspect::Stencil, false);
    CheckLayerCompressed(s, Aspect::Stencil, 1, true);
    CheckLayerCompressed(s, Aspect::Stencil, 2, true);
    CheckLayerCompressed(s, Aspect::Stencil, 3, true);
    CheckLayerCompressed(s, Aspect::Stencil, 4, true);
}

// Test merging storage with a layer band in a stipple patterned storage. This provide coverage
// for the code path that uses the same layer data for other multiple times.
TEST(SubresourceStorageTest, MergeLayerBandInStipple) {
    const uint32_t kLayers = 3;
    const uint32_t kLevels = 5;

    SubresourceStorage<int> s(Aspect::Color, kLayers, kLevels);
    FakeStorage<int> f(Aspect::Color, kLayers, kLevels);
    SubresourceStorage<int> other(Aspect::Color, kLayers, kLevels);

    for (uint32_t layer = 0; layer < kLayers; layer++) {
        for (uint32_t level = 0; level < kLevels; level++) {
            if ((layer + level) % 2 == 0) {
                SubresourceRange range = SubresourceRange::MakeSingle(Aspect::Color, layer, level);
                CallUpdateOnBoth(&s, &f, range,
                                 [](const SubresourceRange&, int* data) { *data += 17; });
            }
        }
        if (layer % 2 == 0) {
            other.Update({Aspect::Color, {layer, 1}, {0, kLevels}},
                         [](const SubresourceRange&, int* data) { *data += 8; });
        }
    }

    // Merge the band in the stipple.
    CallMergeOnBoth(&s, &f, other,
                    [](const SubresourceRange&, int* data, int other) { *data += other; });

    // None of the resulting layers are compressed.
    CheckLayerCompressed(s, Aspect::Color, 0, false);
    CheckLayerCompressed(s, Aspect::Color, 1, false);
    CheckLayerCompressed(s, Aspect::Color, 2, false);
}

// Regression test for a missing check that layer 0 is compressed when recompressing.
TEST(SubresourceStorageTest, Layer0NotCompressedBlocksAspectRecompression) {
    const uint32_t kLayers = 2;
    const uint32_t kLevels = 2;
    SubresourceStorage<int> s(Aspect::Color, kLayers, kLevels);
    FakeStorage<int> f(Aspect::Color, kLayers, kLevels);

    // Set up s with zeros except (0, 1) which is garbage.
    {
        SubresourceRange range = SubresourceRange::MakeSingle(Aspect::Color, 0, 1);
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data += 0xABC; });
    }

    // Other is 2x2 of zeroes
    SubresourceStorage<int> other(Aspect::Color, kLayers, kLevels);

    // Fake updating F with other which is fully compressed and will trigger recompression.
    CallMergeOnBoth(&s, &f, other, [](const SubresourceRange&, int*, int) {});

    // The Color aspect should not have been recompressed.
    CheckAspectCompressed(s, Aspect::Color, false);
    CheckLayerCompressed(s, Aspect::Color, 0, false);
}

// Regression test for aspect decompression not copying to layer 0
TEST(SubresourceStorageTest, AspectDecompressionUpdatesLayer0) {
    const uint32_t kLayers = 2;
    const uint32_t kLevels = 2;
    SubresourceStorage<int> s(Aspect::Color, kLayers, kLevels, 3);
    FakeStorage<int> f(Aspect::Color, kLayers, kLevels, 3);

    // Cause decompression by writing to a single subresource.
    {
        SubresourceRange range = SubresourceRange::MakeSingle(Aspect::Color, 1, 1);
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data += 0xABC; });
    }

    // Check that the aspect's value of 3 was correctly decompressed in layer 0.
    CheckLayerCompressed(s, Aspect::Color, 0, true);
    EXPECT_EQ(3, s.Get(Aspect::Color, 0, 0));
    EXPECT_EQ(3, s.Get(Aspect::Color, 0, 1));
}

// Check that fill after creation overwrites whatever was passed as initial value.
TEST(SubresourceStorageTest, FillAfterInitialization) {
    const uint32_t kLayers = 2;
    const uint32_t kLevels = 2;
    SubresourceStorage<int> s(Aspect::Color, kLayers, kLevels, 3);
    FakeStorage<int> f(Aspect::Color, kLayers, kLevels, 3);

    s.Fill(42);
    f.Fill(42);

    f.CheckSameAs(s);
    CheckAspectCompressed(s, Aspect::Color, true);
}

// Check that fill after some modification overwrites everything and recompresses.
TEST(SubresourceStorageTest, FillAfterModificationRecompresses) {
    const uint32_t kLayers = 2;
    const uint32_t kLevels = 2;
    SubresourceStorage<int> s(Aspect::Depth | Aspect::Stencil, kLayers, kLevels, 3);
    FakeStorage<int> f(Aspect::Depth | Aspect::Stencil, kLayers, kLevels, 3);

    // Cause decompression by writing to a single subresource.
    {
        SubresourceRange range = SubresourceRange::MakeSingle(Aspect::Stencil, 1, 1);
        CallUpdateOnBoth(&s, &f, range, [](const SubresourceRange&, int* data) { *data = 0xCAFE; });
    }
    CheckAspectCompressed(s, Aspect::Depth, true);
    CheckAspectCompressed(s, Aspect::Stencil, false);

    // Fill with 42, aspects should be recompressed entirely.
    s.Fill(42);
    f.Fill(42);

    f.CheckSameAs(s);
    CheckAspectCompressed(s, Aspect::Depth, true);
    CheckAspectCompressed(s, Aspect::Stencil, true);
}

// Bugs found while testing:
//  - mLayersCompressed not initialized to true.
//  - DecompressLayer setting Compressed to true instead of false.
//  - Get() checking for !compressed instead of compressed for the early exit.
//  - DAWN_ASSERT in RecompressLayers was inverted.
//  - Two != being converted to == during a rework.
//  - (with DAWN_ASSERT) that RecompressAspect didn't check that aspect 0 was compressed.
//  - Missing decompression of layer 0 after introducing mInlineAspectData.

}  // namespace dawn::native
