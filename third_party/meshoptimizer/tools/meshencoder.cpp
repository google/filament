#include "../demo/objparser.h"
#include "../src/meshoptimizer.h"

#ifdef WITH_ZSTD
#include <zstd.h>
#endif

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <vector>

#if defined(__linux__)
double timestamp()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return double(ts.tv_sec) + 1e-9 * double(ts.tv_nsec);
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
	return double(clock()) / double(CLOCKS_PER_SEC);
}
#endif

struct Vertex
{
	float px, py, pz;
	float nx, ny, nz;
	float tx, ty;
};

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

Mesh parseObj(const char* path)
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

	result.vertices.resize(total_vertices);
	meshopt_remapVertexBuffer(&result.vertices[0], &vertices[0], total_indices, sizeof(Vertex), &remap[0]);

	return result;
}

struct PackedVertexOct
{
	unsigned short px, py, pz;
	unsigned char nu, nv; // octahedron encoded normal, aliases .pw
	unsigned short tx, ty;
};

void packMesh(std::vector<PackedVertexOct>& pv, const std::vector<Vertex>& vertices, int bitsp, int bitst)
{
	float minp[3] = {+FLT_MAX, +FLT_MAX, +FLT_MAX};
	float maxp[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
	float mint[2] = {+FLT_MAX, +FLT_MAX};
	float maxt[2] = {-FLT_MAX, -FLT_MAX};

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		minp[0] = std::min(minp[0], vertices[i].px);
		minp[1] = std::min(minp[1], vertices[i].py);
		minp[2] = std::min(minp[2], vertices[i].pz);
		mint[0] = std::min(mint[0], vertices[i].tx);
		mint[1] = std::min(mint[1], vertices[i].ty);

		maxp[0] = std::max(maxp[0], vertices[i].px);
		maxp[1] = std::max(maxp[1], vertices[i].py);
		maxp[2] = std::max(maxp[2], vertices[i].pz);
		maxt[0] = std::max(maxt[0], vertices[i].tx);
		maxt[1] = std::max(maxt[1], vertices[i].ty);
	}

	float scalep[3], scalet[2];

	scalep[0] = minp[0] == maxp[0] ? 0 : 1 / (maxp[0] - minp[0]);
	scalep[1] = minp[1] == maxp[1] ? 0 : 1 / (maxp[1] - minp[1]);
	scalep[2] = minp[2] == maxp[2] ? 0 : 1 / (maxp[2] - minp[2]);
	scalet[0] = mint[0] == maxt[0] ? 0 : 1 / (maxt[0] - mint[0]);
	scalet[1] = mint[1] == maxt[1] ? 0 : 1 / (maxt[1] - mint[1]);

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		pv[i].px = meshopt_quantizeUnorm((vertices[i].px - minp[0]) * scalep[0], bitsp);
		pv[i].py = meshopt_quantizeUnorm((vertices[i].px - minp[1]) * scalep[1], bitsp);
		pv[i].pz = meshopt_quantizeUnorm((vertices[i].px - minp[2]) * scalep[2], bitsp);

		float nsum = fabsf(vertices[i].nx) + fabsf(vertices[i].ny) + fabsf(vertices[i].nz);
		float nx = vertices[i].nx / nsum;
		float ny = vertices[i].ny / nsum;
		float nz = vertices[i].nz;

		float nu = nz >= 0 ? nx : (1 - fabsf(ny)) * (nx >= 0 ? 1 : -1);
		float nv = nz >= 0 ? ny : (1 - fabsf(nx)) * (ny >= 0 ? 1 : -1);

		pv[i].nu = char(meshopt_quantizeSnorm(nu, 8));
		pv[i].nv = char(meshopt_quantizeSnorm(nv, 8));

		pv[i].tx = meshopt_quantizeUnorm((vertices[i].tx - mint[0]) * scalet[0], bitst);
		pv[i].ty = meshopt_quantizeUnorm((vertices[i].ty - mint[1]) * scalet[1], bitst);
	}
}

#ifdef WITH_ZSTD
template <typename T>
std::vector<unsigned char> compress(const std::vector<T>& v)
{
	std::vector<unsigned char> result(ZSTD_compressBound(v.size() * sizeof(T)));
	result.resize(ZSTD_compress(&result[0], result.size(), &v[0], v.size() * sizeof(T), 9));
	return result;
}
#endif

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		printf("Usage: %s [.obj file]\n", argv[0]);
		return 1;
	}

	int bitsp = 14;
	int bitst = 12;

	for (int i = 1; i < argc; ++i)
	{
		const char* path = argv[i];

		Mesh mesh = parseObj(path);

		if (mesh.vertices.empty())
		{
			printf("Mesh %s is empty, skipping\n", path);
			continue;
		}

		printf("# %s: %d vertices, %d triangles\n", path, int(mesh.vertices.size()), int(mesh.indices.size() / 3));

		meshopt_optimizeVertexCache(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), mesh.vertices.size());
		meshopt_optimizeVertexFetch(&mesh.vertices[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0], mesh.vertices.size(), sizeof(Vertex));

		typedef PackedVertexOct PV;

		std::vector<PV> pv(mesh.vertices.size());
		packMesh(pv, mesh.vertices, bitsp, bitst);

		printf("baseline   : size: %d bytes; vb %.1f bpv, ib %.1f bpv\n",
		       int(pv.size() * sizeof(PV)) + int(mesh.indices.size() * sizeof(unsigned int)),
		       double(pv.size() * sizeof(PV) * 8) / double(pv.size()),
		       double(mesh.indices.size() * sizeof(unsigned int) * 8) / double(pv.size()));

		std::vector<PV> vbd(mesh.vertices.size());
		std::vector<unsigned int> ibd(mesh.indices.size());

#ifdef WITH_ZSTD
		{
			std::vector<unsigned char> vbz = compress(pv);
			std::vector<unsigned char> ibz = compress(mesh.indices);

			double start = timestamp();
			ZSTD_decompress(&vbd[0], vbd.size(), &vbz[0], vbz.size());
			ZSTD_decompress(&ibd[0], ibd.size() * sizeof(ibd[0]), &ibz[0], ibz.size());
			double end = timestamp();

			printf("zstd only  : size: %d bytes; vb %.1f bpv, ib %.1f bpv; decoding time: %.2f msec\n",
			       int(vbz.size() + ibz.size()),
			       double(vbz.size() * 8) / double(pv.size()),
			       double(ibz.size() * 8) / double(pv.size()),
			       (end - start) * 1000);
		}
#endif

		std::vector<unsigned char> vbuf(meshopt_encodeVertexBufferBound(mesh.vertices.size(), sizeof(PV)));
		vbuf.resize(meshopt_encodeVertexBuffer(&vbuf[0], vbuf.size(), &pv[0], mesh.vertices.size(), sizeof(PV)));

		std::vector<unsigned char> ibuf(meshopt_encodeIndexBufferBound(mesh.indices.size(), mesh.vertices.size()));
		ibuf.resize(meshopt_encodeIndexBuffer(&ibuf[0], ibuf.size(), &mesh.indices[0], mesh.indices.size()));

		{
			double start = timestamp();
			int dvb = meshopt_decodeVertexBuffer(&vbd[0], vbd.size(), sizeof(PV), &vbuf[0], vbuf.size());
			int dib = meshopt_decodeIndexBuffer(&ibd[0], ibd.size(), &ibuf[0], ibuf.size());
			assert(dvb == 0 && dib == 0);
			double end = timestamp();

			printf("codec      : size: %d bytes; vb %.1f bpv, ib %.1f bpv; decoding time: %.2f msec\n",
			       int(vbuf.size() + ibuf.size()),
			       double(vbuf.size() * 8) / double(pv.size()),
			       double(ibuf.size() * 8) / double(pv.size()),
			       (end - start) * 1000);
		}

#ifdef WITH_ZSTD
		{
			std::vector<unsigned char> vbz = compress(vbuf);
			std::vector<unsigned char> ibz = compress(ibuf);

			std::vector<unsigned char> scratch(std::max(vbuf.size(), ibuf.size()));

			double start = timestamp();
			ZSTD_decompress(&scratch[0], scratch.size(), &vbz[0], vbz.size());
			int dvbz = meshopt_decodeVertexBuffer(&vbd[0], vbd.size(), sizeof(PV), &scratch[0], vbuf.size());
			ZSTD_decompress(&scratch[0], scratch.size(), &ibz[0], ibz.size());
			int dibz = meshopt_decodeIndexBuffer(&ibd[0], ibd.size(), &scratch[0], ibuf.size());
			assert(dvbz == 0 && dibz == 0);
			double end = timestamp();

			printf("codec+zstd : size: %d bytes; vb %.1f bpv, ib %.1f bpv; decoding time: %.2f msec\n",
			       int(vbz.size() + ibz.size()),
			       double(vbz.size() * 8) / double(pv.size()),
			       double(ibz.size() * 8) / double(pv.size()),
			       (end - start) * 1000);
		}
#endif
	}
}
