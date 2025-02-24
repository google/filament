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

#include "benchmark/benchmark.h"

#include <array>

namespace {

// https://en.wikipedia.org/wiki/Xorshift
class FastRnd
{
public:
	inline size_t operator()()
	{
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		return x;
	}

private:
	size_t x = 3243298;
};

struct ComplexKey
{
	std::array<size_t, 8> words;
};

bool operator==(const ComplexKey &a, const ComplexKey &b)
{
	for(size_t w = 0; w < a.words.size(); w++)
	{
		if(a.words[w] != b.words[w]) { return false; }
	}
	return true;
}

struct ComplexKeyHash
{
	size_t operator()(const ComplexKey &key) const
	{
		size_t hash = 12227;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			hash = hash * 11801 + key.words[w];
		}
		return hash;
	}
};

}  // namespace

class LRUCacheBenchmark : public benchmark::Fixture
{
public:
	void SetUp(const ::benchmark::State &state)
	{
		size = state.range(0);
	}

	void TearDown(const ::benchmark::State &state) {}

	size_t size;
};

BENCHMARK_DEFINE_F(LRUCacheBenchmark, AddInt)
(benchmark::State &state)
{
	sw::LRUCache<size_t, size_t> cache(size);
	FastRnd rnd;

	int i = 0;
	for(auto _ : state)
	{
		cache.add(rnd() % size, i);
		i++;
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, AddInt)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

BENCHMARK_DEFINE_F(LRUCacheBenchmark, GetIntCacheHit)
(benchmark::State &state)
{
	sw::LRUCache<size_t, size_t> cache(size);
	FastRnd rnd;

	for(size_t i = 0; i < size; i++)
	{
		cache.add(i, i);
	}

	for(auto _ : state)
	{
		cache.lookup(rnd() % size);
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, GetIntCacheHit)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

BENCHMARK_DEFINE_F(LRUCacheBenchmark, GetIntCacheMiss)
(benchmark::State &state)
{
	sw::LRUCache<size_t, size_t> cache(size);
	FastRnd rnd;
	for(size_t i = 0; i < size; i++)
	{
		cache.add(size + i, i);
	}

	for(auto _ : state)
	{
		cache.lookup(rnd() % size);
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, GetIntCacheMiss)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

BENCHMARK_DEFINE_F(LRUCacheBenchmark, AddRandomComplexKey)
(benchmark::State &state)
{
	sw::LRUCache<ComplexKey, size_t, ComplexKeyHash> cache(size);
	FastRnd rnd;

	int i = 0;
	for(auto _ : state)
	{
		ComplexKey key;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			key.words[w] = rnd() & 1;
		}

		cache.add(key, i);
		i++;
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, AddRandomComplexKey)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

BENCHMARK_DEFINE_F(LRUCacheBenchmark, GetComplexKeyCacheHit)
(benchmark::State &state)
{
	sw::LRUCache<ComplexKey, size_t, ComplexKeyHash> cache(size);
	FastRnd rnd;

	for(size_t i = 0; i < size; i++)
	{
		ComplexKey key;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			key.words[w] = (1ull << w);
		}
		cache.add(key, i);
	}

	for(auto _ : state)
	{
		auto i = rnd() & size;

		ComplexKey key;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			key.words[w] = i & (1ull << w);
		}
		cache.lookup(key);
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, GetComplexKeyCacheHit)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");

BENCHMARK_DEFINE_F(LRUCacheBenchmark, GetComplexKeyCacheMiss)
(benchmark::State &state)
{
	sw::LRUCache<ComplexKey, size_t, ComplexKeyHash> cache(size);
	FastRnd rnd;

	for(size_t i = 0; i < size; i++)
	{
		ComplexKey key;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			key.words[w] = 8 + (1ull << w);
		}
		cache.add(key, i);
	}

	for(auto _ : state)
	{
		auto i = rnd() & size;

		ComplexKey key;
		for(size_t w = 0; w < key.words.size(); w++)
		{
			key.words[w] = i & (1ull << w);
		}
		cache.lookup(key);
	}
}
BENCHMARK_REGISTER_F(LRUCacheBenchmark, GetComplexKeyCacheMiss)->RangeMultiplier(8)->Range(1, 0x100000)->ArgName("cache-size");
