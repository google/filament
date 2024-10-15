#include "vulkan/memory/ResourceManager.h"
#include "vulkan/VulkanHandles.h"

#include <utils/Panic.h>

namespace filament::backend::fvkmemory {

namespace {
uint32_t COUNTER[(size_t) ResourceType::UNDEFINED_TYPE] = {};

std::string str(ResourceType type) {
    switch (type) {
        case ResourceType::BUFFER_OBJECT:
            return "BufferObject";
        case ResourceType::INDEX_BUFFER:
            return "IndexBuffer";
        case ResourceType::PROGRAM:
            return "Program";
        case ResourceType::RENDER_TARGET:
            return "RenderTarget";
        case ResourceType::SWAP_CHAIN:
            return "SwapChain";
        case ResourceType::RENDER_PRIMITIVE:
            return "RenderPrimitive";
        case ResourceType::TEXTURE:
            return "Texture";
        case ResourceType::TEXTURE_STATE:
            return "TextureState";
        case ResourceType::TIMER_QUERY:
            return "TimerQuery";
        case ResourceType::VERTEX_BUFFER:
            return "VertexBuffer";
        case ResourceType::VERTEX_BUFFER_INFO:
            return "VertexBufferInfo";
        case ResourceType::DESCRIPTOR_SET_LAYOUT:
            return "DescriptorSetLayout";
        case ResourceType::DESCRIPTOR_SET:
            return "DescriptorSet";
        case ResourceType::FENCE:
            return "Fence";
        case ResourceType::UNDEFINED_TYPE:
            return "";
    }
}

}

ResourceManager* ResourceManager::sSingleton = nullptr;

ResourceManager::ResourceManager(size_t arenaSize, bool disableUseAfterFreeCheck)
    : mHandleAllocatorImpl("Handles", arenaSize, disableUseAfterFreeCheck),
      mPool(arenaSize) {}

void ResourceManager::gcImpl() {
    std::unique_lock<utils::Mutex> lock(mGcListMutex);
    for (auto const& [type, id]: mGcList) {
        ResourceManager::destroyWithType(type, id);
    }
    mGcList.clear();
}

void ResourceManager::destroyWithType(ResourceType type, HandleId id) {
    auto s = sSingleton;
    switch (type) {
        case ResourceType::BUFFER_OBJECT:
            s->destruct<VulkanBufferObject>(Handle<HwBufferObject>(id));
            break;
        case ResourceType::INDEX_BUFFER:
            s->destruct<VulkanIndexBuffer>(Handle<HwIndexBuffer>(id));
            break;
        case ResourceType::PROGRAM:
            s->destruct<VulkanProgram>(Handle<HwProgram>(id));
            break;
        case ResourceType::RENDER_TARGET:
            s->destruct<VulkanRenderTarget>(Handle<HwRenderTarget>(id));
            break;
        case ResourceType::SWAP_CHAIN:
            s->destruct<VulkanSwapChain>(Handle<HwSwapChain>(id));
            break;
        case ResourceType::RENDER_PRIMITIVE:
            s->destruct<VulkanRenderPrimitive>(Handle<VulkanRenderPrimitive>(id));
            break;
        case ResourceType::TEXTURE:
            s->destruct<VulkanTexture>(Handle<HwTexture>(id));
            break;
        case ResourceType::TEXTURE_STATE:
            s->destruct<VulkanTextureState>(Handle<VulkanTextureState>(id));
            break;
        case ResourceType::TIMER_QUERY:
            s->destruct<VulkanTimerQuery>(Handle<HwTimerQuery>(id));
            break;
        case ResourceType::VERTEX_BUFFER:
            s->destruct<VulkanVertexBuffer>(Handle<HwVertexBuffer>(id));
            break;
        case ResourceType::VERTEX_BUFFER_INFO:
            s->destruct<VulkanVertexBufferInfo>(Handle<HwVertexBufferInfo>(id));
            break;
        case ResourceType::DESCRIPTOR_SET_LAYOUT:
            s->destruct<VulkanDescriptorSetLayout>(Handle<VulkanDescriptorSetLayout>(id));
            break;
        case ResourceType::DESCRIPTOR_SET:
            s->destruct<VulkanDescriptorSet>(Handle<VulkanDescriptorSet>(id));
            break;
        case ResourceType::FENCE:
            s->destruct<VulkanFence>(Handle<VulkanFence>(id));
            break;
        case ResourceType::UNDEFINED_TYPE:
            break;
    }
    COUNTER[(size_t) type]--;
}

void ResourceManager::incType(ResourceType type, HandleId id) {
     COUNTER[(size_t) type]++;
}

void ResourceManager::printImpl() const {
    utils::slog.e << "-------------------" << utils::io::endl;
    for (size_t i = 0; i < (size_t) ResourceType::UNDEFINED_TYPE; ++i) {
        utils::slog.e <<"    " << str((ResourceType) i) << "=" << COUNTER[i] << utils::io::endl;
    }
    utils::slog.e << "+++++++++++++++++++" << utils::io::endl;
}

}
