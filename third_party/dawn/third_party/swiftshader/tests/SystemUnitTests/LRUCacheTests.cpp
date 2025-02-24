// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "System/LRUCache.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

using namespace sw;

namespace {

template<typename Cache>
void checkRange(const Cache &cache, std::vector<std::pair<typename Cache::Key, typename Cache::Data>> list)
{
	size_t i = 0;
	for(auto it : cache)
	{
		ASSERT_EQ(list[i].first, it.key());
		ASSERT_EQ(list[i].second, it.data());
		i++;
	}
	ASSERT_EQ(i, list.size());
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// LRUCache
////////////////////////////////////////////////////////////////////////////////
TEST(LRUCache, Empty)
{
	LRUCache<std::string, std::string> cache(8);
	ASSERT_EQ(cache.lookup(""), "");
	ASSERT_EQ(cache.lookup("123"), "");
	bool looped = false;
	for(auto ignored : cache)
	{
		(void)ignored;
		looped = true;
	}
	if(looped)
	{
		FAIL() << "Should not loop on empty cache";
	}
}

TEST(LRUCache, AddNoEviction)
{
	LRUCache<std::string, std::string> cache(4);

	cache.add("1", "one");
	cache.add("2", "two");
	cache.add("3", "three");
	cache.add("4", "four");

	ASSERT_EQ(cache.lookup("1"), "one");
	ASSERT_EQ(cache.lookup("2"), "two");
	ASSERT_EQ(cache.lookup("3"), "three");
	ASSERT_EQ(cache.lookup("4"), "four");

	checkRange(cache, {
	                      { "4", "four" },
	                      { "3", "three" },
	                      { "2", "two" },
	                      { "1", "one" },
	                  });
}

TEST(LRUCache, AddWithEviction)
{
	LRUCache<std::string, std::string> cache(4);

	cache.add("1", "one");
	cache.add("2", "two");
	cache.add("3", "three");
	cache.add("4", "four");
	cache.add("5", "five");
	cache.add("6", "six");

	ASSERT_EQ(cache.lookup("1"), "");
	ASSERT_EQ(cache.lookup("2"), "");
	ASSERT_EQ(cache.lookup("3"), "three");
	ASSERT_EQ(cache.lookup("4"), "four");
	ASSERT_EQ(cache.lookup("5"), "five");
	ASSERT_EQ(cache.lookup("6"), "six");

	checkRange(cache, {
	                      { "6", "six" },
	                      { "5", "five" },
	                      { "4", "four" },
	                      { "3", "three" },
	                  });
}

TEST(LRUCache, AddClearAdd)
{
	LRUCache<std::string, std::string> cache(4);

	// Add some data.
	cache.add("1", "one");
	cache.add("2", "two");
	cache.add("3", "three");
	cache.add("4", "four");
	cache.add("5", "five");
	cache.add("6", "six");

	// Clear it.
	cache.clear();

	// Check has no data.
	ASSERT_EQ(cache.lookup("1"), "");
	ASSERT_EQ(cache.lookup("2"), "");
	ASSERT_EQ(cache.lookup("3"), "");
	ASSERT_EQ(cache.lookup("4"), "");
	ASSERT_EQ(cache.lookup("5"), "");
	ASSERT_EQ(cache.lookup("6"), "");

	checkRange(cache, {});

	// Add it again.
	cache.add("1", "one");
	cache.add("2", "two");
	cache.add("3", "three");
	cache.add("4", "four");
	cache.add("5", "five");
	cache.add("6", "six");

	// Check has data.
	ASSERT_EQ(cache.lookup("1"), "");
	ASSERT_EQ(cache.lookup("2"), "");
	ASSERT_EQ(cache.lookup("3"), "three");
	ASSERT_EQ(cache.lookup("4"), "four");
	ASSERT_EQ(cache.lookup("5"), "five");
	ASSERT_EQ(cache.lookup("6"), "six");

	checkRange(cache, {
	                      { "6", "six" },
	                      { "5", "five" },
	                      { "4", "four" },
	                      { "3", "three" },
	                  });
}

TEST(LRUCache, Reordering)
{
	LRUCache<std::string, std::string> cache(4);

	// Fill
	cache.add("1", "one");
	cache.add("2", "two");
	cache.add("3", "three");
	cache.add("4", "four");

	// Push 2 then 4 to most recent
	cache.add("2", "two");
	cache.add("4", "four");

	checkRange(cache, {
	                      { "4", "four" },
	                      { "2", "two" },
	                      { "3", "three" },
	                      { "1", "one" },
	                  });
}