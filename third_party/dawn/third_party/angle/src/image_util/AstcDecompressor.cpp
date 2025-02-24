//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// AstcDecompressorImpl.cpp: Decodes ASTC-encoded textures.

#include <array>
#include <future>
#include <unordered_map>

#include "astcenc.h"
#include "common/SimpleMutex.h"
#include "common/WorkerThread.h"
#include "image_util/AstcDecompressor.h"

namespace angle
{
namespace
{

const astcenc_swizzle kSwizzle = {ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A};

// Used by std::unique_ptr to release the context when the pointer is destroyed
struct AstcencContextDeleter
{
    void operator()(astcenc_context *c) { astcenc_context_free(c); }
};

using AstcencContextUniquePtr = std::unique_ptr<astcenc_context, AstcencContextDeleter>;

// Returns the max number of threads to use when using multithreaded decompression
uint32_t MaxThreads()
{
    static const uint32_t numThreads = std::min(16u, std::thread::hardware_concurrency());
    return numThreads;
}

// Creates a new astcenc_context and wraps it in a smart pointer.
// It is not needed to call astcenc_context_free() on the returned pointer.
// blockWith, blockSize: ASTC block size for the context
// Error: (output param) Where to put the error status. Must not be null.
// Returns nullptr in case of error.
AstcencContextUniquePtr MakeDecoderContext(uint32_t blockWidth,
                                           uint32_t blockHeight,
                                           astcenc_error *error)
{
    astcenc_config config = {};
    *error =
        // TODO(gregschlom): Do we need a special case for sRGB images? (And pass
        //                   ASTCENC_PRF_LDR_SRGB here?)
        astcenc_config_init(ASTCENC_PRF_LDR, blockWidth, blockHeight, 1, ASTCENC_PRE_FASTEST,
                            ASTCENC_FLG_DECOMPRESS_ONLY, &config);
    if (*error != ASTCENC_SUCCESS)
    {
        return nullptr;
    }

    astcenc_context *context;
    *error = astcenc_context_alloc(&config, MaxThreads(), &context);
    if (*error != ASTCENC_SUCCESS)
    {
        return nullptr;
    }
    return AstcencContextUniquePtr(context);
}

// Returns whether the ASTC decompressor can be used on this machine. It might not be available if
// the CPU doesn't support AVX2 instructions for example. Since this call is a bit expensive and
// never changes, the result should be cached.
bool IsAstcDecompressorAvailable()
{
    astcenc_error error;
    // Try getting an arbitrary context. If it works, the decompressor is available.
    AstcencContextUniquePtr context = MakeDecoderContext(5, 5, &error);
    return context != nullptr;
}

// Caches and manages astcenc_context objects.
//
// Each context is fairly large (around 30 MB) and takes a while to construct, so it's important to
// reuse them as much as possible.
//
// While context objects can be reused across multiple threads, they must be used sequentially. To
// avoid having to lock and manage access between threads, we keep one cache per thread. This avoids
// any concurrency issues, at the cost of extra memory.
//
// Currently, there is no eviction strategy. Each cache could grow to a maximum of ~400 MB in size
// since they are 13 possible ASTC block sizes.
//
// Thread-safety: not thread safe.
class AstcDecompressorContextCache
{
  public:
    // Returns a context object for a given ASTC block size, along with the error code if the
    // context initialization failed.
    // In this case, the context will be null, and the status code will be non-zero.
    std::pair<astcenc_context *, astcenc_error> get(uint32_t blockWidth, uint32_t blockHeight)
    {
        Value &value = mContexts[{blockWidth, blockHeight}];
        if (value.context == nullptr)
        {
            value.context = MakeDecoderContext(blockWidth, blockHeight, &value.error);
        }
        return {value.context.get(), value.error};
    }

  private:
    // Holds the data we use as the cache key
    struct Key
    {
        uint32_t blockWidth;
        uint32_t blockHeight;

        bool operator==(const Key &other) const
        {
            return blockWidth == other.blockWidth && blockHeight == other.blockHeight;
        }
    };

    struct Value
    {
        AstcencContextUniquePtr context = nullptr;
        astcenc_error error             = ASTCENC_SUCCESS;
    };

    // Computes the hash of a Key
    struct KeyHash
    {
        std::size_t operator()(const Key &k) const
        {
            // blockWidth and blockHeight are < 256 (actually, < 16), so this is safe
            return k.blockWidth << 8 | k.blockHeight;
        }
    };

    std::unordered_map<Key, Value, KeyHash> mContexts;
};

struct DecompressTask : public Closure
{
    DecompressTask(astcenc_context *context,
                   uint32_t threadIndex,
                   const uint8_t *data,
                   size_t dataLength,
                   astcenc_image *image)
        : context(context),
          threadIndex(threadIndex),
          data(data),
          dataLength(dataLength),
          image(image)
    {}

    void operator()() override
    {
        result = astcenc_decompress_image(context, data, dataLength, image, &kSwizzle, threadIndex);
    }

    astcenc_context *context;
    uint32_t threadIndex;
    const uint8_t *data;
    size_t dataLength;
    astcenc_image *image;
    astcenc_error result;
};

// Performs ASTC decompression of an image on the CPU
class AstcDecompressorImpl : public AstcDecompressor
{
  public:
    AstcDecompressorImpl()
        : AstcDecompressor(), mContextCache(std::make_unique<AstcDecompressorContextCache>())
    {
        mTasks.reserve(MaxThreads());
        mWaitEvents.reserve(MaxThreads());
    }

    ~AstcDecompressorImpl() override = default;

    bool available() const override
    {
        static bool available = IsAstcDecompressorAvailable();
        return available;
    }

    int32_t decompress(std::shared_ptr<WorkerThreadPool> singleThreadPool,
                       std::shared_ptr<WorkerThreadPool> multiThreadPool,
                       const uint32_t imgWidth,
                       const uint32_t imgHeight,
                       const uint32_t blockWidth,
                       const uint32_t blockHeight,
                       const uint8_t *input,
                       size_t inputLength,
                       uint8_t *output) override
    {
        // A given astcenc context can only decompress one image at a time, which we why we keep
        // this mutex locked the whole time.
        std::lock_guard global_lock(mMutex);

        auto [context, context_status] = mContextCache->get(blockWidth, blockHeight);
        if (context_status != ASTCENC_SUCCESS)
            return context_status;

        astcenc_image image;
        image.dim_x     = imgWidth;
        image.dim_y     = imgHeight;
        image.dim_z     = 1;
        image.data_type = ASTCENC_TYPE_U8;
        image.data      = reinterpret_cast<void **>(&output);

        // For smaller images the overhead of multithreading exceeds the benefits.
        const bool singleThreaded = (imgHeight <= 32 && imgWidth <= 32) || !multiThreadPool;

        std::shared_ptr<WorkerThreadPool> &threadPool =
            singleThreaded ? singleThreadPool : multiThreadPool;
        const uint32_t threadCount = singleThreaded ? 1 : MaxThreads();

        mTasks.clear();
        mWaitEvents.clear();

        for (uint32_t i = 0; i < threadCount; ++i)
        {
            mTasks.push_back(
                std::make_shared<DecompressTask>(context, i, input, inputLength, &image));
            mWaitEvents.push_back(threadPool->postWorkerTask(mTasks[i]));
        }
        WaitableEvent::WaitMany(&mWaitEvents);
        astcenc_decompress_reset(context);

        for (auto &task : mTasks)
        {
            if (task->result != ASTCENC_SUCCESS)
                return task->result;
        }
        return ASTCENC_SUCCESS;
    }

    const char *getStatusString(int32_t statusCode) const override
    {
        const char *msg = astcenc_get_error_string((astcenc_error)statusCode);
        return msg ? msg : "ASTCENC_UNKNOWN_STATUS";
    }

  private:
    std::unique_ptr<AstcDecompressorContextCache> mContextCache;
    angle::SimpleMutex mMutex;  // Locked while calling `decode()`
    std::vector<std::shared_ptr<DecompressTask>> mTasks;
    std::vector<std::shared_ptr<WaitableEvent>> mWaitEvents;
};

}  // namespace

AstcDecompressor &AstcDecompressor::get()
{
    static auto *instance = new AstcDecompressorImpl();
    return *instance;
}

}  // namespace angle
