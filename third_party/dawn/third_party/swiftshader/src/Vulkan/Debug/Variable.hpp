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

#ifndef VK_DEBUG_VARIABLE_HPP_
#define VK_DEBUG_VARIABLE_HPP_

#include "ID.hpp"
#include "Value.hpp"

#include "System/Debug.hpp"

#include "marl/mutex.h"
#include "marl/tsa.h"

#include <algorithm>
#include <atomic>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace vk {
namespace dbg {

// Variable is a named value.
struct Variable
{
	std::string name;
	std::shared_ptr<Value> value;

	// operator bool returns true iff value is not nullptr.
	operator bool() const { return value != nullptr; }
};

// Variables is an interface to a collection of named values.
class Variables
{
public:
	using ID = dbg::ID<Variables>;

	using ForeachCallback = std::function<bool(const Variable &)>;
	using FindCallback = std::function<void(const Variable &)>;

	inline Variables();
	virtual ~Variables();

	// foreach() calls cb with each of the variables in the container, while cb
	// returns true.
	// foreach() will return when cb returns false.
	inline void foreach(const ForeachCallback &cb);

	// foreach() calls cb with each of the variables in the container within the
	// indexed range: [startIndex, startIndex+count), while cb returns true.
	// foreach() will return when cb returns false.
	virtual void foreach(size_t startIndex, size_t count, const ForeachCallback &cb) = 0;

	// get() looks up and returns the variable with the given name.
	virtual std::shared_ptr<Value> get(const std::string &name);

	// string() returns the list of variables formatted to a string using the
	// given flags.
	virtual std::string string(const FormatFlags &fmt /* = FormatFlags::Default */);

	// The unique identifier of the variables.
	const ID id;

private:
	static std::atomic<int> nextID;
};

Variables::Variables()
    : id(nextID++)
{}

void Variables::foreach(const ForeachCallback &cb)
{
	foreach(0, std::numeric_limits<size_t>::max(), cb);
}

// VariableContainer is mutable collection of named values.
class VariableContainer : public Variables
{
public:
	using ID = dbg::ID<VariableContainer>;

	inline void foreach(size_t startIndex, size_t count, const ForeachCallback &cb) override;
	inline std::shared_ptr<Value> get(const std::string &name) override;

	// put() places the variable var into the container.
	inline void put(const Variable &var);

	// put() places the variable with the given name and value into the container.
	inline void put(const std::string &name, const std::shared_ptr<Value> &value);

	// extend() adds base to the list of Variables that will be searched and
	// traversed for variables after those in this VariableContainer are
	// searched / traversed.
	inline void extend(const std::shared_ptr<Variables> &base);

private:
	struct ForeachIndex
	{
		size_t start;
		size_t count;
	};

	template<typename F>
	inline void foreach(ForeachIndex &index, const F &cb);

	mutable marl::mutex mutex;
	std::vector<Variable> variables GUARDED_BY(mutex);
	std::unordered_map<std::string, int> indices GUARDED_BY(mutex);
	std::vector<std::shared_ptr<Variables>> extends GUARDED_BY(mutex);
};

void VariableContainer::foreach(size_t startIndex, size_t count, const ForeachCallback &cb)
{
	auto index = ForeachIndex{ startIndex, count };
	foreach(index, cb);
}

template<typename F>
void VariableContainer::foreach(ForeachIndex &index, const F &cb)
{
	marl::lock lock(mutex);
	for(size_t i = index.start; i < variables.size() && i < index.count; i++)
	{
		if(!cb(variables[i]))
		{
			return;
		}
	}

	index.start -= std::min(index.start, variables.size());
	index.count -= std::min(index.count, variables.size());

	for(auto &base : extends)
	{
		base->foreach(index.start, index.count, cb);
	}
}

std::shared_ptr<Value> VariableContainer::get(const std::string &name)
{
	marl::lock lock(mutex);
	for(const auto &var : variables)
	{
		if(var.name == name)
		{
			return var.value;
		}
	}
	for(auto &base : extends)
	{
		if(auto val = base->get(name))
		{
			return val;
		}
	}
	return nullptr;
}

void VariableContainer::put(const Variable &var)
{
	ASSERT(var.value);

	marl::lock lock(mutex);
	auto it = indices.find(var.name);
	if(it == indices.end())
	{
		indices.emplace(var.name, variables.size());
		variables.push_back(var);
	}
	else
	{
		variables[it->second].value = var.value;
	}
}

void VariableContainer::put(const std::string &name,
                            const std::shared_ptr<Value> &value)
{
	put({ name, value });
}

void VariableContainer::extend(const std::shared_ptr<Variables> &base)
{
	marl::lock lock(mutex);
	extends.emplace_back(base);
}

}  // namespace dbg
}  // namespace vk

#endif  // VK_DEBUG_VARIABLE_HPP_
