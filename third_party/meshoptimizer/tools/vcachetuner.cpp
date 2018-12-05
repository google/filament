#include "../demo/objparser.h"
#include "../src/meshoptimizer.h"

#include <algorithm>
#include <functional>
#include <vector>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

const int kCacheSizeMax = 16;
const int kValenceMax = 8;

namespace meshopt
{
extern thread_local float kVertexScoreTableCache[1 + kCacheSizeMax];
extern thread_local float kVertexScoreTableLive[1 + kValenceMax];
} // namespace meshopt

struct { int cache, warp, triangle; } profiles[] =
{
	{14, 64, 128}, // AMD GCN
	{32, 32, 32},  // NVidia Pascal
	// { 16, 32, 32 }, // NVidia Kepler, Maxwell
	// { 128, 0, 0 }, // Intel
};

const int Profile_Count = sizeof(profiles) / sizeof(profiles[0]);

struct pcg32_random_t
{
	uint64_t state;
	uint64_t inc;
};

#define PCG32_INITIALIZER { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL }

uint32_t pcg32_random_r(pcg32_random_t* rng)
{
	uint64_t oldstate = rng->state;
	// Advance internal state
	rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
	// Calculate output function (XSH RR), uses old state for max ILP
	uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	uint32_t rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

pcg32_random_t rngstate = PCG32_INITIALIZER;

float rand01()
{
	return pcg32_random_r(&rngstate) / float(1ull << 32);
}

uint32_t rand32()
{
	return pcg32_random_r(&rngstate);
}

struct State
{
	float cache[kCacheSizeMax];
	float live[kValenceMax];
};

struct Mesh
{
	size_t vertex_count;
	std::vector<unsigned int> indices;

	float atvr_base[Profile_Count];
};

Mesh gridmesh(unsigned int N)
{
	Mesh result;

	result.vertex_count = (N + 1) * (N + 1);
	result.indices.reserve(N * N * 6);

	for (unsigned int y = 0; y < N; ++y)
		for (unsigned int x = 0; x < N; ++x)
		{
			result.indices.push_back((y + 0) * (N + 1) + (x + 0));
			result.indices.push_back((y + 0) * (N + 1) + (x + 1));
			result.indices.push_back((y + 1) * (N + 1) + (x + 0));

			result.indices.push_back((y + 1) * (N + 1) + (x + 0));
			result.indices.push_back((y + 0) * (N + 1) + (x + 1));
			result.indices.push_back((y + 1) * (N + 1) + (x + 1));
		}

	return result;
}

Mesh objmesh(const char* path)
{
	ObjFile file;

	if (!objParseFile(file, path))
	{
		printf("Error loading %s: file not found\n", path);
		return Mesh();
	}

	if (!objValidate(file))
	{
		printf("Error loading %s: invalid file data\n", path);
		return Mesh();
	}

	size_t total_indices = file.f_size / 3;

	struct Vertex
	{
		float px, py, pz;
		float nx, ny, nz;
		float tx, ty;
	};

	std::vector<Vertex> vertices(total_indices);

	for (size_t i = 0; i < total_indices; ++i)
	{
		int vi = file.f[i * 3 + 0];
		int vti = file.f[i * 3 + 1];
		int vni = file.f[i * 3 + 2];

		Vertex v =
		    {
		        file.v[vi * 3 + 0],
		        file.v[vi * 3 + 1],
		        file.v[vi * 3 + 2],

		        vni >= 0 ? file.vn[vni * 3 + 0] : 0,
		        vni >= 0 ? file.vn[vni * 3 + 1] : 0,
		        vni >= 0 ? file.vn[vni * 3 + 2] : 0,

		        vti >= 0 ? file.vt[vti * 3 + 0] : 0,
		        vti >= 0 ? file.vt[vti * 3 + 1] : 0,
		    };

		vertices[i] = v;
	}

	Mesh result;

	std::vector<unsigned int> remap(total_indices);

	size_t total_vertices = meshopt_generateVertexRemap(&remap[0], NULL, total_indices, &vertices[0], total_indices, sizeof(Vertex));

	result.indices.resize(total_indices);
	meshopt_remapIndexBuffer(&result.indices[0], NULL, total_indices, &remap[0]);

	result.vertex_count = total_vertices;

	return result;
}

void compute_atvr(const State& state, const Mesh& mesh, float result[Profile_Count])
{
	memcpy(meshopt::kVertexScoreTableCache + 1, state.cache, kCacheSizeMax * sizeof(float));
	memcpy(meshopt::kVertexScoreTableLive + 1, state.live, kValenceMax * sizeof(float));

	std::vector<unsigned int> indices(mesh.indices.size());

	meshopt_optimizeVertexCache(&indices[0], &mesh.indices[0], mesh.indices.size(), mesh.vertex_count);

	for (int profile = 0; profile < Profile_Count; ++profile)
		result[profile] = meshopt_analyzeVertexCache(&indices[0], indices.size(), mesh.vertex_count, profiles[profile].cache, profiles[profile].warp, profiles[profile].triangle).atvr;
}

float fitness_score(const State& state, const std::vector<Mesh>& meshes)
{
	float result = 0;
	float count = 0;

	for (auto& mesh : meshes)
	{
		float atvr[Profile_Count];
		compute_atvr(state, mesh, atvr);

		for (int profile = 0; profile < Profile_Count; ++profile)
		{
			result += mesh.atvr_base[profile] / atvr[profile];
			count += 1;
		}
	}

	return result / count;
}

float rndcache()
{
	return rand01();
}

float rndlive()
{
	return rand01();
}

std::vector<State> gen0(size_t count)
{
	std::vector<State> result;

	for (size_t i = 0; i < count; ++i)
	{
		State state = {};

		for (int j = 0; j < kCacheSizeMax; ++j)
			state.cache[j] = rndcache();

		for (int j = 0; j < kValenceMax; ++j)
			state.live[j] = rndlive();

		result.push_back(state);
	}

	return result;
}

size_t rndindex(const std::vector<float>& prob)
{
	float r = rand01();

	for (size_t i = 0; i < prob.size(); ++i)
	{
		r -= prob[i];

		if (r <= 0)
			return i;
	}

	return prob.size() - 1;
}

State mutate(const State& state)
{
	State result = state;

	if (rand01() < 0.7f)
	{
		size_t idxcache = std::min(int(rand01() * kCacheSizeMax + 0.5f), int(kCacheSizeMax - 1));

		result.cache[idxcache] = rndcache();
	}

	if (rand01() < 0.7f)
	{
		size_t idxlive = std::min(int(rand01() * kValenceMax + 0.5f), int(kValenceMax - 1));

		result.live[idxlive] = rndlive();
	}

	if (rand01() < 0.2f)
	{
		uint32_t mask = rand32();

		for (size_t i = 0; i < kCacheSizeMax; ++i)
			if (mask & (1 << i))
				result.cache[i] *= 0.9f + 0.2f * rand01();
	}

	if (rand01() < 0.2f)
	{
		uint32_t mask = rand32();

		for (size_t i = 0; i < kValenceMax; ++i)
			if (mask & (1 << i))
				result.live[i] *= 0.9f + 0.2f * rand01();
	}

	if (rand01() < 0.05f)
	{
		uint32_t mask = rand32();

		for (size_t i = 0; i < kCacheSizeMax; ++i)
			if (mask & (1 << i))
				result.cache[i] = rndcache();
	}

	if (rand01() < 0.05f)
	{
		uint32_t mask = rand32();

		for (size_t i = 0; i < kValenceMax; ++i)
			if (mask & (1 << i))
				result.live[i] = rndlive();
	}

	return result;
}

bool accept(float fitnew, float fitold, float temp)
{
	if (fitnew >= fitold)
		return true;

	if (temp == 0)
		return false;

	float prob = exp2((fitnew - fitold) / temp);

	return rand01() < prob;
}

std::pair<State, float> genN_SA(std::vector<State>& seed, const std::vector<Mesh>& meshes, size_t steps)
{
	std::vector<State> result;
	result.reserve(seed.size() * (1 + steps));

	// perform several parallel steps of mutation for each temperature
	for (size_t i = 0; i < seed.size(); ++i)
	{
		result.push_back(seed[i]);

		for (size_t s = 0; s < steps; ++s)
			result.push_back(mutate(seed[i]));
	}

	// compute fitness for all temperatures & mutations in parallel
	std::vector<float> resultfit(result.size());

#pragma omp parallel for
	for (size_t i = 0; i < result.size(); ++i)
	{
		resultfit[i] = fitness_score(result[i], meshes);
	}

	// perform annealing for each temperature
	std::vector<float> seedfit(seed.size());

	for (size_t i = 0; i < seed.size(); ++i)
	{
		size_t offset = i * (1 + steps);

		seedfit[i] = resultfit[offset];

		float temp = (float(i) / float(seed.size() - 1)) / 0.1f;

		for (size_t s = 0; s < steps; ++s)
		{
			if (accept(resultfit[offset + s + 1], seedfit[i], temp))
			{
				seedfit[i] = resultfit[offset + s + 1];
				seed[i] = result[offset + s + 1];
			}
		}
	}

	// perform promotion from each temperature to the next one
	for (size_t i = seed.size() - 1; i > 0; --i)
	{
		if (seedfit[i] > seedfit[i - 1])
		{
			seedfit[i - 1] = seedfit[i];
			seed[i - 1] = seed[i];
		}
	}

	return std::make_pair(seed[0], seedfit[0]);
}

std::pair<State, float> genN_GA(std::vector<State>& seed, const std::vector<Mesh>& meshes, float crossover, float mutate)
{
	std::vector<State> result;
	result.reserve(seed.size());

	std::vector<float> seedprob(seed.size());

#pragma omp parallel for
	for (size_t i = 0; i < seed.size(); ++i)
	{
		seedprob[i] = fitness_score(seed[i], meshes);
	}

	State best = {};
	float bestfit = 0;
	float probsum = 0;

	for (size_t i = 0; i < seed.size(); ++i)
	{
		float score = seedprob[i];
		probsum += score;

		if (score > bestfit)
		{
			best = seed[i];
			bestfit = score;
		}
	}

	for (auto& prob : seedprob)
	{
		prob /= probsum;
	}

	std::vector<unsigned int> seedidx;
	seedidx.reserve(seed.size());
	for (size_t i = 0; i < seed.size(); ++i)
		seedidx.push_back(i);

	std::sort(seedidx.begin(), seedidx.end(), [&](size_t l, size_t r) { return seedprob[l] < seedprob[r]; });

	while (result.size() < seed.size() / 4)
	{
		size_t idx = seedidx.back();
		seedidx.pop_back();

		result.push_back(seed[idx]);
	}

	while (result.size() < seed.size())
	{
		State s0 = seed[rndindex(seedprob)];
		State s1 = seed[rndindex(seedprob)];

		State state = s0;

		// crossover
		if (rand01() < crossover)
		{
			size_t idxcache = std::min(int(rand01() * kCacheSizeMax + 0.5f), 15);

			memcpy(state.cache + idxcache, s1.cache + idxcache, (kCacheSizeMax - idxcache) * sizeof(float));
		}

		if (rand01() < crossover)
		{
			size_t idxlive = std::min(int(rand01() * kValenceMax + 0.5f), 7);

			memcpy(state.live + idxlive, s1.live + idxlive, (kValenceMax - idxlive) * sizeof(float));
		}

		// mutate
		if (rand01() < mutate)
		{
			size_t idxcache = std::min(int(rand01() * kCacheSizeMax + 0.5f), 15);

			state.cache[idxcache] = rndcache();
		}

		if (rand01() < mutate)
		{
			size_t idxlive = std::min(int(rand01() * kValenceMax + 0.5f), 7);

			state.live[idxlive] = rndlive();
		}

		result.push_back(state);
	}

	seed.swap(result);

	return std::make_pair(best, bestfit);
}

bool load_state(const char* path, std::vector<State>& result)
{
	FILE* file = fopen(path, "rb");
	if (!file)
		return false;

	State state;

	result.clear();

	while (fread(&state, sizeof(State), 1, file) == 1)
		result.push_back(state);

	fclose(file);

	return true;
}

bool save_state(const char* path, const std::vector<State>& result)
{
	FILE* file = fopen(path, "wb");
	if (!file)
		return false;

	for (auto& state : result)
	{
		if (fwrite(&state, sizeof(State), 1, file) != 1)
		{
			fclose(file);
			return false;
		}
	}

	return fclose(file) == 0;
}

void dump_state(const State& state)
{
	printf("cache:");
	for (int i = 0; i < kCacheSizeMax; ++i)
	{
		printf(" %.3f", state.cache[i]);
	}
	printf("\n");

	printf("live:");
	for (int i = 0; i < kValenceMax; ++i)
	{
		printf(" %.3f", state.live[i]);
	}
	printf("\n");
}

int main(int argc, char** argv)
{
	bool annealing = false;

	State baseline;
	memcpy(baseline.cache, meshopt::kVertexScoreTableCache + 1, kCacheSizeMax * sizeof(float));
	memcpy(baseline.live, meshopt::kVertexScoreTableLive + 1, kValenceMax * sizeof(float));

	std::vector<Mesh> meshes;

	meshes.push_back(gridmesh(50));

	for (int i = 1; i < argc; ++i)
		meshes.push_back(objmesh(argv[i]));

	size_t total_triangles = 0;

	for (auto& mesh : meshes)
	{
		compute_atvr(baseline, mesh, mesh.atvr_base);

		total_triangles += mesh.indices.size() / 3;
	}

	std::vector<State> pop;
	size_t gen = 0;

	if (load_state("mutator.state", pop))
	{
		printf("Loaded %d state vectors\n", int(pop.size()));
	}
	else
	{
		pop = gen0(annealing ? 32 : 1000);
	}

	printf("%d meshes, %.1fM triangles\n", int(meshes.size()), double(total_triangles) / 1e6);

	float atvr_0[Profile_Count];
	float atvr_N[Profile_Count];
	compute_atvr(baseline, meshes[0], atvr_0);
	compute_atvr(baseline, meshes.back(), atvr_N);

	printf("baseline: grid %f %f %s %f %f\n", atvr_0[0], atvr_0[1], argv[argc - 1], atvr_N[0], atvr_N[1]);

	for (;;)
	{
		auto best = annealing ? genN_SA(pop, meshes, 31) : genN_GA(pop, meshes, 0.7f, 0.3f);
		gen++;

		compute_atvr(best.first, meshes[0], atvr_0);
		compute_atvr(best.first, meshes.back(), atvr_N);

		printf("%d: fitness %f; grid %f %f %s %f %f\n", int(gen), best.second, atvr_0[0], atvr_0[1], argv[argc - 1], atvr_N[0], atvr_N[1]);

		if (gen % 100 == 0)
		{
			char buf[128];
			sprintf(buf, "gcloud logging write vcache-log \"fitness %f; grid %f %f %s %f %f\"", best.second, atvr_0[0], atvr_0[1], argv[argc - 1], atvr_N[0], atvr_N[1]);
			system(buf);
		}

		dump_state(best.first);

		if (save_state("mutator.state-temp", pop) && rename("mutator.state-temp", "mutator.state") == 0)
		{
		}
		else
		{
			printf("ERROR: Can't save state\n");
		}
	}
}
