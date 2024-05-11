#include "../src/meshoptimizer.h"

#include <vector>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

double timestamp()
{
	return emscripten_get_now() * 1e-3;
}
#elif defined(_WIN32)
struct LARGE_INTEGER
{
	__int64 QuadPart;
};
extern "C" __declspec(dllimport) int __stdcall QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);
extern "C" __declspec(dllimport) int __stdcall QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);

double timestamp()
{
	LARGE_INTEGER freq, counter;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&counter);
	return double(counter.QuadPart) / double(freq.QuadPart);
}
#else
double timestamp()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return double(ts.tv_sec) + 1e-9 * double(ts.tv_nsec);
}
#endif

struct Vertex
{
	uint16_t data[16];
};

uint32_t murmur3(uint32_t h)
{
	h ^= h >> 16;
	h *= 0x85ebca6bu;
	h ^= h >> 13;
	h *= 0xc2b2ae35u;
	h ^= h >> 16;

	return h;
}

void benchCodecs(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, double& bestvd, double& bestid, bool verbose)
{
	std::vector<Vertex> vb(vertices.size());
	std::vector<unsigned int> ib(indices.size());

	std::vector<unsigned char> vc(meshopt_encodeVertexBufferBound(vertices.size(), sizeof(Vertex)));
	std::vector<unsigned char> ic(meshopt_encodeIndexBufferBound(indices.size(), vertices.size()));

	if (verbose)
		printf("source: vertex data %d bytes, index data %d bytes\n", int(vertices.size() * sizeof(Vertex)), int(indices.size() * 4));

	for (int pass = 0; pass < (verbose ? 2 : 1); ++pass)
	{
		if (pass == 1)
			meshopt_optimizeVertexCacheStrip(&ib[0], &indices[0], indices.size(), vertices.size());
		else
			meshopt_optimizeVertexCache(&ib[0], &indices[0], indices.size(), vertices.size());

		meshopt_optimizeVertexFetch(&vb[0], &ib[0], indices.size(), &vertices[0], vertices.size(), sizeof(Vertex));

		vc.resize(vc.capacity());
		vc.resize(meshopt_encodeVertexBuffer(&vc[0], vc.size(), &vb[0], vertices.size(), sizeof(Vertex)));

		ic.resize(ic.capacity());
		ic.resize(meshopt_encodeIndexBuffer(&ic[0], ic.size(), &ib[0], indices.size()));

		if (verbose)
			printf("pass %d: vertex data %d bytes, index data %d bytes\n", pass, int(vc.size()), int(ic.size()));

		for (int attempt = 0; attempt < 10; ++attempt)
		{
			double t0 = timestamp();

			int rv = meshopt_decodeVertexBuffer(&vb[0], vertices.size(), sizeof(Vertex), &vc[0], vc.size());
			assert(rv == 0);
			(void)rv;

			double t1 = timestamp();

			int ri = meshopt_decodeIndexBuffer(&ib[0], indices.size(), 4, &ic[0], ic.size());
			assert(ri == 0);
			(void)ri;

			double t2 = timestamp();

			double GB = 1024 * 1024 * 1024;

			if (verbose)
				printf("decode: vertex %.2f ms (%.2f GB/sec), index %.2f ms (%.2f GB/sec)\n",
				       (t1 - t0) * 1000, double(vertices.size() * sizeof(Vertex)) / GB / (t1 - t0),
				       (t2 - t1) * 1000, double(indices.size() * 4) / GB / (t2 - t1));

			if (pass == 0)
			{
				bestvd = std::max(bestvd, double(vertices.size() * sizeof(Vertex)) / GB / (t1 - t0));
				bestid = std::max(bestid, double(indices.size() * 4) / GB / (t2 - t1));
			}
		}
	}
}

void benchFilters(size_t count, double& besto8, double& besto12, double& bestq12, double& bestexp, bool verbose)
{
	// note: the filters are branchless so we just run them on runs of zeroes
	size_t count4 = (count + 3) & ~3;
	std::vector<unsigned char> d4(count4 * 4);
	std::vector<unsigned char> d8(count4 * 8);

	if (verbose)
		printf("filters: oct8 data %d bytes, oct12/quat12 data %d bytes\n", int(d4.size()), int(d8.size()));

	for (int attempt = 0; attempt < 10; ++attempt)
	{
		double t0 = timestamp();

		meshopt_decodeFilterOct(&d4[0], count4, 4);

		double t1 = timestamp();

		meshopt_decodeFilterOct(&d8[0], count4, 8);

		double t2 = timestamp();

		meshopt_decodeFilterQuat(&d8[0], count4, 8);

		double t3 = timestamp();

		meshopt_decodeFilterExp(&d8[0], count4, 8);

		double t4 = timestamp();

		double GB = 1024 * 1024 * 1024;

		if (verbose)
			printf("filter: oct8 %.2f ms (%.2f GB/sec), oct12 %.2f ms (%.2f GB/sec), quat12 %.2f ms (%.2f GB/sec), exp %.2f ms (%.2f GB/sec)\n",
			       (t1 - t0) * 1000, double(d4.size()) / GB / (t1 - t0),
			       (t2 - t1) * 1000, double(d8.size()) / GB / (t2 - t1),
			       (t3 - t2) * 1000, double(d8.size()) / GB / (t3 - t2),
			       (t4 - t3) * 1000, double(d8.size()) / GB / (t4 - t3));

		besto8 = std::max(besto8, double(d4.size()) / GB / (t1 - t0));
		besto12 = std::max(besto12, double(d8.size()) / GB / (t2 - t1));
		bestq12 = std::max(bestq12, double(d8.size()) / GB / (t3 - t2));
		bestexp = std::max(bestexp, double(d8.size()) / GB / (t4 - t3));
	}
}

int main(int argc, char** argv)
{
	meshopt_encodeIndexVersion(1);

	bool verbose = false;

	for (int i = 1; i < argc; ++i)
		if (strcmp(argv[i], "-v") == 0)
			verbose = true;

	const int N = 1000;

	std::vector<Vertex> vertices;
	vertices.reserve((N + 1) * (N + 1));

	for (int x = 0; x <= N; ++x)
	{
		for (int y = 0; y <= N; ++y)
		{
			Vertex v;

			for (int k = 0; k < 16; ++k)
			{
				uint32_t h = murmur3((x * (N + 1) + y) * 16 + k);

				// use random k-bit sequence for each word to test all encoding types
				// note: this doesn't stress the sentinel logic too much but it's all branchless so it's probably fine?
				v.data[k] = h & ((1 << (k + 1)) - 1);
			}

			vertices.push_back(v);
		}
	}

	std::vector<unsigned int> indices;
	indices.reserve(N * N * 6);

	for (int x = 0; x < N; ++x)
	{
		for (int y = 0; y < N; ++y)
		{
			indices.push_back((x + 0) * N + (y + 0));
			indices.push_back((x + 1) * N + (y + 0));
			indices.push_back((x + 0) * N + (y + 1));

			indices.push_back((x + 0) * N + (y + 1));
			indices.push_back((x + 1) * N + (y + 0));
			indices.push_back((x + 1) * N + (y + 1));
		}
	}

	double bestvd = 0, bestid = 0;
	benchCodecs(vertices, indices, bestvd, bestid, verbose);

	double besto8 = 0, besto12 = 0, bestq12 = 0, bestexp = 0;
	benchFilters(8 * N * N, besto8, besto12, bestq12, bestexp, verbose);

	printf("Algorithm   :\tvtx\tidx\toct8\toct12\tquat12\texp\n");
	printf("Score (GB/s):\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",
	       bestvd, bestid, besto8, besto12, bestq12, bestexp);
}
