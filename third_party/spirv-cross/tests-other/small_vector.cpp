/*
 * Copyright 2019 Hans-Kristian Arntzen
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

#include "spirv_cross.hpp"
#include <memory>

using namespace spirv_cross;

// Test the tricky bits of the implementation.
// Running the entire test suite on this implementation should find all other potential issues.

static int allocations = 0;
static int deallocations = 0;

#define SPVC_ASSERT(x) do { \
	if (!(x)) SPIRV_CROSS_THROW("Assert: " #x " failed!"); \
} while(0)

struct RAIIInt
{
	RAIIInt(int v_) : v(v_) { allocations++; }
	~RAIIInt() { deallocations++; }
	RAIIInt() { allocations++; }
	RAIIInt(const RAIIInt &other) { v = other.v; allocations++; }
	RAIIInt(RAIIInt &&other) SPIRV_CROSS_NOEXCEPT { v = other.v; allocations++; }
	RAIIInt &operator=(RAIIInt &&) = default;
	RAIIInt &operator=(const RAIIInt &) = default;

	int v = 0;
};

static void propagate_stack_to_heap()
{
	SmallVector<RAIIInt, 2> ints;
	ints.emplace_back(1);
	ints.emplace_back(2);
	auto *old_data = ints.data();
	SPVC_ASSERT(ints[0].v == 1);
	SPVC_ASSERT(ints[1].v == 2);
	ints.emplace_back(3);
	SPVC_ASSERT(old_data != ints.data());
	SPVC_ASSERT(ints[0].v == 1);
	SPVC_ASSERT(ints[1].v == 2);
	SPVC_ASSERT(ints[2].v == 3);
	SPVC_ASSERT(ints.size() == 3);
}

static void insert_end()
{
	SmallVector<RAIIInt, 2> ints;
	ints.emplace_back(1);
	ints.emplace_back(2);

	const RAIIInt new_ints[3] = { 10, 20, 30 };
	ints.insert(ints.end(), new_ints, new_ints + 3);
	SPVC_ASSERT(ints.size() == 5);

	SPVC_ASSERT(ints[0].v == 1);
	SPVC_ASSERT(ints[1].v == 2);
	SPVC_ASSERT(ints[2].v == 10);
	SPVC_ASSERT(ints[3].v == 20);
	SPVC_ASSERT(ints[4].v == 30);
}

static void insert_begin_realloc()
{
	SmallVector<RAIIInt, 2> ints;
	ints.emplace_back(1);
	ints.emplace_back(2);

	const RAIIInt new_ints[3] = { 10, 20, 30 };
	ints.insert(ints.begin(), new_ints, new_ints + 3);
	SPVC_ASSERT(ints.size() == 5);

	SPVC_ASSERT(ints[0].v == 10);
	SPVC_ASSERT(ints[1].v == 20);
	SPVC_ASSERT(ints[2].v == 30);
	SPVC_ASSERT(ints[3].v == 1);
	SPVC_ASSERT(ints[4].v == 2);
}

static void insert_middle_realloc()
{
	SmallVector<RAIIInt, 2> ints;
	ints.emplace_back(1);
	ints.emplace_back(2);

	const RAIIInt new_ints[3] = { 10, 20, 30 };
	ints.insert(ints.begin() + 1, new_ints, new_ints + 3);
	SPVC_ASSERT(ints.size() == 5);

	SPVC_ASSERT(ints[0].v == 1);
	SPVC_ASSERT(ints[1].v == 10);
	SPVC_ASSERT(ints[2].v == 20);
	SPVC_ASSERT(ints[3].v == 30);
	SPVC_ASSERT(ints[4].v == 2);
}

static void insert_begin_no_realloc()
{
	SmallVector<RAIIInt, 2> ints;
	ints.reserve(10);
	ints.emplace_back(1);
	ints.emplace_back(2);

	const RAIIInt new_ints[3] = { 10, 20, 30 };
	ints.insert(ints.begin(), new_ints, new_ints + 3);
	SPVC_ASSERT(ints.size() == 5);

	SPVC_ASSERT(ints[0].v == 10);
	SPVC_ASSERT(ints[1].v == 20);
	SPVC_ASSERT(ints[2].v == 30);
	SPVC_ASSERT(ints[3].v == 1);
	SPVC_ASSERT(ints[4].v == 2);
}

static void insert_middle_no_realloc()
{
	SmallVector<RAIIInt, 2> ints;
	ints.reserve(10);
	ints.emplace_back(1);
	ints.emplace_back(2);

	const RAIIInt new_ints[3] = { 10, 20, 30 };
	ints.insert(ints.begin() + 1, new_ints, new_ints + 3);
	SPVC_ASSERT(ints.size() == 5);

	SPVC_ASSERT(ints[0].v == 1);
	SPVC_ASSERT(ints[1].v == 10);
	SPVC_ASSERT(ints[2].v == 20);
	SPVC_ASSERT(ints[3].v == 30);
	SPVC_ASSERT(ints[4].v == 2);
}

static void erase_end()
{
	SmallVector<RAIIInt, 2> ints;
	ints.emplace_back(1);
	ints.emplace_back(2);
	ints.emplace_back(3);
	ints.emplace_back(4);
	ints.erase(ints.begin() + 1, ints.end());

	SPVC_ASSERT(ints.size() == 1);
	SPVC_ASSERT(ints[0].v == 1);
}

static void erase_middle()
{
	SmallVector<RAIIInt, 2> ints;
	ints.emplace_back(1);
	ints.emplace_back(2);
	ints.emplace_back(3);
	ints.emplace_back(4);
	ints.erase(ints.begin() + 1, ints.end() - 1);

	SPVC_ASSERT(ints.size() == 2);
	SPVC_ASSERT(ints[0].v == 1);
	SPVC_ASSERT(ints[1].v == 4);
}

static void erase_start()
{
	SmallVector<RAIIInt, 2> ints;
	ints.emplace_back(1);
	ints.emplace_back(2);
	ints.emplace_back(3);
	ints.emplace_back(4);
	ints.erase(ints.begin(), ints.end() - 2);

	SPVC_ASSERT(ints.size() == 2);
	SPVC_ASSERT(ints[0].v == 3);
	SPVC_ASSERT(ints[1].v == 4);
}

static void convert_to_std_vector()
{
	SmallVector<RAIIInt, 4> foo;
	foo.push_back(1);
	foo.push_back(2);
	std::vector<RAIIInt> ints(foo);
	SPVC_ASSERT(ints.size() == 2);
	SPVC_ASSERT(foo.size() == 2);
	SPVC_ASSERT(ints[0].v == 1);
	SPVC_ASSERT(ints[1].v == 2);

	// This doesn't work on MSVC 2013. Ignore it.
#if !(defined(_MSC_VER) && _MSC_VER < 1900)
	SmallVector<std::unique_ptr<RAIIInt>> move_only_buffer;
	move_only_buffer.emplace_back(new RAIIInt(40));
	std::vector<std::unique_ptr<RAIIInt>> move_only_vector(std::move(move_only_buffer));
	SPVC_ASSERT(move_only_vector.size() == 1);
	SPVC_ASSERT(move_only_vector[0]->v == 40);
#endif
}

int main()
{
	propagate_stack_to_heap();
	insert_end();
	insert_begin_realloc();
	insert_begin_no_realloc();
	insert_middle_realloc();
	insert_middle_no_realloc();
	erase_end();
	erase_middle();
	erase_start();

	convert_to_std_vector();

	SPVC_ASSERT(allocations > 0 && deallocations > 0 && deallocations == allocations);
}

