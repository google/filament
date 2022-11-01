#include "../src/meshoptimizer.h"
#include "../extern/fast_obj.h"

#define SDEFL_IMPLEMENTATION
#include "../extern/sdefl.h"

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
	struct VertexScoreTable
	{
		float cache[1 + kCacheSizeMax];
		float live[1 + kValenceMax];
	};
} // namespace meshopt

void meshopt_optimizeVertexCacheTable(unsigned int* destination, const unsigned int* indices, size_t index_count, size_t vertex_count, const meshopt::VertexScoreTable* table);

struct Profile
{
	float weight;
	int cache, warp, triangle; // vcache tuning parameters
	int compression;
};

Profile profiles[] =
{
	{1.f, 0, 0, 0, 0},  // Compression
	{1.f, 0, 0, 0, 1},  // Compression w/deflate
	// {1.f, 14, 64, 128}, // AMD GCN
	// {1.f, 32, 32, 32},  // NVidia Pascal
	// {1.f, 16, 32, 32}, // NVidia Kepler, Maxwell
	// {1.f, 128, 0, 0}, // Intel
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
	float fitness;
};

struct Mesh
{
	const char* name;

	size_t vertex_count;
	std::vector<unsigned int> indices;

	float metric_base[Profile_Count];
};

Mesh gridmesh(unsigned int N)
{
	Mesh result;

	result.name = "grid";

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
	fastObjMesh* obj = fast_obj_read(path);
	if (!obj)
	{
		printf("Error loading %s: file not found\n", path);
		return Mesh();
	}

	size_t total_indices = 0;

	for (unsigned int i = 0; i < obj->face_count; ++i)
		total_indices += 3 * (obj->face_vertices[i] - 2);

	struct Vertex
	{
		float px, py, pz;
		float nx, ny, nz;
		float tx, ty;
	};

	std::vector<Vertex> vertices(total_indices);

	size_t vertex_offset = 0;
	size_t index_offset = 0;

	for (unsigned int i = 0; i < obj->face_count; ++i)
	{
		for (unsigned int j = 0; j < obj->face_vertices[i]; ++j)
		{
			fastObjIndex gi = obj->indices[index_offset + j];

			Vertex v =
			    {
			        obj->positions[gi.p * 3 + 0],
			        obj->positions[gi.p * 3 + 1],
			        obj->positions[gi.p * 3 + 2],
			        obj->normals[gi.n * 3 + 0],
			        obj->normals[gi.n * 3 + 1],
			        obj->normals[gi.n * 3 + 2],
			        obj->texcoords[gi.t * 2 + 0],
			        obj->texcoords[gi.t * 2 + 1],
			    };

			// triangulate polygon on the fly; offset-3 is always the first polygon vertex
			if (j >= 3)
			{
				vertices[vertex_offset + 0] = vertices[vertex_offset - 3];
				vertices[vertex_offset + 1] = vertices[vertex_offset - 1];
				vertex_offset += 2;
			}

			vertices[vertex_offset] = v;
			vertex_offset++;
		}

		index_offset += obj->face_vertices[i];
	}

	fast_obj_destroy(obj);

	Mesh result;

	result.name = path;

	std::vector<unsigned int> remap(total_indices);

	size_t total_vertices = meshopt_generateVertexRemap(&remap[0], NULL, total_indices, &vertices[0], total_indices, sizeof(Vertex));

	result.indices.resize(total_indices);
	meshopt_remapIndexBuffer(&result.indices[0], NULL, total_indices, &remap[0]);

	result.vertex_count = total_vertices;

	return result;
}

template <typename T>
size_t compress(const std::vector<T>& data, int level = SDEFL_LVL_DEF)
{
	std::vector<unsigned char> cbuf(sdefl_bound(int(data.size() * sizeof(T))));
	sdefl s = {};
	return sdeflate(&s, &cbuf[0], reinterpret_cast<const unsigned char*>(&data[0]), int(data.size() * sizeof(T)), level);
}

void compute_metric(const State* state, const Mesh& mesh, float result[Profile_Count])
{
	std::vector<unsigned int> indices(mesh.indices.size());

	if (state)
	{
		meshopt::VertexScoreTable table = {};
		memcpy(table.cache + 1, state->cache, kCacheSizeMax * sizeof(float));
		memcpy(table.live + 1, state->live, kValenceMax * sizeof(float));
		meshopt_optimizeVertexCacheTable(&indices[0], &mesh.indices[0], mesh.indices.size(), mesh.vertex_count, &table);
	}
	else
	{
		meshopt_optimizeVertexCache(&indices[0], &mesh.indices[0], mesh.indices.size(), mesh.vertex_count);
	}

	meshopt_optimizeVertexFetch(NULL, &indices[0], indices.size(), NULL, mesh.vertex_count, 0);

	std::vector<unsigned char> ibuf;

	for (int profile = 0; profile < Profile_Count; ++profile)
	{
		if (profiles[profile].cache == 0)
		{
			ibuf.resize(meshopt_encodeIndexBufferBound(indices.size(), mesh.vertex_count));
			ibuf.resize(meshopt_encodeIndexBuffer(&ibuf[0], ibuf.size(), &indices[0], indices.size()));
		}
	}

	for (int profile = 0; profile < Profile_Count; ++profile)
	{
		if (profiles[profile].cache)
		{
			meshopt_VertexCacheStatistics stats = meshopt_analyzeVertexCache(&indices[0], indices.size(), mesh.vertex_count, profiles[profile].cache, profiles[profile].warp, profiles[profile].triangle);
			result[profile] = stats.atvr;
		}
		else
		{
			// take into account both pre-deflate and post-deflate size but focus a bit more on post-deflate
			size_t csize = profiles[profile].compression ? compress(ibuf) : ibuf.size();

			result[profile] = double(csize) / double(indices.size() / 3);
		}
	}
}

float fitness_score(const State& state, const std::vector<Mesh>& meshes)
{
	float result = 0;
	float count = 0;

	for (auto& mesh : meshes)
	{
		float metric[Profile_Count];
		compute_metric(&state, mesh, metric);

		for (int profile = 0; profile < Profile_Count; ++profile)
		{
			result += mesh.metric_base[profile] / metric[profile] * profiles[profile].weight;
			count += profiles[profile].weight;
		}
	}

	return result / count;
}

std::vector<State> gen0(size_t count, const std::vector<Mesh>& meshes)
{
	std::vector<State> result;

	for (size_t i = 0; i < count; ++i)
	{
		State state = {};

		for (int j = 0; j < kCacheSizeMax; ++j)
			state.cache[j] = rand01();

		for (int j = 0; j < kValenceMax; ++j)
			state.live[j] = rand01();

		state.fitness = fitness_score(state, meshes);

		result.push_back(state);
	}

	return result;
}

// https://en.wikipedia.org/wiki/Differential_evolution
// Good Parameters for Differential Evolution. Magnus Erik Hvass Pedersen, 2010
std::pair<State, float> genN(std::vector<State>& seed, const std::vector<Mesh>& meshes, float crossover = 0.8803f, float weight = 0.4717f)
{
	std::vector<State> result(seed.size());

	for (size_t i = 0; i < seed.size(); ++i)
	{
		for (;;)
		{
			int a = rand32() % seed.size();
			int b = rand32() % seed.size();
			int c = rand32() % seed.size();

			if (a == b || a == c || b == c || a == int(i) || b == int(i) || c == int(i))
				continue;

			int rc = rand32() % kCacheSizeMax;
			int rl = rand32() % kValenceMax;

			for (int j = 0; j < kCacheSizeMax; ++j)
			{
				float r = rand01();

				if (r < crossover || j == rc)
					result[i].cache[j] = std::max(0.f, std::min(1.f, seed[a].cache[j] + weight * (seed[b].cache[j] - seed[c].cache[j])));
				else
					result[i].cache[j] = seed[i].cache[j];
			}

			for (int j = 0; j < kValenceMax; ++j)
			{
				float r = rand01();

				if (r < crossover || j == rl)
					result[i].live[j] = std::max(0.f, std::min(1.f, seed[a].live[j] + weight * (seed[b].live[j] - seed[c].live[j])));
				else
					result[i].live[j] = seed[i].live[j];
			}

			break;
		}
	}

	#pragma omp parallel for
	for (size_t i = 0; i < seed.size(); ++i)
	{
		result[i].fitness = fitness_score(result[i], meshes);
	}

	State best = {};
	float bestfit = 0;

	for (size_t i = 0; i < seed.size(); ++i)
	{
		if (result[i].fitness > seed[i].fitness)
			seed[i] = result[i];

		if (seed[i].fitness > bestfit)
		{
			best = seed[i];
			bestfit = seed[i].fitness;
		}
	}

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

void dump_stats(const State& state, const std::vector<Mesh>& meshes)
{
	float improvement[Profile_Count] = {};

	for (size_t i = 0; i < meshes.size(); ++i)
	{
		float metric[Profile_Count];
		compute_metric(&state, meshes[i], metric);

		printf(" %s", meshes[i].name);
		for (int profile = 0; profile < Profile_Count; ++profile)
			printf(" %f", metric[profile]);

		for (int profile = 0; profile < Profile_Count; ++profile)
			improvement[profile] += meshes[i].metric_base[profile] / metric[profile];
	}

	printf("; improvement");
	for (int profile = 0; profile < Profile_Count; ++profile)
		printf(" %f", improvement[profile] / float(meshes.size()));

	printf("\n");
}

int main(int argc, char** argv)
{
	meshopt_encodeIndexVersion(1);

	std::vector<Mesh> meshes;

	meshes.push_back(gridmesh(50));

	for (int i = 1; i < argc; ++i)
		meshes.push_back(objmesh(argv[i]));

	size_t total_triangles = 0;

	for (auto& mesh : meshes)
	{
		compute_metric(nullptr, mesh, mesh.metric_base);

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
		pop = gen0(95, meshes);
	}

	printf("%d meshes, %.1fM triangles\n", int(meshes.size()), double(total_triangles) / 1e6);

	for (;;)
	{
		auto best = genN(pop, meshes);
		gen++;

		if (gen % 10 == 0)
		{
			printf("%d: fitness %f;", int(gen), best.second);
			dump_stats(best.first, meshes);
		}
		else
		{
			printf("%d: fitness %f\n", int(gen), best.second);
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
