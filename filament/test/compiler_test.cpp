#include <iostream>
#include <utils/Hash.h>
#include <tsl/robin_map.h>
#include <bluevk/BlueVK.h>

namespace {

struct T {
};

constexpr uint8_t const UNIQUE_DESCRIPTOR_SET_COUNT = 3;
constexpr uint8_t const SHADER_TYPE_COUNT = 3;
using Timestamp = uint64_t;
using VulkanResourceAllocator = T;

struct PushConstantKey {
    uint8_t stage;// We have one set of push constant per shader stage (fragment, vertex, etc).
    uint8_t size;
};

struct PipelineLayoutKey {
    using DescriptorSetLayoutArray = std::array<VkDescriptorSetLayout, UNIQUE_DESCRIPTOR_SET_COUNT>;
    DescriptorSetLayoutArray descSetLayouts = {};        // 8 * 3
    PushConstantKey pushConstant[SHADER_TYPE_COUNT] = {};// 2 * 3
    uint16_t padding = 0;
};
static_assert(sizeof(PipelineLayoutKey) == 32);

using PipelineLayoutKeyHashFn = utils::hash::MurmurHashFn<PipelineLayoutKey>;
struct PipelineLayoutKeyEqual {
    bool operator()(PipelineLayoutKey const& k1, PipelineLayoutKey const& k2) const {
        return 0 == memcmp((const void*) &k1, (const void*) &k2, sizeof(PipelineLayoutKey));
    }
};

struct PipelineLayoutCacheEntry {
    VkPipelineLayout handle;
    Timestamp lastUsed;
};

using PipelineLayoutMap = tsl::robin_map<PipelineLayoutKey, PipelineLayoutCacheEntry,
        PipelineLayoutKeyHashFn, PipelineLayoutKeyEqual>;

class TestClass {
public:
    VkDevice mDevice;
    VulkanResourceAllocator* mAllocator;
    Timestamp mTimestamp;
    PipelineLayoutMap layout;
};

void insert(PipelineLayoutMap& layout, uint64_t val) {
    PipelineLayoutKey key{};
    key.descSetLayouts[0] = (VkDescriptorSetLayout) val;
    layout[key] = {(VkPipelineLayout) val, val};
}

}// namespace

int main(int argc, char** argv) {
    constexpr uint32_t START = 1000;
    TestClass c;
    auto& layout = c.layout;

    uint64_t count = 0;
    for (; count < 20; count++) {
        insert(layout, count + START);
    }

    for (uint64_t t = 0; t < count; ++t) {
        PipelineLayoutKey key = {};
        key.descSetLayouts[0] = (VkDescriptorSetLayout)(START + t);

        if (layout.find(key) != layout.end()) {
            std::cout << "found " << t << std::endl;
        } else {
            std::cout << "not found" << t << std::endl;
        }
    }

    return 0;
}
