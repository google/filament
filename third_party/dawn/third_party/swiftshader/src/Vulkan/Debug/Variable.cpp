// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Variable.hpp"

namespace vk {
namespace dbg {

std::atomic<int> Variables::nextID{};

Variables::~Variables() = default;

std::shared_ptr<Value> Variables::get(const std::string &name)
{
	std::shared_ptr<Value> found;
	foreach([&](const Variable &var) {
		if(var.name == name)
		{
			found = var.value;
			return false;
		}
		return true;
	});
	return found;
}

std::string Variables::string(const FormatFlags &fmt /* = FormatFlags::Default */)
{
	std::string out = "";
	auto subfmt = *fmt.subListFmt;
	subfmt.listIndent = fmt.listIndent + fmt.subListFmt->listIndent;
	bool first = true;
	foreach([&](const Variable &var) {
		if(!first) { out += fmt.listDelimiter; }
		first = false;
		out += fmt.listIndent;
		out += var.name;
		out += ": ";
		out += var.value->get(subfmt);
		return true;
	});
	return fmt.listPrefix + out + fmt.listSuffix;
}

}  // namespace dbg
}  // namespace vk
