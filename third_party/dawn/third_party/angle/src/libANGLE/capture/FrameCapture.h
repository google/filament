//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FrameCapture.h:
//   ANGLE Frame capture interface.
//

#ifndef LIBANGLE_FRAME_CAPTURE_H_
#define LIBANGLE_FRAME_CAPTURE_H_

#include "common/PackedEnums.h"
#include "common/SimpleMutex.h"
#include "common/frame_capture_utils.h"
#include "common/system_utils.h"
#include "libANGLE/Context.h"
#include "libANGLE/ShareGroup.h"
#include "libANGLE/Thread.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/entry_points_utils.h"

namespace gl
{
enum class BigGLEnum;
enum class GLESEnum;
}  // namespace gl

namespace angle
{
// Helper to use unique IDs for each local data variable.
class DataCounters final : angle::NonCopyable
{
  public:
    DataCounters();
    ~DataCounters();

    int getAndIncrement(EntryPoint entryPoint, const std::string &paramName);

  private:
    // <CallName, ParamName>
    using Counter = std::pair<EntryPoint, std::string>;
    std::map<Counter, int> mData;
};

constexpr int kStringsNotFound = -1;
class StringCounters final : angle::NonCopyable
{
  public:
    StringCounters();
    ~StringCounters();

    int getStringCounter(const std::vector<std::string> &str);
    void setStringCounter(const std::vector<std::string> &str, int &counter);

  private:
    std::map<std::vector<std::string>, int> mStringCounterMap;
};

class DataTracker final : angle::NonCopyable
{
  public:
    DataTracker();
    ~DataTracker();

    DataCounters &getCounters() { return mCounters; }
    StringCounters &getStringCounters() { return mStringCounters; }

  private:
    DataCounters mCounters;
    StringCounters mStringCounters;
};

class ReplayWriter final : angle::NonCopyable
{
  public:
    ReplayWriter();
    ~ReplayWriter();

    void setSourceFileExtension(const char *ext);
    void setSourceFileSizeThreshold(size_t sourceFileSizeThreshold);
    void setFilenamePattern(const std::string &pattern);
    void setCaptureLabel(const std::string &label);
    void setSourcePrologue(const std::string &prologue);
    void setHeaderPrologue(const std::string &prologue);

    void addPublicFunction(const std::string &functionProto,
                           const std::stringstream &headerStream,
                           const std::stringstream &bodyStream);
    void addPrivateFunction(const std::string &functionProto,
                            const std::stringstream &headerStream,
                            const std::stringstream &bodyStream);
    std::string getInlineVariableName(EntryPoint entryPoint, const std::string &paramName);

    std::string getInlineStringSetVariableName(EntryPoint entryPoint,
                                               const std::string &paramName,
                                               const std::vector<std::string> &strings,
                                               bool *isNewEntryOut);

    void saveFrame();
    void saveFrameIfFull();
    void saveIndexFilesAndHeader();
    void saveSetupFile();

    std::vector<std::string> getAndResetWrittenFiles();

  private:
    static std::string GetVarName(EntryPoint entryPoint, const std::string &paramName, int counter);

    void saveHeader();
    void writeReplaySource(const std::string &filename);
    void addWrittenFile(const std::string &filename);
    size_t getStoredReplaySourceSize() const;

    std::string mSourceFileExtension;
    size_t mSourceFileSizeThreshold;
    size_t mFrameIndex;

    DataTracker mDataTracker;
    std::string mFilenamePattern;
    std::string mCaptureLabel;
    std::string mSourcePrologue;
    std::string mHeaderPrologue;

    std::vector<std::string> mReplayHeaders;
    std::vector<std::string> mGlobalVariableDeclarations;

    std::vector<std::string> mPublicFunctionPrototypes;
    std::vector<std::string> mPublicFunctions;

    std::vector<std::string> mPrivateFunctionPrototypes;
    std::vector<std::string> mPrivateFunctions;

    std::vector<std::string> mWrittenFiles;
};

using BufferCalls = std::map<GLuint, std::vector<CallCapture>>;

// true means mapped, false means unmapped
using BufferMapStatusMap = std::map<GLuint, bool>;

using FenceSyncSet   = std::set<gl::SyncID>;
using FenceSyncCalls = std::map<gl::SyncID, std::vector<CallCapture>>;

// For default uniforms, we need to track which ones are dirty, and the series of calls to reset.
// Each program has unique default uniforms, and each uniform has one or more locations in the
// default buffer. For reset efficiency, we track only the uniforms dirty by location, per program.

// A set of all default uniforms (per program) that were modified during the run
using DefaultUniformLocationsSet = std::set<gl::UniformLocation>;
using DefaultUniformLocationsPerProgramMap =
    std::map<gl::ShaderProgramID, DefaultUniformLocationsSet>;

// A map of programs which maps to locations and their reset calls
using DefaultUniformCallsPerLocationMap = std::map<gl::UniformLocation, std::vector<CallCapture>>;
using DefaultUniformCallsPerProgramMap =
    std::map<gl::ShaderProgramID, DefaultUniformCallsPerLocationMap>;

using DefaultUniformBaseLocationMap =
    std::map<std::pair<gl::ShaderProgramID, gl::UniformLocation>, gl::UniformLocation>;

using ResourceSet   = std::set<GLuint>;
using ResourceCalls = std::map<GLuint, std::vector<CallCapture>>;

class TrackedResource final : angle::NonCopyable
{
  public:
    TrackedResource();
    ~TrackedResource();

    const ResourceSet &getStartingResources() const { return mStartingResources; }
    ResourceSet &getStartingResources() { return mStartingResources; }
    const ResourceSet &getNewResources() const { return mNewResources; }
    ResourceSet &getNewResources() { return mNewResources; }
    const ResourceSet &getResourcesToDelete() const { return mResourcesToDelete; }
    ResourceSet &getResourcesToDelete() { return mResourcesToDelete; }
    const ResourceSet &getResourcesToRegen() const { return mResourcesToRegen; }
    ResourceSet &getResourcesToRegen() { return mResourcesToRegen; }
    const ResourceSet &getResourcesToRestore() const { return mResourcesToRestore; }
    ResourceSet &getResourcesToRestore() { return mResourcesToRestore; }

    void setGennedResource(GLuint id);
    void setDeletedResource(GLuint id);
    void setModifiedResource(GLuint id);
    bool resourceIsGenerated(GLuint id);

    ResourceCalls &getResourceRegenCalls() { return mResourceRegenCalls; }
    ResourceCalls &getResourceRestoreCalls() { return mResourceRestoreCalls; }

  private:
    // Resource regen calls will gen a resource
    ResourceCalls mResourceRegenCalls;
    // Resource restore calls will restore the contents of a resource
    ResourceCalls mResourceRestoreCalls;

    // Resources created during startup
    ResourceSet mStartingResources;

    // Resources created during the run that need to be deleted
    ResourceSet mNewResources;
    // Resources recreated during the run that need to be deleted
    ResourceSet mResourcesToDelete;
    // Resources deleted during the run that need to be recreated
    ResourceSet mResourcesToRegen;
    // Resources modified during the run that need to be restored
    ResourceSet mResourcesToRestore;
};

using TrackedResourceArray =
    std::array<TrackedResource, static_cast<uint32_t>(ResourceIDType::EnumCount)>;

enum class ShaderProgramType
{
    ShaderType,
    ProgramType
};

// Helper to track resource changes during the capture
class ResourceTracker final : angle::NonCopyable
{
  public:
    ResourceTracker();
    ~ResourceTracker();

    BufferCalls &getBufferMapCalls() { return mBufferMapCalls; }
    BufferCalls &getBufferUnmapCalls() { return mBufferUnmapCalls; }

    std::vector<CallCapture> &getBufferBindingCalls() { return mBufferBindingCalls; }

    void setBufferMapped(gl::ContextID contextID, GLuint id);
    void setBufferUnmapped(gl::ContextID contextID, GLuint id);

    bool getStartingBuffersMappedCurrent(GLuint id) const;
    bool getStartingBuffersMappedInitial(GLuint id) const;

    void setStartingBufferMapped(GLuint id, bool mapped)
    {
        // Track the current state (which will change throughout the trace)
        mStartingBuffersMappedCurrent[id] = mapped;

        // And the initial state, to compare during frame loop reset
        mStartingBuffersMappedInitial[id] = mapped;
    }

    void onShaderProgramAccess(gl::ShaderProgramID shaderProgramID);
    uint32_t getMaxShaderPrograms() const { return mMaxShaderPrograms; }

    FenceSyncSet &getStartingFenceSyncs() { return mStartingFenceSyncs; }
    FenceSyncCalls &getFenceSyncRegenCalls() { return mFenceSyncRegenCalls; }
    FenceSyncSet &getFenceSyncsToRegen() { return mFenceSyncsToRegen; }
    void setDeletedFenceSync(gl::SyncID sync);

    DefaultUniformLocationsPerProgramMap &getDefaultUniformsToReset()
    {
        return mDefaultUniformsToReset;
    }
    DefaultUniformCallsPerLocationMap &getDefaultUniformResetCalls(gl::ShaderProgramID id)
    {
        return mDefaultUniformResetCalls[id];
    }
    void setModifiedDefaultUniform(gl::ShaderProgramID programID, gl::UniformLocation location);
    void setDefaultUniformBaseLocation(gl::ShaderProgramID programID,
                                       gl::UniformLocation location,
                                       gl::UniformLocation baseLocation);
    gl::UniformLocation getDefaultUniformBaseLocation(gl::ShaderProgramID programID,
                                                      gl::UniformLocation location)
    {
        ASSERT(mDefaultUniformBaseLocations.find({programID, location}) !=
               mDefaultUniformBaseLocations.end());
        return mDefaultUniformBaseLocations[{programID, location}];
    }

    TrackedResource &getTrackedResource(gl::ContextID contextID, ResourceIDType type);

    void getContextIDs(std::set<gl::ContextID> &idsOut);

    std::map<EGLImage, egl::AttributeMap> &getImageToAttribTable() { return mMatchImageToAttribs; }

    std::map<GLuint, egl::ImageID> &getTextureIDToImageTable() { return mMatchTextureIDToImage; }

    void setShaderProgramType(gl::ShaderProgramID id, angle::ShaderProgramType type)
    {
        mShaderProgramType[id] = type;
    }
    ShaderProgramType getShaderProgramType(gl::ShaderProgramID id)
    {
        ASSERT(mShaderProgramType.find(id) != mShaderProgramType.end());
        return mShaderProgramType[id];
    }

  private:
    // Buffer map calls will map a buffer with correct offset, length, and access flags
    BufferCalls mBufferMapCalls;
    // Buffer unmap calls will bind and unmap a given buffer
    BufferCalls mBufferUnmapCalls;

    // Buffer binding calls to restore bindings recorded during MEC
    std::vector<CallCapture> mBufferBindingCalls;

    // Whether a given buffer was mapped at the start of the trace
    BufferMapStatusMap mStartingBuffersMappedInitial;
    // The status of buffer mapping throughout the trace, modified with each Map/Unmap call
    BufferMapStatusMap mStartingBuffersMappedCurrent;

    // Maximum accessed shader program ID.
    uint32_t mMaxShaderPrograms = 0;

    // Fence sync objects created during MEC setup
    FenceSyncSet mStartingFenceSyncs;
    // Fence sync regen calls will create a fence sync objects
    FenceSyncCalls mFenceSyncRegenCalls;
    // Fence syncs to regen are a list of starting fence sync objects that were deleted and need to
    // be regen'ed.
    FenceSyncSet mFenceSyncsToRegen;

    // Default uniforms that were modified during the run
    DefaultUniformLocationsPerProgramMap mDefaultUniformsToReset;
    // Calls per default uniform to return to original state
    DefaultUniformCallsPerProgramMap mDefaultUniformResetCalls;

    // Base location of arrayed uniforms
    DefaultUniformBaseLocationMap mDefaultUniformBaseLocations;

    // Tracked resources per context
    TrackedResourceArray mTrackedResourcesShared;
    std::map<gl::ContextID, TrackedResourceArray> mTrackedResourcesPerContext;

    std::map<EGLImage, egl::AttributeMap> mMatchImageToAttribs;
    std::map<GLuint, egl::ImageID> mMatchTextureIDToImage;

    std::map<gl::ShaderProgramID, ShaderProgramType> mShaderProgramType;
};

// Used by the CPP replay to filter out unnecessary code.
using HasResourceTypeMap = angle::PackedEnumBitSet<ResourceIDType>;

// Map of ResourceType to IDs and range of setup calls
using ResourceIDToSetupCallsMap =
    PackedEnumMap<ResourceIDType, std::map<GLuint, gl::Range<size_t>>>;

// Map of buffer ID to offset and size used when mapped
using BufferDataMap = std::map<gl::BufferID, std::pair<GLintptr, GLsizeiptr>>;

// A dictionary of sources indexed by shader type.
using ProgramSources = gl::ShaderMap<std::string>;

// Maps from IDs to sources.
using ShaderSourceMap  = std::map<gl::ShaderProgramID, std::string>;
using ProgramSourceMap = std::map<gl::ShaderProgramID, ProgramSources>;

// Map from textureID to level and data
using TextureLevels       = std::map<GLint, std::vector<uint8_t>>;
using TextureLevelDataMap = std::map<gl::TextureID, TextureLevels>;

struct SurfaceParams
{
    gl::Extents extents;
    egl::ColorSpace colorSpace;
};

// Map from ContextID to SurfaceParams
using SurfaceParamsMap = std::map<gl::ContextID, SurfaceParams>;

using CallVector = std::vector<std::vector<CallCapture> *>;

// A map from API entry point to calls
using CallResetMap = std::map<angle::EntryPoint, std::vector<CallCapture>>;

using TextureBinding  = std::pair<size_t, gl::TextureType>;
using TextureResetMap = std::map<TextureBinding, gl::TextureID>;

using BufferBindingPair = std::pair<gl::BufferBinding, gl::BufferID>;

// StateResetHelper provides a simple way to track whether an entry point has been called during the
// trace, along with the reset calls to get it back to starting state.  This is useful for things
// that are one dimensional, like context bindings or context state.
class StateResetHelper final : angle::NonCopyable
{
  public:
    StateResetHelper();
    ~StateResetHelper();

    const std::set<angle::EntryPoint> &getDirtyEntryPoints() const { return mDirtyEntryPoints; }
    void setEntryPointDirty(EntryPoint entryPoint) { mDirtyEntryPoints.insert(entryPoint); }

    CallResetMap &getResetCalls() { return mResetCalls; }
    const CallResetMap &getResetCalls() const { return mResetCalls; }

    void setDefaultResetCalls(const gl::Context *context, angle::EntryPoint);

    const std::set<TextureBinding> &getDirtyTextureBindings() const
    {
        return mDirtyTextureBindings;
    }
    void setTextureBindingDirty(size_t unit, gl::TextureType target)
    {
        mDirtyTextureBindings.emplace(unit, target);
    }

    TextureResetMap &getResetTextureBindings() { return mResetTextureBindings; }

    void setResetActiveTexture(size_t textureID) { mResetActiveTexture = textureID; }
    size_t getResetActiveTexture() { return mResetActiveTexture; }

    const std::set<gl::BufferBinding> &getDirtyBufferBindings() const
    {
        return mDirtyBufferBindings;
    }
    void setBufferBindingDirty(gl::BufferBinding binding) { mDirtyBufferBindings.insert(binding); }

    const std::set<BufferBindingPair> &getStartingBufferBindings() const
    {
        return mStartingBufferBindings;
    }
    void setStartingBufferBinding(gl::BufferBinding binding, gl::BufferID bufferID)
    {
        mStartingBufferBindings.insert({binding, bufferID});
    }

  private:
    // Dirty state per entry point
    std::set<angle::EntryPoint> mDirtyEntryPoints;

    // Reset calls per API entry point
    CallResetMap mResetCalls;

    // Dirty state per texture binding
    std::set<TextureBinding> mDirtyTextureBindings;

    // Texture bindings and active texture to restore
    TextureResetMap mResetTextureBindings;
    size_t mResetActiveTexture = 0;

    // Starting and dirty buffer bindings
    std::set<BufferBindingPair> mStartingBufferBindings;
    std::set<gl::BufferBinding> mDirtyBufferBindings;
};

class FrameCapture final : angle::NonCopyable
{
  public:
    FrameCapture();
    ~FrameCapture();

    std::vector<CallCapture> &getSetupCalls() { return mSetupCalls; }
    void clearSetupCalls() { mSetupCalls.clear(); }

    StateResetHelper &getStateResetHelper() { return mStateResetHelper; }

    void reset();

  private:
    std::vector<CallCapture> mSetupCalls;

    StateResetHelper mStateResetHelper;
};

// Page range inside a coherent buffer
struct PageRange
{
    PageRange(size_t start, size_t end);
    ~PageRange();

    // Relative start page
    size_t start;

    // First page after the relative end
    size_t end;
};

// Memory address range defined by start and size
struct AddressRange
{
    AddressRange();
    AddressRange(uintptr_t start, size_t size);
    ~AddressRange();

    uintptr_t end();

    uintptr_t start;
    size_t size;
};

// Used to handle protection of buffers that overlap in pages.
enum class PageSharingType
{
    NoneShared,
    FirstShared,
    LastShared,
    FirstAndLastShared
};

class CoherentBuffer
{
  public:
    CoherentBuffer(uintptr_t start, size_t size, size_t pageSize, bool useShadowMemory);
    ~CoherentBuffer();

    // Sets the a range in the buffer clean and protects a selected range
    void protectPageRange(const PageRange &pageRange);

    // Sets all pages to clean and enables protection
    void protectAll();

    // Sets a page dirty state and sets it's protection
    void setDirty(size_t relativePage, bool dirty);

    // Shadow memory synchronization
    void updateBufferMemory();
    void updateShadowMemory();

    // Removes protection
    void removeProtection(PageSharingType sharingType);

    bool contains(size_t page, size_t *relativePage);
    bool isDirty();

    // Returns dirty page ranges
    std::vector<PageRange> getDirtyPageRanges();

    // Calculates address range from page range
    AddressRange getDirtyAddressRange(const PageRange &dirtyPageRange);
    AddressRange getRange();

    void markShadowDirty() { mShadowDirty = true; }
    bool isShadowDirty() { return mShadowDirty; }

  private:
    // Actual buffer start and size
    AddressRange mRange;

    // Start and size of page aligned protected area
    AddressRange mProtectionRange;

    // Start and end of protection in relative pages, calculated from mProtectionRange.
    size_t mProtectionStartPage;
    size_t mProtectionEndPage;

    size_t mPageCount;
    size_t mPageSize;

    // Clean pages are protected
    std::vector<bool> mDirtyPages;

    // shadow memory releated fields
    bool mShadowMemoryEnabled;
    uintptr_t mBufferStart;
    void *mShadowMemory;
    bool mShadowDirty;
};

class CoherentBufferTracker final : angle::NonCopyable
{
  public:
    CoherentBufferTracker();
    ~CoherentBufferTracker();

    bool isDirty(gl::BufferID id);
    uintptr_t addBuffer(gl::BufferID id, uintptr_t start, size_t size);
    void removeBuffer(gl::BufferID id);
    void disable();
    void enable();
    void onEndFrame();
    bool haveBuffer(gl::BufferID id);
    bool isShadowMemoryEnabled() { return mShadowMemoryEnabled; }
    void enableShadowMemory() { mShadowMemoryEnabled = true; }
    void maybeUpdateShadowMemory();
    void markAllShadowDirty();
    // Determine whether memory protection can be used directly on graphics memory
    bool canProtectDirectly(gl::Context *context);

  private:
    // Detect overlapping pages when removing protection
    PageSharingType doesBufferSharePage(gl::BufferID id);

    // Returns a map to found buffers and the corresponding pages for a given address.
    // For addresses that are in a page shared by 2 buffers, 2 results are returned.
    HashMap<std::shared_ptr<CoherentBuffer>, size_t> getBufferPagesForAddress(uintptr_t address);
    PageFaultHandlerRangeType handleWrite(uintptr_t address);

  public:
    angle::SimpleMutex mMutex;
    HashMap<GLuint, std::shared_ptr<CoherentBuffer>> mBuffers;

  private:
    bool mEnabled;
    std::unique_ptr<PageFaultHandler> mPageFaultHandler;
    size_t mPageSize;

    bool mShadowMemoryEnabled;
};

// Shared class for any items that need to be tracked by FrameCapture across shared contexts
class FrameCaptureShared final : angle::NonCopyable
{
  public:
    FrameCaptureShared();
    ~FrameCaptureShared();

    void captureCall(gl::Context *context, CallCapture &&call, bool isCallValid);
    void checkForCaptureTrigger();
    void onEndFrame(gl::Context *context);
    void onDestroyContext(const gl::Context *context);
    void onMakeCurrent(const gl::Context *context, const egl::Surface *drawSurface);
    bool enabled() const { return mEnabled; }

    bool isCapturing() const;
    uint32_t getFrameCount() const;

    // Returns a frame index starting from "1" as the first frame.
    uint32_t getReplayFrameIndex() const;

    void trackBufferMapping(const gl::Context *context,
                            CallCapture *call,
                            gl::BufferID id,
                            gl::Buffer *buffer,
                            GLintptr offset,
                            GLsizeiptr length,
                            bool writable,
                            bool coherent);

    void trackTextureUpdate(const gl::Context *context, const CallCapture &call);
    void trackImageUpdate(const gl::Context *context, const CallCapture &call);
    void trackDefaultUniformUpdate(const gl::Context *context, const CallCapture &call);
    void trackVertexArrayUpdate(const gl::Context *context, const CallCapture &call);

    const std::string &getShaderSource(gl::ShaderProgramID id) const;
    void setShaderSource(gl::ShaderProgramID id, std::string sources);

    const ProgramSources &getProgramSources(gl::ShaderProgramID id) const;
    void setProgramSources(gl::ShaderProgramID id, ProgramSources sources);

    // Load data from a previously stored texture level
    const std::vector<uint8_t> &retrieveCachedTextureLevel(gl::TextureID id,
                                                           gl::TextureTarget target,
                                                           GLint level);

    // Create new texture level data and copy the source into it
    void copyCachedTextureLevel(const gl::Context *context,
                                gl::TextureID srcID,
                                GLint srcLevel,
                                gl::TextureID dstID,
                                GLint dstLevel,
                                const CallCapture &call);

    // Create the location that should be used to cache texture level data
    std::vector<uint8_t> &getCachedTextureLevelData(gl::Texture *texture,
                                                    gl::TextureTarget target,
                                                    GLint level,
                                                    EntryPoint entryPoint);

    // Capture coherent buffer storages
    void captureCoherentBufferSnapshot(const gl::Context *context, gl::BufferID bufferID);

    // Remove any cached texture levels on deletion
    void deleteCachedTextureLevelData(gl::TextureID id);

    void eraseBufferDataMapEntry(const gl::BufferID bufferId)
    {
        const auto &bufferDataInfo = mBufferDataMap.find(bufferId);
        if (bufferDataInfo != mBufferDataMap.end())
        {
            mBufferDataMap.erase(bufferDataInfo);
        }
    }

    bool hasBufferData(gl::BufferID bufferID)
    {
        const auto &bufferDataInfo = mBufferDataMap.find(bufferID);
        if (bufferDataInfo != mBufferDataMap.end())
        {
            return true;
        }
        return false;
    }

    std::pair<GLintptr, GLsizeiptr> getBufferDataOffsetAndLength(gl::BufferID bufferID)
    {
        const auto &bufferDataInfo = mBufferDataMap.find(bufferID);
        ASSERT(bufferDataInfo != mBufferDataMap.end());
        return bufferDataInfo->second;
    }

    void setCaptureActive() { mCaptureActive = true; }
    void setCaptureInactive() { mCaptureActive = false; }
    bool isCaptureActive() { return mCaptureActive; }
    bool usesMidExecutionCapture() { return mCaptureStartFrame > 1; }

    gl::ContextID getWindowSurfaceContextID() const { return mWindowSurfaceContextID; }

    void markResourceSetupCallsInactive(std::vector<CallCapture> *setupCalls,
                                        ResourceIDType type,
                                        GLuint id,
                                        gl::Range<size_t> range);

    void updateReadBufferSize(size_t readBufferSize)
    {
        mReadBufferSize = std::max(mReadBufferSize, readBufferSize);
    }

    template <typename ResourceType>
    void handleGennedResource(const gl::Context *context, ResourceType resourceID)
    {
        if (isCaptureActive())
        {
            ResourceIDType idType    = GetResourceIDTypeFromType<ResourceType>::IDType;
            TrackedResource &tracker = mResourceTracker.getTrackedResource(context->id(), idType);
            tracker.setGennedResource(resourceID.value);
        }
    }

    template <typename ResourceType>
    bool resourceIsGenerated(const gl::Context *context, ResourceType resourceID)
    {
        ResourceIDType idType    = GetResourceIDTypeFromType<ResourceType>::IDType;
        TrackedResource &tracker = mResourceTracker.getTrackedResource(context->id(), idType);
        return tracker.resourceIsGenerated(resourceID.value);
    }

    template <typename ResourceType>
    void handleDeletedResource(const gl::Context *context, ResourceType resourceID)
    {
        if (isCaptureActive())
        {
            ResourceIDType idType    = GetResourceIDTypeFromType<ResourceType>::IDType;
            TrackedResource &tracker = mResourceTracker.getTrackedResource(context->id(), idType);
            tracker.setDeletedResource(resourceID.value);
        }
    }

    void *maybeGetShadowMemoryPointer(gl::Buffer *buffer, GLsizeiptr length, GLbitfield access);
    void determineMemoryProtectionSupport(gl::Context *context);

    angle::SimpleMutex &getFrameCaptureMutex() { return mFrameCaptureMutex; }

    void setDeferredLinkProgram(gl::ShaderProgramID programID)
    {
        mDeferredLinkPrograms.emplace(programID);
    }
    bool isDeferredLinkProgram(gl::ShaderProgramID programID)
    {
        return (mDeferredLinkPrograms.find(programID) != mDeferredLinkPrograms.end());
    }

  private:
    void writeJSON(const gl::Context *context);
    void writeCppReplayIndexFiles(const gl::Context *context, bool writeResetContextCall);
    void writeMainContextCppReplay(const gl::Context *context,
                                   const std::vector<CallCapture> &setupCalls,
                                   StateResetHelper &StateResetHelper);

    void captureClientArraySnapshot(const gl::Context *context,
                                    size_t vertexCount,
                                    size_t instanceCount);
    void captureMappedBufferSnapshot(const gl::Context *context, const CallCapture &call);

    void copyCompressedTextureData(const gl::Context *context, const CallCapture &call);
    void captureCompressedTextureData(const gl::Context *context, const CallCapture &call);

    void reset();
    void maybeOverrideEntryPoint(const gl::Context *context,
                                 CallCapture &call,
                                 std::vector<CallCapture> &newCalls);
    void maybeCapturePreCallUpdates(const gl::Context *context,
                                    CallCapture &call,
                                    std::vector<CallCapture> *shareGroupSetupCalls,
                                    ResourceIDToSetupCallsMap *resourceIDToSetupCalls);
    template <typename ParamValueType>
    void maybeGenResourceOnBind(const gl::Context *context, CallCapture &call);
    void maybeCapturePostCallUpdates(const gl::Context *context);
    void maybeCaptureDrawArraysClientData(const gl::Context *context,
                                          CallCapture &call,
                                          size_t instanceCount);
    void maybeCaptureDrawElementsClientData(const gl::Context *context,
                                            CallCapture &call,
                                            size_t instanceCount);
    void maybeCaptureCoherentBuffers(const gl::Context *context);
    void captureCustomMapBufferFromContext(const gl::Context *context,
                                           const char *entryPointName,
                                           CallCapture &call,
                                           std::vector<CallCapture> &callsOut);
    void updateCopyImageSubData(CallCapture &call);
    void overrideProgramBinary(const gl::Context *context,
                               CallCapture &call,
                               std::vector<CallCapture> &outCalls);
    void updateResourceCountsFromParamCapture(const ParamCapture &param, ResourceIDType idType);
    void updateResourceCountsFromCallCapture(const CallCapture &call);

    void runMidExecutionCapture(gl::Context *context);

    void scanSetupCalls(std::vector<CallCapture> &setupCalls);

    std::vector<CallCapture> mFrameCalls;

    // We save one large buffer of binary data for the whole CPP replay.
    // This simplifies a lot of file management.
    std::vector<uint8_t> mBinaryData;

    bool mEnabled;
    bool mSerializeStateEnabled;
    std::string mOutDirectory;
    std::string mCaptureLabel;
    bool mCompression;
    gl::AttribArray<int> mClientVertexArrayMap;
    uint32_t mFrameIndex;
    uint32_t mCaptureStartFrame;
    uint32_t mCaptureEndFrame;
    bool mIsFirstFrame   = true;
    bool mWroteIndexFile = false;
    SurfaceParamsMap mDrawSurfaceParams;
    gl::AttribArray<size_t> mClientArraySizes;
    size_t mReadBufferSize;
    size_t mResourceIDBufferSize;
    HasResourceTypeMap mHasResourceType;
    ResourceIDToSetupCallsMap mResourceIDToSetupCalls;
    BufferDataMap mBufferDataMap;
    bool mValidateSerializedState = false;
    std::string mValidationExpression;
    PackedEnumMap<ResourceIDType, uint32_t> mMaxAccessedResourceIDs;
    CoherentBufferTracker mCoherentBufferTracker;
    angle::SimpleMutex mFrameCaptureMutex;

    ResourceTracker mResourceTracker;
    ReplayWriter mReplayWriter;

    // If you don't know which frame you want to start capturing at, use the capture trigger.
    // Initialize it to the number of frames you want to capture, and then clear the value to 0 when
    // you reach the content you want to capture. Currently only available on Android.
    uint32_t mCaptureTrigger;

    bool mCaptureActive;
    std::vector<uint32_t> mActiveFrameIndices;

    // Cache most recently compiled and linked sources.
    ShaderSourceMap mCachedShaderSource;
    ProgramSourceMap mCachedProgramSources;

    // Set of programs which were created but not linked before capture was started
    std::set<gl::ShaderProgramID> mDeferredLinkPrograms;

    gl::ContextID mWindowSurfaceContextID;

    std::vector<CallCapture> mShareGroupSetupCalls;
    // Track which Contexts were created and made current at least once before MEC,
    // requiring setup for replay
    std::unordered_set<GLuint> mActiveContexts;

    // Invalid call counts per entry point while capture is active and inactive.
    std::unordered_map<EntryPoint, size_t> mInvalidCallCountsActive;
    std::unordered_map<EntryPoint, size_t> mInvalidCallCountsInactive;
};

template <typename CaptureFuncT, typename... ArgsT>
void CaptureGLCallToFrameCapture(CaptureFuncT captureFunc,
                                 bool isCallValid,
                                 gl::Context *context,
                                 ArgsT... captureParams)
{
    FrameCaptureShared *frameCaptureShared = context->getShareGroup()->getFrameCaptureShared();

    // EGL calls are protected by the global context mutex but only a subset of GL calls
    // are so protected. Ensure FrameCaptureShared access thread safety by using a
    // frame-capture only mutex.
    std::lock_guard<angle::SimpleMutex> lock(frameCaptureShared->getFrameCaptureMutex());

    if (!frameCaptureShared->isCapturing())
    {
        return;
    }

    CallCapture call = captureFunc(context->getState(), isCallValid, captureParams...);
    frameCaptureShared->captureCall(context, std::move(call), isCallValid);
}

template <typename FirstT, typename... OthersT>
egl::Display *GetEGLDisplayArg(FirstT display, OthersT... others)
{
    if constexpr (std::is_same<egl::Display *, FirstT>::value)
    {
        return display;
    }
    return nullptr;
}

template <typename CaptureFuncT, typename... ArgsT>
void CaptureEGLCallToFrameCapture(CaptureFuncT captureFunc,
                                  bool isCallValid,
                                  egl::Thread *thread,
                                  ArgsT... captureParams)
{
    gl::Context *context = thread->getContext();
    if (!context)
    {
        // Get a valid context from the display argument if no context is associated with this
        // thread
        egl::Display *display = GetEGLDisplayArg(captureParams...);
        if (display)
        {
            for (const auto &contextIter : display->getState().contextMap)
            {
                context = contextIter.second;
                break;
            }
        }
        if (!context)
        {
            return;
        }
    }
    std::lock_guard<egl::ContextMutex> lock(context->getContextMutex());

    angle::FrameCaptureShared *frameCaptureShared =
        context->getShareGroup()->getFrameCaptureShared();
    if (!frameCaptureShared->isCapturing())
    {
        return;
    }

    angle::CallCapture call = captureFunc(thread, isCallValid, captureParams...);
    frameCaptureShared->captureCall(context, std::move(call), true);
}

// Pointer capture helpers.
void CaptureMemory(const void *source, size_t size, ParamCapture *paramCapture);
void CaptureString(const GLchar *str, ParamCapture *paramCapture);
void CaptureStringLimit(const GLchar *str, uint32_t limit, ParamCapture *paramCapture);
void CaptureVertexPointerGLES1(const gl::State &glState,
                               gl::ClientVertexArrayType type,
                               const void *pointer,
                               ParamCapture *paramCapture);

gl::Program *GetProgramForCapture(const gl::State &glState, gl::ShaderProgramID handle);

// For GetIntegerv, GetFloatv, etc.
void CaptureGetParameter(const gl::State &glState,
                         GLenum pname,
                         size_t typeSize,
                         ParamCapture *paramCapture);

void CaptureGetActiveUniformBlockivParameters(const gl::State &glState,
                                              gl::ShaderProgramID handle,
                                              gl::UniformBlockIndex uniformBlockIndex,
                                              GLenum pname,
                                              ParamCapture *paramCapture);

template <typename T>
void CaptureClearBufferValue(GLenum buffer, const T *value, ParamCapture *paramCapture)
{
    // Per the spec, color buffers have a vec4, the rest a single value
    uint32_t valueSize = (buffer == GL_COLOR) ? 4 : 1;
    CaptureMemory(value, valueSize * sizeof(T), paramCapture);
}

void CaptureGenHandlesImpl(GLsizei n, GLuint *handles, ParamCapture *paramCapture);

template <typename T>
void CaptureGenHandles(GLsizei n, T *handles, ParamCapture *paramCapture)
{
    paramCapture->dataNElements = n;
    CaptureGenHandlesImpl(n, reinterpret_cast<GLuint *>(handles), paramCapture);
}

template <typename T>
void CaptureArray(T *elements, GLsizei n, ParamCapture *paramCapture)
{
    paramCapture->dataNElements = n;
    CaptureMemory(elements, n * sizeof(T), paramCapture);
}

void CaptureShaderStrings(GLsizei count,
                          const GLchar *const *strings,
                          const GLint *length,
                          ParamCapture *paramCapture);

bool IsTrackedPerContext(ResourceIDType type);
}  // namespace angle

template <typename T>
void CaptureTextureAndSamplerParameter_params(GLenum pname,
                                              const T *param,
                                              angle::ParamCapture *paramCapture)
{
    if (pname == GL_TEXTURE_BORDER_COLOR || pname == GL_TEXTURE_CROP_RECT_OES)
    {
        CaptureMemory(param, sizeof(T) * 4, paramCapture);
    }
    else
    {
        CaptureMemory(param, sizeof(T), paramCapture);
    }
}

namespace egl
{
angle::ParamCapture CaptureAttributeMap(const egl::AttributeMap &attribMap);
}  // namespace egl

#endif  // LIBANGLE_FRAME_CAPTURE_H_
