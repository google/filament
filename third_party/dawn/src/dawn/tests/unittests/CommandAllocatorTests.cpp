// Copyright 2017 The Dawn & Tint Authors
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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/40285824): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#include <array>
#include <limits>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/dawn/common/Enumerator.h"
#include "src/dawn/common/MemoryBlockAllocator.h"
#include "src/dawn/native/CommandAllocator.h"

namespace dawn::native {
namespace {

class CommandAllocatorAndPool : public CommandAllocator {
  public:
    CommandAllocatorAndPool() : CommandAllocator(&mBlockAllocator) {}

  private:
    MemoryBlockAllocator mBlockAllocator;
};

}  // anonymous namespace

// Definition of the command types used in the tests
enum class CommandType : uint32_t {
    Draw,
    Pipeline,
    Immediate,
    Big,
    Small,
};

struct CommandDraw {
    uint32_t first;
    uint32_t count;
};

struct CommandPipeline {
    uint64_t pipeline;
    uint32_t attachmentPoint;
};

struct CommandImmediate {
    uint8_t size;
    uint8_t offset;
};

constexpr int kBigBufferSize = 65536;

struct CommandBig {
    std::array<uint32_t, kBigBufferSize> buffer;
};

struct CommandSmall {
    uint16_t data;
};

// Test allocating nothing works
TEST(CommandAllocator, DoNothingAllocator) {
    CommandAllocatorAndPool allocator;
}

// Test iterating over nothing works
TEST(CommandAllocator, DoNothingAllocatorWithIterator) {
    CommandAllocatorAndPool allocator;
    CommandIterator iterator(std::move(allocator));
    iterator.MakeEmptyAsDataWasDestroyed();
}

// Test basic usage of allocator + iterator
TEST(CommandAllocator, Basic) {
    CommandAllocatorAndPool allocator;

    uint64_t myPipeline = 0xDEADBEEFBEEFDEAD;
    uint32_t myAttachmentPoint = 2;
    uint32_t myFirst = 42;
    uint32_t myCount = 16;

    {
        CommandPipeline* pipeline = allocator.Allocate<CommandPipeline>(CommandType::Pipeline);
        pipeline->pipeline = myPipeline;
        pipeline->attachmentPoint = myAttachmentPoint;

        CommandDraw* draw = allocator.Allocate<CommandDraw>(CommandType::Draw);
        draw->first = myFirst;
        draw->count = myCount;
    }

    {
        CommandIterator iterator(std::move(allocator));
        CommandType type;

        bool hasNext = iterator.NextCommandId(&type);
        ASSERT_TRUE(hasNext);
        ASSERT_EQ(type, CommandType::Pipeline);

        CommandPipeline* pipeline = iterator.NextCommand<CommandPipeline>();
        ASSERT_EQ(pipeline->pipeline, myPipeline);
        ASSERT_EQ(pipeline->attachmentPoint, myAttachmentPoint);

        hasNext = iterator.NextCommandId(&type);
        ASSERT_TRUE(hasNext);
        ASSERT_EQ(type, CommandType::Draw);

        CommandDraw* draw = iterator.NextCommand<CommandDraw>();
        ASSERT_EQ(draw->first, myFirst);
        ASSERT_EQ(draw->count, myCount);

        hasNext = iterator.NextCommandId(&type);
        ASSERT_FALSE(hasNext);

        iterator.MakeEmptyAsDataWasDestroyed();
    }
}

// Test basic usage of allocator + iterator with data
TEST(CommandAllocator, BasicWithData) {
    CommandAllocatorAndPool allocator;

    uint8_t mySize = 8;
    uint8_t myOffset = 3;
    std::array<uint32_t, 5> myValues = {6, 42, 0xFFFFFFFF, 0, 54};

    {
        CommandImmediate* immediates = allocator.Allocate<CommandImmediate>(CommandType::Immediate);
        immediates->size = mySize;
        immediates->offset = myOffset;

        Span<uint32_t> values = allocator.AllocateData<uint32_t>(size_t{5});
        for (size_t i = 0; i < 5; i++) {
            values[i] = myValues[i];
        }
    }

    {
        CommandIterator iterator(std::move(allocator));
        CommandType type;

        bool hasNext = iterator.NextCommandId(&type);
        ASSERT_TRUE(hasNext);
        ASSERT_EQ(type, CommandType::Immediate);

        CommandImmediate* immediates = iterator.NextCommand<CommandImmediate>();
        ASSERT_EQ(immediates->size, mySize);
        ASSERT_EQ(immediates->offset, myOffset);

        Span<const uint32_t> values = iterator.NextData<uint32_t>(size_t{5});
        ASSERT_EQ(values.size(), 5u);
        for (auto [i, value] : Enumerate(values)) {
            ASSERT_EQ(value, myValues[i]);
        }

        hasNext = iterator.NextCommandId(&type);
        ASSERT_FALSE(hasNext);

        iterator.MakeEmptyAsDataWasDestroyed();
    }
}

// Test basic iterating several times
TEST(CommandAllocator, MultipleIterations) {
    CommandAllocatorAndPool allocator;

    uint32_t myFirst = 42;
    uint32_t myCount = 16;

    {
        CommandDraw* draw = allocator.Allocate<CommandDraw>(CommandType::Draw);
        draw->first = myFirst;
        draw->count = myCount;
    }

    {
        CommandIterator iterator(std::move(allocator));
        CommandType type;

        // First iteration
        bool hasNext = iterator.NextCommandId(&type);
        ASSERT_TRUE(hasNext);
        ASSERT_EQ(type, CommandType::Draw);

        CommandDraw* draw = iterator.NextCommand<CommandDraw>();
        ASSERT_EQ(draw->first, myFirst);
        ASSERT_EQ(draw->count, myCount);

        hasNext = iterator.NextCommandId(&type);
        ASSERT_FALSE(hasNext);

        // Second iteration
        hasNext = iterator.NextCommandId(&type);
        ASSERT_TRUE(hasNext);
        ASSERT_EQ(type, CommandType::Draw);

        draw = iterator.NextCommand<CommandDraw>();
        ASSERT_EQ(draw->first, myFirst);
        ASSERT_EQ(draw->count, myCount);

        hasNext = iterator.NextCommandId(&type);
        ASSERT_FALSE(hasNext);

        iterator.MakeEmptyAsDataWasDestroyed();
    }
}
// Test large commands work
TEST(CommandAllocator, LargeCommands) {
    CommandAllocatorAndPool allocator;

    const int kCommandCount = 5;

    uint32_t count = 0;
    for (int i = 0; i < kCommandCount; i++) {
        CommandBig* big = allocator.Allocate<CommandBig>(CommandType::Big);
        for (int j = 0; j < kBigBufferSize; j++) {
            big->buffer[j] = count++;
        }
    }

    CommandIterator iterator(std::move(allocator));
    CommandType type;
    count = 0;
    int numCommands = 0;
    while (iterator.NextCommandId(&type)) {
        ASSERT_EQ(type, CommandType::Big);

        CommandBig* big = iterator.NextCommand<CommandBig>();
        for (int i = 0; i < kBigBufferSize; i++) {
            ASSERT_EQ(big->buffer[i], count);
            count++;
        }
        numCommands++;
    }
    ASSERT_EQ(numCommands, kCommandCount);

    iterator.MakeEmptyAsDataWasDestroyed();
}

// Test many small commands work
TEST(CommandAllocator, ManySmallCommands) {
    CommandAllocatorAndPool allocator;

    // Stay under max representable uint16_t
    const int kCommandCount = 50000;

    uint16_t count = 0;
    for (int i = 0; i < kCommandCount; i++) {
        CommandSmall* small = allocator.Allocate<CommandSmall>(CommandType::Small);
        small->data = count++;
    }

    CommandIterator iterator(std::move(allocator));
    CommandType type;
    count = 0;
    int numCommands = 0;
    while (iterator.NextCommandId(&type)) {
        ASSERT_EQ(type, CommandType::Small);

        CommandSmall* small = iterator.NextCommand<CommandSmall>();
        ASSERT_EQ(small->data, count);
        count++;
        numCommands++;
    }
    ASSERT_EQ(numCommands, kCommandCount);

    iterator.MakeEmptyAsDataWasDestroyed();
}

/*        ________
 *       /        \
 *       | POUIC! |
 *       \_ ______/
 *         v
 *    ()_()
 *    (O.o)
 *    (> <)o
 */

// Test usage of iterator.Reset
TEST(CommandAllocator, IteratorReset) {
    CommandAllocatorAndPool allocator;

    uint64_t myPipeline = 0xDEADBEEFBEEFDEAD;
    uint32_t myAttachmentPoint = 2;
    uint32_t myFirst = 42;
    uint32_t myCount = 16;

    {
        CommandPipeline* pipeline = allocator.Allocate<CommandPipeline>(CommandType::Pipeline);
        pipeline->pipeline = myPipeline;
        pipeline->attachmentPoint = myAttachmentPoint;

        CommandDraw* draw = allocator.Allocate<CommandDraw>(CommandType::Draw);
        draw->first = myFirst;
        draw->count = myCount;
    }

    {
        CommandIterator iterator(std::move(allocator));
        CommandType type;

        bool hasNext = iterator.NextCommandId(&type);
        ASSERT_TRUE(hasNext);
        ASSERT_EQ(type, CommandType::Pipeline);

        CommandPipeline* pipeline = iterator.NextCommand<CommandPipeline>();
        ASSERT_EQ(pipeline->pipeline, myPipeline);
        ASSERT_EQ(pipeline->attachmentPoint, myAttachmentPoint);

        iterator.Reset();

        hasNext = iterator.NextCommandId(&type);
        ASSERT_TRUE(hasNext);
        ASSERT_EQ(type, CommandType::Pipeline);

        pipeline = iterator.NextCommand<CommandPipeline>();
        ASSERT_EQ(pipeline->pipeline, myPipeline);
        ASSERT_EQ(pipeline->attachmentPoint, myAttachmentPoint);

        hasNext = iterator.NextCommandId(&type);
        ASSERT_TRUE(hasNext);
        ASSERT_EQ(type, CommandType::Draw);

        CommandDraw* draw = iterator.NextCommand<CommandDraw>();
        ASSERT_EQ(draw->first, myFirst);
        ASSERT_EQ(draw->count, myCount);

        hasNext = iterator.NextCommandId(&type);
        ASSERT_FALSE(hasNext);

        iterator.MakeEmptyAsDataWasDestroyed();
    }
}

// Test iterating empty iterators
TEST(CommandAllocator, EmptyIterator) {
    {
        CommandAllocatorAndPool allocator;
        CommandIterator iterator(std::move(allocator));

        CommandType type;
        bool hasNext = iterator.NextCommandId(&type);
        ASSERT_FALSE(hasNext);

        iterator.MakeEmptyAsDataWasDestroyed();
    }
    {
        CommandAllocatorAndPool allocator;
        CommandIterator iterator1(std::move(allocator));
        CommandIterator iterator2(std::move(iterator1));

        CommandType type;
        bool hasNext = iterator2.NextCommandId(&type);
        ASSERT_FALSE(hasNext);

        iterator2.MakeEmptyAsDataWasDestroyed();
    }
    {
        CommandIterator iterator1;
        CommandIterator iterator2(std::move(iterator1));

        CommandType type;
        bool hasNext = iterator2.NextCommandId(&type);
        ASSERT_FALSE(hasNext);

        iterator2.MakeEmptyAsDataWasDestroyed();
    }
}

template <size_t A>
struct alignas(A) AlignedStruct {
    char placeholder;
};

// Test for overflows in Allocate's computations, size 1 variant
TEST(CommandAllocatorDeathTest, AllocationOverflow_1) {
    CommandAllocatorAndPool allocator;
    EXPECT_DEATH_IF_SUPPORTED(
        allocator.AllocateData<AlignedStruct<1>>(std::numeric_limits<size_t>::max() / 1u), "");
}

// Test for overflows in Allocate's computations, size 2 variant
TEST(CommandAllocatorDeathTest, AllocationOverflow_2) {
    CommandAllocatorAndPool allocator;
    EXPECT_DEATH_IF_SUPPORTED(
        allocator.AllocateData<AlignedStruct<2>>(std::numeric_limits<size_t>::max() / 2u), "");
}

// Test for overflows in Allocate's computations, size 4 variant
TEST(CommandAllocatorDeathTest, AllocationOverflow_4) {
    CommandAllocatorAndPool allocator;
    EXPECT_DEATH_IF_SUPPORTED(
        allocator.AllocateData<AlignedStruct<4>>(std::numeric_limits<size_t>::max() / 4u), "");
}

// Test for overflows in Allocate's computations, size 8 variant
TEST(CommandAllocatorDeathTest, AllocationOverflow_8) {
    CommandAllocatorAndPool allocator;
    EXPECT_DEATH_IF_SUPPORTED(
        allocator.AllocateData<AlignedStruct<8>>(std::numeric_limits<size_t>::max() / 8u), "");
}

template <int DefaultValue>
struct IntWithDefault {
    IntWithDefault() : value(DefaultValue) {}

    int value;
};

// Test that the allcator correctly defaults initalizes data for Allocate
TEST(CommandAllocator, AllocateDefaultInitializes) {
    CommandAllocatorAndPool allocator;

    IntWithDefault<42>* int42 = allocator.Allocate<IntWithDefault<42>>(CommandType::Draw);
    ASSERT_EQ(int42->value, 42);

    IntWithDefault<43>* int43 = allocator.Allocate<IntWithDefault<43>>(CommandType::Draw);
    ASSERT_EQ(int43->value, 43);

    IntWithDefault<44>* int44 = allocator.Allocate<IntWithDefault<44>>(CommandType::Draw);
    ASSERT_EQ(int44->value, 44);

    CommandIterator iterator(std::move(allocator));
    iterator.MakeEmptyAsDataWasDestroyed();
}

// Test that the allocator correctly default-initalizes data for AllocateData
TEST(CommandAllocator, AllocateDataDefaultInitializes) {
    CommandAllocatorAndPool allocator;

    Span<IntWithDefault<33>> int33 = allocator.AllocateData<IntWithDefault<33>>(size_t{1});
    ASSERT_EQ(int33.size(), size_t{1});
    ASSERT_EQ(int33[0].value, 33);

    Span<IntWithDefault<34>> int34 = allocator.AllocateData<IntWithDefault<34>>(size_t{2});
    ASSERT_EQ(int34.size(), size_t{2});
    ASSERT_EQ(int34[0].value, 34);
    ASSERT_EQ(int34[0].value, 34);

    Span<IntWithDefault<35>> int35 = allocator.AllocateData<IntWithDefault<35>>(size_t{3});
    ASSERT_EQ(int35.size(), size_t{3});
    ASSERT_EQ(int35[0].value, 35);
    ASSERT_EQ(int35[1].value, 35);
    ASSERT_EQ(int35[2].value, 35);

    CommandIterator iterator(std::move(allocator));
    iterator.MakeEmptyAsDataWasDestroyed();
}

// Tests flattening of multiple CommandAllocators into a single CommandIterator using
// AcquireCommandBlocks.
TEST(CommandAllocator, AcquireCommandBlocks) {
    constexpr size_t kNumAllocators = 2;
    constexpr size_t kNumCommandsPerAllocator = 2;
    const std::array<std::array<const uint64_t, kNumCommandsPerAllocator>, kNumAllocators>
        pipelines = {{
            {0xDEADBEEFBEEFDEAD, 0xC0FFEEF00DC0FFEE},
            {0x1337C0DE1337C0DE, 0xCAFEFACEFACECAFE},
        }};
    const std::array<std::array<const uint32_t, kNumCommandsPerAllocator>, kNumAllocators>
        attachmentPoints = {{{1, 2}, {3, 4}}};
    const std::array<std::array<const uint32_t, kNumCommandsPerAllocator>, kNumAllocators> firsts =
        {{{42, 43}, {5, 6}}};
    const std::array<std::array<const uint32_t, kNumCommandsPerAllocator>, kNumAllocators> counts =
        {{{16, 32}, {4, 8}}};

    MemoryBlockAllocator pool;
    std::vector<CommandAllocator> allocators;
    allocators.reserve(kNumAllocators);
    for (size_t j = 0; j < kNumAllocators; j++) {
        allocators.emplace_back(&pool);
    }
    for (size_t j = 0; j < kNumAllocators; ++j) {
        CommandAllocator& allocator = allocators[j];
        for (size_t i = 0; i < kNumCommandsPerAllocator; ++i) {
            CommandPipeline* pipeline = allocator.Allocate<CommandPipeline>(CommandType::Pipeline);
            pipeline->pipeline = pipelines[j][i];
            pipeline->attachmentPoint = attachmentPoints[j][i];

            CommandDraw* draw = allocator.Allocate<CommandDraw>(CommandType::Draw);
            draw->first = firsts[j][i];
            draw->count = counts[j][i];
        }
    }

    CommandIterator iterator;
    iterator.AcquireCommandBlocks(std::move(allocators));
    for (size_t j = 0; j < kNumAllocators; ++j) {
        for (size_t i = 0; i < kNumCommandsPerAllocator; ++i) {
            CommandType type;
            bool hasNext = iterator.NextCommandId(&type);
            ASSERT_TRUE(hasNext);
            ASSERT_EQ(type, CommandType::Pipeline);

            CommandPipeline* pipeline = iterator.NextCommand<CommandPipeline>();
            ASSERT_EQ(pipeline->pipeline, pipelines[j][i]);
            ASSERT_EQ(pipeline->attachmentPoint, attachmentPoints[j][i]);

            hasNext = iterator.NextCommandId(&type);
            ASSERT_TRUE(hasNext);
            ASSERT_EQ(type, CommandType::Draw);

            CommandDraw* draw = iterator.NextCommand<CommandDraw>();
            ASSERT_EQ(draw->first, firsts[j][i]);
            ASSERT_EQ(draw->count, counts[j][i]);
        }
    }
    CommandType type;
    ASSERT_FALSE(iterator.NextCommandId(&type));
    iterator.MakeEmptyAsDataWasDestroyed();
}

}  // namespace dawn::native
