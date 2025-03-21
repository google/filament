#include "spirv/unified1/spirv.hpp"
#include <utils/Log.h>

#include <functional>
#include <fstream>
#include <vector>
#include <unordered_map>

size_t getFileSize(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return -1;
    }
    size_t size = file.tellg();
    file.close();
    return size;
}

std::vector<uint32_t> transform(std::vector<uint32_t> const& spirv) {
    constexpr size_t HEADER_SIZE = 5;
    size_t const dataSize = spirv.size();
    std::vector<uint32_t> output;
    output.resize(dataSize);
    uint32_t const* data = (uint32_t*) spirv.data();
    uint32_t* outputData = output.data();
    memcpy(&outputData[0], &data[0], HEADER_SIZE * 4);

    struct Descriptor {
        uint32_t binding = 0xFFFFFFFF;
        uint32_t set = 0xFFFFFFFF;
        bool fixed = false;
    };

    std::unordered_map<uint32_t, Descriptor> descriptors;
    auto getDesc = [&](uint32_t var) -> Descriptor& {
        using itertype = std::unordered_map<uint32_t, Descriptor>::iterator;
        itertype itr;
        if (itr = descriptors.find(var); itr == descriptors.end()) {
            itr = descriptors.insert({var, {}}).first;
        }
        return itr->second;
    };

    auto pass = [&](uint32_t targetOp, std::function<void(uint32_t, uint32_t)> f) {
        for (uint32_t cursor = HEADER_SIZE, cursorEnd = dataSize; cursor < cursorEnd;) {
            uint32_t const firstWord = data[cursor];
            uint32_t const wordCount = firstWord >> 16;
            uint32_t const op = firstWord & 0x0000FFFF;
            if (targetOp == op) {
                f(wordCount-1, cursor+1);
            }
            cursor += wordCount;
        }
    };

    uint32_t samplerTypeId = 0;
    uint32_t imageTypeId = 0;

    // First we find the ID used to identify the sampler type and the image type.
    pass(spv::Op::OpTypeSampler, [&](uint32_t count, uint32_t pos) {
        samplerTypeId = data[pos];
    });

    pass(spv::Op::OpTypeImage, [&](uint32_t count, uint32_t pos) {
        imageTypeId = data[pos];
    });

    // Next we identify all the descriptors and record their set id and binding.
    pass(spv::Op::OpDecorate, [&](uint32_t count, uint32_t pos) {
        uint32_t const type = data[pos + 1];
        if (type == spv::Decoration::DecorationBinding) {
            uint32_t const targetVar = data[pos];
            uint32_t const binding = data[pos + 2];
            auto& desc = getDesc(targetVar);
            desc.binding = binding;
            // Note these decorations do not need to be written to the output.
        } else if (type == spv::Decoration::DecorationDescriptorSet) {
            uint32_t const targetVar = data[pos];
            uint32_t const set = data[pos + 2];
            auto& desc = getDesc(targetVar);
            desc.set = set;
        }
    });

    // If any descriptor is loaded to be a sampler, we offset it by a fixed amount. Note that it
    // could be "loaded" multiple times, so we use a boolean to skip if it's already offsetted.
    constexpr uint32_t SAMPLER_BINDING_OFFSET = 100;
    pass(spv::Op::OpLoad, [&](uint32_t count, uint32_t pos) {
        uint32_t const retType = data[pos];
        uint32_t const targetVar = data[pos + 2];
        if (retType == samplerTypeId) {
            auto& desc = getDesc(targetVar);
            if (!desc.fixed) {
                desc.binding += SAMPLER_BINDING_OFFSET;
                desc.fixed = true;
            }
        }
    });

    // Write out the offsetted bindings and copy the rest.
    for (uint32_t cursor = HEADER_SIZE, cursorEnd = dataSize; cursor < cursorEnd;) {
        uint32_t const firstWord = data[cursor];
        uint32_t const wordCount = firstWord >> 16;
        uint32_t const op = firstWord & 0x0000FFFF;

        uint32_t* out = &outputData[cursor];
        std::memcpy(out, &data[cursor], wordCount * 4);

        if (op == spv::Op::OpDecorate && data[cursor + 2] == spv::Decoration::DecorationBinding) {
            uint32_t const targetVar = data[cursor + 1];
            auto const& modified = getDesc(targetVar);
            out[3] = modified.binding;
        }
        cursor += wordCount;
    }

    // Note that you should be able to do this in-place (without making another vector).
    return output;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        utils::slog.e <<"need input and output spirv filenames" << utils::io::endl;
        return 1;
    }

    std::string infname {argv[1]};
    size_t const fsize = getFileSize(infname);
    size_t const wordCount = fsize / 4;

    std::ifstream inputFile(infname, std::ios::binary);

    std::vector<uint32_t> words;
    words.resize(wordCount);
    inputFile.read((char*) words.data(), fsize);
    inputFile.close();

    std::string outfname {argv[2]};
    std::vector<uint32_t> output = transform(words);
    std::ofstream outputFile(outfname, std::ios::binary);
    outputFile.write((char const*) output.data(), output.size() * 4);
    outputFile.close();

    return 0;
}
