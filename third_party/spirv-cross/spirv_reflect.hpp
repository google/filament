/*
 * Copyright 2018 Bradley Austin Davis
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SPIRV_CROSS_REFLECT_HPP
#define SPIRV_CROSS_REFLECT_HPP

#include "spirv_glsl.hpp"
#include <utility>
#include <vector>

namespace simple_json
{
class Stream;
}

namespace spirv_cross
{
class CompilerReflection : public CompilerGLSL
{
	using Parent = CompilerGLSL;

public:
	CompilerReflection(std::vector<uint32_t> spirv_)
	    : Parent(move(spirv_))
	{
		options.vulkan_semantics = true;
	}

	CompilerReflection(const uint32_t *ir, size_t word_count)
	    : Parent(ir, word_count)
	{
		options.vulkan_semantics = true;
	}

	void set_format(const std::string &format);
	std::string compile() override;

private:
	static std::string execution_model_to_str(spv::ExecutionModel model);

	void emit_entry_points();
	void emit_types();
	void emit_resources();
	void emit_specialization_constants();

	void emit_type(const SPIRType &type, bool &emitted_open_tag);
	void emit_type_member(const SPIRType &type, uint32_t index);
	void emit_type_member_qualifiers(const SPIRType &type, uint32_t index);
	void emit_type_array(const SPIRType &type);
	void emit_resources(const char *tag, const std::vector<Resource> &resources);

	std::string to_member_name(const SPIRType &type, uint32_t index) const;

	std::shared_ptr<simple_json::Stream> json_stream;
};

} // namespace spirv_cross

#endif
