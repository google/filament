/*
* Copyright (C) 2022 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
 */

#ifndef TNT_METALARGUMENTBUFFER_H
#define TNT_METALARGUMENTBUFFER_H

#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include <backend/DriverEnums.h>

namespace filamat {

class MetalArgumentBuffer {
public:

    class Builder {
    public:

        /**
         * Set the name of the argument buffer structure.
         */
        Builder& name(const std::string& name) noexcept;

        /**
         * Add a texture argument to the argument buffer structure.
         * All combinations of type/format are supported, except for SAMPLER_3D/SHADOW.
         *
         * @param index the [[id(n)]] index of the texture argument
         * @param name the name of the texture argument
         * @param type controls the texture data type, e.g., texture2d, texturecube, etc
         * @param format controls the data format of the texture, e.g., int, float, etc
         */
        Builder& texture(size_t index, const std::string& name,
                filament::backend::SamplerType type,
                filament::backend::SamplerFormat format,
                bool multisample) noexcept;

        /**
         * Add a sampler argument to the argument buffer structure.
         * @param index the [[id(n)]] index of the texture argument
         * @param name the name of the texture argument
         */
        Builder& sampler(size_t index, const std::string& name) noexcept;

        /**
         * Add a buffer argument to the argument buffer structure.
         * @param index the [[id(n)]] index of the buffer argument
         * @param type the type of data the buffer points to
         * @param name the name of the buffer argument
         */
        Builder& buffer(size_t index, const std::string& type, const std::string& name) noexcept;

        MetalArgumentBuffer* build();

        friend class MetalArgumentBuffer;

    private:
        std::string mName;

        struct TextureArgument {
            std::string name;
            size_t index;
            filament::backend::SamplerType type;
            filament::backend::SamplerFormat format;
            bool multisample;

            std::ostream& write(std::ostream& os) const;
        };

        struct SamplerArgument {
            std::string name;
            size_t index;

            std::ostream& write(std::ostream& os) const;
        };

        struct BufferArgument {
            std::string name;
            size_t index;
            std::string type;

            std::ostream& write(std::ostream& os) const;
        };

        using ArgumentType = std::variant<TextureArgument, SamplerArgument, BufferArgument>;
        std::vector<ArgumentType> mArguments;
    };

    static void destroy(MetalArgumentBuffer** argumentBuffer);

    const std::string& getName() const noexcept { return mName; }

    /**
     * Returns the generated MSL argument buffer definition.
     */
    const std::string& getMsl() const noexcept { return mShaderText; }

    /**
     * Searches shader for the target argument buffer, and replaces it with the replacement string.
     *
     * @param shader the source MSL shader that contains the target argument buffer definition
     * @param targetArgBufferName the name of the argument buffer definition to replace
     * @param replacement the replacement argument buffer definition
     * @return true if the target was found, false otherwise
     */
    static bool replaceInShader(std::string& shader, const std::string& targetArgBufferName,
            const std::string& replacement) noexcept;
private:

    MetalArgumentBuffer(Builder& builder);
    ~MetalArgumentBuffer() = default;

    std::string mName;
    std::string mShaderText;
};

} // namespace filamat

#endif  // TNT_METALARGUMENTBUFFER_H
