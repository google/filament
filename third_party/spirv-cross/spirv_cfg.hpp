/*
 * Copyright 2016-2018 ARM Limited
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

#ifndef SPIRV_CROSS_CFG_HPP
#define SPIRV_CROSS_CFG_HPP

#include "spirv_common.hpp"
#include <assert.h>

namespace spirv_cross
{
class Compiler;
class CFG
{
public:
	CFG(Compiler &compiler, const SPIRFunction &function);

	Compiler &get_compiler()
	{
		return compiler;
	}

	const Compiler &get_compiler() const
	{
		return compiler;
	}

	const SPIRFunction &get_function() const
	{
		return func;
	}

	uint32_t get_immediate_dominator(uint32_t block) const
	{
		return immediate_dominators[block];
	}

	uint32_t get_visit_order(uint32_t block) const
	{
		int v = visit_order[block];
		assert(v > 0);
		return uint32_t(v);
	}

	uint32_t find_common_dominator(uint32_t a, uint32_t b) const;

	const std::vector<uint32_t> &get_preceding_edges(uint32_t block) const
	{
		return preceding_edges[block];
	}

	const std::vector<uint32_t> &get_succeeding_edges(uint32_t block) const
	{
		return succeeding_edges[block];
	}

	template <typename Op>
	void walk_from(std::unordered_set<uint32_t> &seen_blocks, uint32_t block, const Op &op) const
	{
		if (seen_blocks.count(block))
			return;
		seen_blocks.insert(block);

		op(block);
		for (auto b : succeeding_edges[block])
			walk_from(seen_blocks, b, op);
	}

private:
	Compiler &compiler;
	const SPIRFunction &func;
	std::vector<std::vector<uint32_t>> preceding_edges;
	std::vector<std::vector<uint32_t>> succeeding_edges;
	std::vector<uint32_t> immediate_dominators;
	std::vector<int> visit_order;
	std::vector<uint32_t> post_order;

	void add_branch(uint32_t from, uint32_t to);
	void build_post_order_visit_order();
	void build_immediate_dominators();
	bool post_order_visit(uint32_t block);
	uint32_t visit_count = 0;

	bool is_back_edge(uint32_t to) const;
};

class DominatorBuilder
{
public:
	DominatorBuilder(const CFG &cfg);

	void add_block(uint32_t block);
	uint32_t get_dominator() const
	{
		return dominator;
	}

	void lift_continue_block_dominator();

private:
	const CFG &cfg;
	uint32_t dominator = 0;
};
} // namespace spirv_cross

#endif
