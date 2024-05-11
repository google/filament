#include "../src/meshoptimizer.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <vector>

#include "../extern/fast_obj.h"

#define SDEFL_IMPLEMENTATION
#include "../extern/sdefl.h"

// This file uses assert() to verify algorithm correctness
#undef NDEBUG
#include <assert.h>

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

const size_t kCacheSize = 16;

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

union Triangle
{
	Vertex v[3];
	char data[sizeof(Vertex) * 3];
};

Mesh parseObj(const char* path, double& reindex)
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

	reindex = timestamp();

	Mesh result;

	std::vector<unsigned int> remap(total_indices);

	size_t total_vertices = meshopt_generateVertexRemap(&remap[0], NULL, total_indices, &vertices[0], total_indices, sizeof(Vertex));

	result.indices.resize(total_indices);
	meshopt_remapIndexBuffer(&result.indices[0], NULL, total_indices, &remap[0]);

	result.vertices.resize(total_vertices);
	meshopt_remapVertexBuffer(&result.vertices[0], &vertices[0], total_indices, sizeof(Vertex), &remap[0]);

	return result;
}

void dumpObj(const Mesh& mesh, bool recomputeNormals = false)
{
	std::vector<float> normals;

	if (recomputeNormals)
	{
		normals.resize(mesh.vertices.size() * 3);

		for (size_t i = 0; i < mesh.indices.size(); i += 3)
		{
			unsigned int a = mesh.indices[i], b = mesh.indices[i + 1], c = mesh.indices[i + 2];

			const Vertex& va = mesh.vertices[a];
			const Vertex& vb = mesh.vertices[b];
			const Vertex& vc = mesh.vertices[c];

			float nx = (vb.py - va.py) * (vc.pz - va.pz) - (vb.pz - va.pz) * (vc.py - va.py);
			float ny = (vb.pz - va.pz) * (vc.px - va.px) - (vb.px - va.px) * (vc.pz - va.pz);
			float nz = (vb.px - va.px) * (vc.py - va.py) - (vb.py - va.py) * (vc.px - va.px);

			for (int k = 0; k < 3; ++k)
			{
				unsigned int index = mesh.indices[i + k];

				normals[index * 3 + 0] += nx;
				normals[index * 3 + 1] += ny;
				normals[index * 3 + 2] += nz;
			}
		}
	}

	for (size_t i = 0; i < mesh.vertices.size(); ++i)
	{
		const Vertex& v = mesh.vertices[i];

		float nx = v.nx, ny = v.ny, nz = v.nz;

		if (recomputeNormals)
		{
			nx = normals[i * 3 + 0];
			ny = normals[i * 3 + 1];
			nz = normals[i * 3 + 2];

			float l = sqrtf(nx * nx + ny * ny + nz * nz);
			float s = l == 0.f ? 0.f : 1.f / l;

			nx *= s;
			ny *= s;
			nz *= s;
		}

		fprintf(stderr, "v %f %f %f\n", v.px, v.py, v.pz);
		fprintf(stderr, "vn %f %f %f\n", nx, ny, nz);
	}

	for (size_t i = 0; i < mesh.indices.size(); i += 3)
	{
		unsigned int a = mesh.indices[i], b = mesh.indices[i + 1], c = mesh.indices[i + 2];

		fprintf(stderr, "f %d %d %d\n", a + 1, b + 1, c + 1);
	}
}

bool isMeshValid(const Mesh& mesh)
{
	size_t index_count = mesh.indices.size();
	size_t vertex_count = mesh.vertices.size();

	if (index_count % 3 != 0)
		return false;

	const unsigned int* indices = &mesh.indices[0];

	for (size_t i = 0; i < index_count; ++i)
		if (indices[i] >= vertex_count)
			return false;

	return true;
}

bool rotateTriangle(Triangle& t)
{
	int c01 = memcmp(&t.v[0], &t.v[1], sizeof(Vertex));
	int c02 = memcmp(&t.v[0], &t.v[2], sizeof(Vertex));
	int c12 = memcmp(&t.v[1], &t.v[2], sizeof(Vertex));

	if (c12 < 0 && c01 > 0)
	{
		// 1 is minimum, rotate 012 => 120
		Vertex tv = t.v[0];
		t.v[0] = t.v[1], t.v[1] = t.v[2], t.v[2] = tv;
	}
	else if (c02 > 0 && c12 > 0)
	{
		// 2 is minimum, rotate 012 => 201
		Vertex tv = t.v[2];
		t.v[2] = t.v[1], t.v[1] = t.v[0], t.v[0] = tv;
	}

	return c01 != 0 && c02 != 0 && c12 != 0;
}

unsigned int hashRange(const char* key, size_t len)
{
	// MurmurHash2
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h = 0;

	while (len >= 4)
	{
		unsigned int k = *reinterpret_cast<const unsigned int*>(key);

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		key += 4;
		len -= 4;
	}

	return h;
}

unsigned int hashMesh(const Mesh& mesh)
{
	size_t triangle_count = mesh.indices.size() / 3;

	const Vertex* vertices = &mesh.vertices[0];
	const unsigned int* indices = &mesh.indices[0];

	unsigned int h1 = 0;
	unsigned int h2 = 0;

	for (size_t i = 0; i < triangle_count; ++i)
	{
		Triangle t;
		t.v[0] = vertices[indices[i * 3 + 0]];
		t.v[1] = vertices[indices[i * 3 + 1]];
		t.v[2] = vertices[indices[i * 3 + 2]];

		// skip degenerate triangles since some algorithms don't preserve them
		if (rotateTriangle(t))
		{
			unsigned int hash = hashRange(t.data, sizeof(t.data));

			h1 ^= hash;
			h2 += hash;
		}
	}

	return h1 * 0x5bd1e995 + h2;
}

void optNone(Mesh& mesh)
{
	(void)mesh;
}

void optRandomShuffle(Mesh& mesh)
{
	size_t triangle_count = mesh.indices.size() / 3;

	unsigned int* indices = &mesh.indices[0];

	unsigned int rng = 0;

	for (size_t i = triangle_count - 1; i > 0; --i)
	{
		// Fisher-Yates shuffle
		size_t j = rng % (i + 1);

		unsigned int t;
		t = indices[3 * j + 0], indices[3 * j + 0] = indices[3 * i + 0], indices[3 * i + 0] = t;
		t = indices[3 * j + 1], indices[3 * j + 1] = indices[3 * i + 1], indices[3 * i + 1] = t;
		t = indices[3 * j + 2], indices[3 * j + 2] = indices[3 * i + 2], indices[3 * i + 2] = t;

		// LCG RNG, constants from Numerical Recipes
		rng = rng * 1664525 + 1013904223;
	}
}

void optCache(Mesh& mesh)
{
	meshopt_optimizeVertexCache(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), mesh.vertices.size());
}

void optCacheFifo(Mesh& mesh)
{
	meshopt_optimizeVertexCacheFifo(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), mesh.vertices.size(), kCacheSize);
}

void optCacheStrip(Mesh& mesh)
{
	meshopt_optimizeVertexCacheStrip(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), mesh.vertices.size());
}

void optOverdraw(Mesh& mesh)
{
	// use worst-case ACMR threshold so that overdraw optimizer can sort *all* triangles
	// warning: this significantly deteriorates the vertex cache efficiency so it is not advised; look at optComplete for the recommended method
	const float kThreshold = 3.f;
	meshopt_optimizeOverdraw(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex), kThreshold);
}

void optFetch(Mesh& mesh)
{
	meshopt_optimizeVertexFetch(&mesh.vertices[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0], mesh.vertices.size(), sizeof(Vertex));
}

void optFetchRemap(Mesh& mesh)
{
	// this produces results equivalent to optFetch, but can be used to remap multiple vertex streams
	std::vector<unsigned int> remap(mesh.vertices.size());
	meshopt_optimizeVertexFetchRemap(&remap[0], &mesh.indices[0], mesh.indices.size(), mesh.vertices.size());

	meshopt_remapIndexBuffer(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), &remap[0]);
	meshopt_remapVertexBuffer(&mesh.vertices[0], &mesh.vertices[0], mesh.vertices.size(), sizeof(Vertex), &remap[0]);
}

void optComplete(Mesh& mesh)
{
	// vertex cache optimization should go first as it provides starting order for overdraw
	meshopt_optimizeVertexCache(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), mesh.vertices.size());

	// reorder indices for overdraw, balancing overdraw and vertex cache efficiency
	const float kThreshold = 1.01f; // allow up to 1% worse ACMR to get more reordering opportunities for overdraw
	meshopt_optimizeOverdraw(&mesh.indices[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex), kThreshold);

	// vertex fetch optimization should go last as it depends on the final index order
	meshopt_optimizeVertexFetch(&mesh.vertices[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0], mesh.vertices.size(), sizeof(Vertex));
}

struct PackedVertex
{
	unsigned short px, py, pz;
	unsigned short pw; // padding to 4b boundary
	signed char nx, ny, nz, nw;
	unsigned short tx, ty;
};

void packMesh(std::vector<PackedVertex>& pv, const std::vector<Vertex>& vertices)
{
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		const Vertex& vi = vertices[i];
		PackedVertex& pvi = pv[i];

		pvi.px = meshopt_quantizeHalf(vi.px);
		pvi.py = meshopt_quantizeHalf(vi.py);
		pvi.pz = meshopt_quantizeHalf(vi.pz);
		pvi.pw = 0;

		pvi.nx = char(meshopt_quantizeSnorm(vi.nx, 8));
		pvi.ny = char(meshopt_quantizeSnorm(vi.ny, 8));
		pvi.nz = char(meshopt_quantizeSnorm(vi.nz, 8));
		pvi.nw = 0;

		pvi.tx = meshopt_quantizeHalf(vi.tx);
		pvi.ty = meshopt_quantizeHalf(vi.ty);
	}
}

struct PackedVertexOct
{
	unsigned short px, py, pz;
	signed char nu, nv; // octahedron encoded normal, aliases .pw
	unsigned short tx, ty;
};

void packMesh(std::vector<PackedVertexOct>& pv, const std::vector<Vertex>& vertices)
{
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		const Vertex& vi = vertices[i];
		PackedVertexOct& pvi = pv[i];

		pvi.px = meshopt_quantizeHalf(vi.px);
		pvi.py = meshopt_quantizeHalf(vi.py);
		pvi.pz = meshopt_quantizeHalf(vi.pz);

		float nsum = fabsf(vi.nx) + fabsf(vi.ny) + fabsf(vi.nz);
		float nx = vi.nx / nsum;
		float ny = vi.ny / nsum;
		float nz = vi.nz;

		float nu = nz >= 0 ? nx : (1 - fabsf(ny)) * (nx >= 0 ? 1 : -1);
		float nv = nz >= 0 ? ny : (1 - fabsf(nx)) * (ny >= 0 ? 1 : -1);

		pvi.nu = char(meshopt_quantizeSnorm(nu, 8));
		pvi.nv = char(meshopt_quantizeSnorm(nv, 8));

		pvi.tx = meshopt_quantizeHalf(vi.tx);
		pvi.ty = meshopt_quantizeHalf(vi.ty);
	}
}

void simplify(const Mesh& mesh, float threshold = 0.2f)
{
	Mesh lod;

	double start = timestamp();

	size_t target_index_count = size_t(mesh.indices.size() * threshold);
	float target_error = 1e-2f;
	float result_error = 0;

	lod.indices.resize(mesh.indices.size()); // note: simplify needs space for index_count elements in the destination array, not target_index_count
	lod.indices.resize(meshopt_simplify(&lod.indices[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex), target_index_count, target_error, 0, &result_error));

	lod.vertices.resize(lod.indices.size() < mesh.vertices.size() ? lod.indices.size() : mesh.vertices.size()); // note: this is just to reduce the cost of resize()
	lod.vertices.resize(meshopt_optimizeVertexFetch(&lod.vertices[0], &lod.indices[0], lod.indices.size(), &mesh.vertices[0], mesh.vertices.size(), sizeof(Vertex)));

	double end = timestamp();

	printf("%-9s: %d triangles => %d triangles (%.2f%% deviation) in %.2f msec\n",
	    "Simplify",
	    int(mesh.indices.size() / 3), int(lod.indices.size() / 3),
	    result_error * 100,
	    (end - start) * 1000);
}

void simplifySloppy(const Mesh& mesh, float threshold = 0.2f)
{
	Mesh lod;

	double start = timestamp();

	size_t target_index_count = size_t(mesh.indices.size() * threshold);
	float target_error = 1e-1f;
	float result_error = 0;

	lod.indices.resize(mesh.indices.size()); // note: simplify needs space for index_count elements in the destination array, not target_index_count
	lod.indices.resize(meshopt_simplifySloppy(&lod.indices[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex), target_index_count, target_error, &result_error));

	lod.vertices.resize(lod.indices.size() < mesh.vertices.size() ? lod.indices.size() : mesh.vertices.size()); // note: this is just to reduce the cost of resize()
	lod.vertices.resize(meshopt_optimizeVertexFetch(&lod.vertices[0], &lod.indices[0], lod.indices.size(), &mesh.vertices[0], mesh.vertices.size(), sizeof(Vertex)));

	double end = timestamp();

	printf("%-9s: %d triangles => %d triangles (%.2f%% deviation) in %.2f msec\n",
	    "SimplifyS",
	    int(mesh.indices.size() / 3), int(lod.indices.size() / 3),
	    result_error * 100,
	    (end - start) * 1000);
}

void simplifyPoints(const Mesh& mesh, float threshold = 0.2f)
{
	double start = timestamp();

	size_t target_vertex_count = size_t(mesh.vertices.size() * threshold);

	std::vector<unsigned int> indices(target_vertex_count);
	indices.resize(meshopt_simplifyPoints(&indices[0], &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex), target_vertex_count));

	double end = timestamp();

	printf("%-9s: %d points => %d points in %.2f msec\n",
	    "SimplifyP",
	    int(mesh.vertices.size()), int(indices.size()), (end - start) * 1000);
}

void simplifyComplete(const Mesh& mesh)
{
	static const size_t lod_count = 5;

	double start = timestamp();

	// generate 4 LOD levels (1-4), with each subsequent LOD using 70% triangles
	// note that each LOD uses the same (shared) vertex buffer
	std::vector<unsigned int> lods[lod_count];

	lods[0] = mesh.indices;

	for (size_t i = 1; i < lod_count; ++i)
	{
		std::vector<unsigned int>& lod = lods[i];

		float threshold = powf(0.7f, float(i));
		size_t target_index_count = size_t(mesh.indices.size() * threshold) / 3 * 3;
		float target_error = 1e-2f;

		// we can simplify all the way from base level or from the last result
		// simplifying from the base level sometimes produces better results, but simplifying from last level is faster
		const std::vector<unsigned int>& source = lods[i - 1];

		if (source.size() < target_index_count)
			target_index_count = source.size();

		lod.resize(source.size());
		lod.resize(meshopt_simplify(&lod[0], &source[0], source.size(), &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex), target_index_count, target_error));
	}

	double middle = timestamp();

	// optimize each individual LOD for vertex cache & overdraw
	for (size_t i = 0; i < lod_count; ++i)
	{
		std::vector<unsigned int>& lod = lods[i];

		meshopt_optimizeVertexCache(&lod[0], &lod[0], lod.size(), mesh.vertices.size());
		meshopt_optimizeOverdraw(&lod[0], &lod[0], lod.size(), &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex), 1.0f);
	}

	// concatenate all LODs into one IB
	// note: the order of concatenation is important - since we optimize the entire IB for vertex fetch,
	// putting coarse LODs first makes sure that the vertex range referenced by them is as small as possible
	// some GPUs process the entire range referenced by the index buffer region so doing this optimizes the vertex transform
	// cost for coarse LODs
	// this order also produces much better vertex fetch cache coherency for coarse LODs (since they're essentially optimized first)
	// somewhat surprisingly, the vertex fetch cache coherency for fine LODs doesn't seem to suffer that much.
	size_t lod_index_offsets[lod_count] = {};
	size_t lod_index_counts[lod_count] = {};
	size_t total_index_count = 0;

	for (int i = lod_count - 1; i >= 0; --i)
	{
		lod_index_offsets[i] = total_index_count;
		lod_index_counts[i] = lods[i].size();

		total_index_count += lods[i].size();
	}

	std::vector<unsigned int> indices(total_index_count);

	for (size_t i = 0; i < lod_count; ++i)
	{
		memcpy(&indices[lod_index_offsets[i]], &lods[i][0], lods[i].size() * sizeof(lods[i][0]));
	}

	std::vector<Vertex> vertices = mesh.vertices;

	// vertex fetch optimization should go last as it depends on the final index order
	// note that the order of LODs above affects vertex fetch results
	meshopt_optimizeVertexFetch(&vertices[0], &indices[0], indices.size(), &vertices[0], vertices.size(), sizeof(Vertex));

	double end = timestamp();

	printf("%-9s: %d triangles => %d LOD levels down to %d triangles in %.2f msec, optimized in %.2f msec\n",
	    "SimplifyC",
	    int(lod_index_counts[0]) / 3, int(lod_count), int(lod_index_counts[lod_count - 1]) / 3,
	    (middle - start) * 1000, (end - middle) * 1000);

	// for using LOD data at runtime, in addition to vertices and indices you have to save lod_index_offsets/lod_index_counts.

	{
		meshopt_VertexCacheStatistics vcs0 = meshopt_analyzeVertexCache(&indices[lod_index_offsets[0]], lod_index_counts[0], vertices.size(), kCacheSize, 0, 0);
		meshopt_VertexFetchStatistics vfs0 = meshopt_analyzeVertexFetch(&indices[lod_index_offsets[0]], lod_index_counts[0], vertices.size(), sizeof(Vertex));
		meshopt_VertexCacheStatistics vcsN = meshopt_analyzeVertexCache(&indices[lod_index_offsets[lod_count - 1]], lod_index_counts[lod_count - 1], vertices.size(), kCacheSize, 0, 0);
		meshopt_VertexFetchStatistics vfsN = meshopt_analyzeVertexFetch(&indices[lod_index_offsets[lod_count - 1]], lod_index_counts[lod_count - 1], vertices.size(), sizeof(Vertex));

		typedef PackedVertexOct PV;

		std::vector<PV> pv(vertices.size());
		packMesh(pv, vertices);

		std::vector<unsigned char> vbuf(meshopt_encodeVertexBufferBound(vertices.size(), sizeof(PV)));
		vbuf.resize(meshopt_encodeVertexBuffer(&vbuf[0], vbuf.size(), &pv[0], vertices.size(), sizeof(PV)));

		std::vector<unsigned char> ibuf(meshopt_encodeIndexBufferBound(indices.size(), vertices.size()));
		ibuf.resize(meshopt_encodeIndexBuffer(&ibuf[0], ibuf.size(), &indices[0], indices.size()));

		printf("%-9s  ACMR %f...%f Overfetch %f..%f Codec VB %.1f bits/vertex IB %.1f bits/triangle\n",
		    "",
		    vcs0.acmr, vcsN.acmr, vfs0.overfetch, vfsN.overfetch,
		    double(vbuf.size()) / double(vertices.size()) * 8,
		    double(ibuf.size()) / double(indices.size() / 3) * 8);
	}
}

void optimize(const Mesh& mesh, const char* name, void (*optf)(Mesh& mesh))
{
	Mesh copy = mesh;

	double start = timestamp();
	optf(copy);
	double end = timestamp();

	assert(isMeshValid(copy));
	assert(hashMesh(mesh) == hashMesh(copy));

	meshopt_VertexCacheStatistics vcs = meshopt_analyzeVertexCache(&copy.indices[0], copy.indices.size(), copy.vertices.size(), kCacheSize, 0, 0);
	meshopt_VertexFetchStatistics vfs = meshopt_analyzeVertexFetch(&copy.indices[0], copy.indices.size(), copy.vertices.size(), sizeof(Vertex));
	meshopt_OverdrawStatistics os = meshopt_analyzeOverdraw(&copy.indices[0], copy.indices.size(), &copy.vertices[0].px, copy.vertices.size(), sizeof(Vertex));

	meshopt_VertexCacheStatistics vcs_nv = meshopt_analyzeVertexCache(&copy.indices[0], copy.indices.size(), copy.vertices.size(), 32, 32, 32);
	meshopt_VertexCacheStatistics vcs_amd = meshopt_analyzeVertexCache(&copy.indices[0], copy.indices.size(), copy.vertices.size(), 14, 64, 128);
	meshopt_VertexCacheStatistics vcs_intel = meshopt_analyzeVertexCache(&copy.indices[0], copy.indices.size(), copy.vertices.size(), 128, 0, 0);

	printf("%-9s: ACMR %f ATVR %f (NV %f AMD %f Intel %f) Overfetch %f Overdraw %f in %.2f msec\n", name, vcs.acmr, vcs.atvr, vcs_nv.atvr, vcs_amd.atvr, vcs_intel.atvr, vfs.overfetch, os.overdraw, (end - start) * 1000);
}

template <typename T>
size_t compress(const std::vector<T>& data, int level = SDEFL_LVL_DEF)
{
	std::vector<unsigned char> cbuf(sdefl_bound(int(data.size() * sizeof(T))));
	sdefl s = {};
	return sdeflate(&s, &cbuf[0], reinterpret_cast<const unsigned char*>(&data[0]), int(data.size() * sizeof(T)), level);
}

void encodeIndex(const Mesh& mesh, char desc)
{
	// allocate result outside of the timing loop to exclude memset() from decode timing
	std::vector<unsigned int> result(mesh.indices.size());

	double start = timestamp();

	std::vector<unsigned char> buffer(meshopt_encodeIndexBufferBound(mesh.indices.size(), mesh.vertices.size()));
	buffer.resize(meshopt_encodeIndexBuffer(&buffer[0], buffer.size(), &mesh.indices[0], mesh.indices.size()));

	double middle = timestamp();

	int res = meshopt_decodeIndexBuffer(&result[0], mesh.indices.size(), &buffer[0], buffer.size());
	assert(res == 0);
	(void)res;

	double end = timestamp();

	size_t csize = compress(buffer);

	for (size_t i = 0; i < mesh.indices.size(); i += 3)
	{
		assert(
		    (result[i + 0] == mesh.indices[i + 0] && result[i + 1] == mesh.indices[i + 1] && result[i + 2] == mesh.indices[i + 2]) ||
		    (result[i + 1] == mesh.indices[i + 0] && result[i + 2] == mesh.indices[i + 1] && result[i + 0] == mesh.indices[i + 2]) ||
		    (result[i + 2] == mesh.indices[i + 0] && result[i + 0] == mesh.indices[i + 1] && result[i + 1] == mesh.indices[i + 2]));
	}

	printf("IdxCodec%c: %.1f bits/triangle (post-deflate %.1f bits/triangle); encode %.2f msec, decode %.2f msec (%.2f GB/s)\n",
	    desc,
	    double(buffer.size() * 8) / double(mesh.indices.size() / 3),
	    double(csize * 8) / double(mesh.indices.size() / 3),
	    (middle - start) * 1000,
	    (end - middle) * 1000,
	    (double(result.size() * 4) / (1 << 30)) / (end - middle));
}

void encodeIndexSequence(const std::vector<unsigned int>& data, size_t vertex_count, char desc)
{
	// allocate result outside of the timing loop to exclude memset() from decode timing
	std::vector<unsigned int> result(data.size());

	double start = timestamp();

	std::vector<unsigned char> buffer(meshopt_encodeIndexSequenceBound(data.size(), vertex_count));
	buffer.resize(meshopt_encodeIndexSequence(&buffer[0], buffer.size(), &data[0], data.size()));

	double middle = timestamp();

	int res = meshopt_decodeIndexSequence(&result[0], data.size(), &buffer[0], buffer.size());
	assert(res == 0);
	(void)res;

	double end = timestamp();

	size_t csize = compress(buffer);

	assert(memcmp(&data[0], &result[0], data.size() * sizeof(unsigned int)) == 0);

	printf("IdxCodec%c: %.1f bits/index (post-deflate %.1f bits/index); encode %.2f msec, decode %.2f msec (%.2f GB/s)\n",
	    desc,
	    double(buffer.size() * 8) / double(data.size()),
	    double(csize * 8) / double(data.size()),
	    (middle - start) * 1000,
	    (end - middle) * 1000,
	    (double(result.size() * 4) / (1 << 30)) / (end - middle));
}

template <typename PV>
void packVertex(const Mesh& mesh, const char* pvn)
{
	std::vector<PV> pv(mesh.vertices.size());
	packMesh(pv, mesh.vertices);

	size_t csize = compress(pv);

	printf("VtxPack%s  : %.1f bits/vertex (post-deflate %.1f bits/vertex)\n", pvn,
	    double(pv.size() * sizeof(PV) * 8) / double(mesh.vertices.size()),
	    double(csize * 8) / double(mesh.vertices.size()));
}

template <typename PV>
void encodeVertex(const Mesh& mesh, const char* pvn)
{
	std::vector<PV> pv(mesh.vertices.size());
	packMesh(pv, mesh.vertices);

	// allocate result outside of the timing loop to exclude memset() from decode timing
	std::vector<PV> result(mesh.vertices.size());

	double start = timestamp();

	std::vector<unsigned char> vbuf(meshopt_encodeVertexBufferBound(mesh.vertices.size(), sizeof(PV)));
	vbuf.resize(meshopt_encodeVertexBuffer(&vbuf[0], vbuf.size(), &pv[0], mesh.vertices.size(), sizeof(PV)));

	double middle = timestamp();

	int res = meshopt_decodeVertexBuffer(&result[0], mesh.vertices.size(), sizeof(PV), &vbuf[0], vbuf.size());
	assert(res == 0);
	(void)res;

	double end = timestamp();

	assert(memcmp(&pv[0], &result[0], pv.size() * sizeof(PV)) == 0);

	size_t csize = compress(vbuf);

	printf("VtxCodec%1s: %.1f bits/vertex (post-deflate %.1f bits/vertex); encode %.2f msec, decode %.2f msec (%.2f GB/s)\n", pvn,
	    double(vbuf.size() * 8) / double(mesh.vertices.size()),
	    double(csize * 8) / double(mesh.vertices.size()),
	    (middle - start) * 1000,
	    (end - middle) * 1000,
	    (double(result.size() * sizeof(PV)) / (1 << 30)) / (end - middle));
}

void stripify(const Mesh& mesh, bool use_restart, char desc)
{
	unsigned int restart_index = use_restart ? ~0u : 0;

	// note: input mesh is assumed to be optimized for vertex cache and vertex fetch
	double start = timestamp();
	std::vector<unsigned int> strip(meshopt_stripifyBound(mesh.indices.size()));
	strip.resize(meshopt_stripify(&strip[0], &mesh.indices[0], mesh.indices.size(), mesh.vertices.size(), restart_index));
	double end = timestamp();

	Mesh copy = mesh;
	copy.indices.resize(meshopt_unstripify(&copy.indices[0], &strip[0], strip.size(), restart_index));
	assert(copy.indices.size() <= meshopt_unstripifyBound(strip.size()));

	assert(isMeshValid(copy));
	assert(hashMesh(mesh) == hashMesh(copy));

	meshopt_VertexCacheStatistics vcs = meshopt_analyzeVertexCache(&copy.indices[0], mesh.indices.size(), mesh.vertices.size(), kCacheSize, 0, 0);
	meshopt_VertexCacheStatistics vcs_nv = meshopt_analyzeVertexCache(&copy.indices[0], mesh.indices.size(), mesh.vertices.size(), 32, 32, 32);
	meshopt_VertexCacheStatistics vcs_amd = meshopt_analyzeVertexCache(&copy.indices[0], mesh.indices.size(), mesh.vertices.size(), 14, 64, 128);
	meshopt_VertexCacheStatistics vcs_intel = meshopt_analyzeVertexCache(&copy.indices[0], mesh.indices.size(), mesh.vertices.size(), 128, 0, 0);

	printf("Stripify%c: ACMR %f ATVR %f (NV %f AMD %f Intel %f); %d strip indices (%.1f%%) in %.2f msec\n",
	    desc,
	    vcs.acmr, vcs.atvr, vcs_nv.atvr, vcs_amd.atvr, vcs_intel.atvr,
	    int(strip.size()), double(strip.size()) / double(mesh.indices.size()) * 100,
	    (end - start) * 1000);
}

void shadow(const Mesh& mesh)
{
	// note: input mesh is assumed to be optimized for vertex cache and vertex fetch

	double start = timestamp();
	// this index buffer can be used for position-only rendering using the same vertex data that the original index buffer uses
	std::vector<unsigned int> shadow_indices(mesh.indices.size());
	meshopt_generateShadowIndexBuffer(&shadow_indices[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0], mesh.vertices.size(), sizeof(float) * 3, sizeof(Vertex));
	double end = timestamp();

	// while you can't optimize the vertex data after shadow IB was constructed, you can and should optimize the shadow IB for vertex cache
	// this is valuable even if the original indices array was optimized for vertex cache!
	meshopt_optimizeVertexCache(&shadow_indices[0], &shadow_indices[0], shadow_indices.size(), mesh.vertices.size());

	meshopt_VertexCacheStatistics vcs = meshopt_analyzeVertexCache(&mesh.indices[0], mesh.indices.size(), mesh.vertices.size(), kCacheSize, 0, 0);
	meshopt_VertexCacheStatistics vcss = meshopt_analyzeVertexCache(&shadow_indices[0], shadow_indices.size(), mesh.vertices.size(), kCacheSize, 0, 0);

	std::vector<char> shadow_flags(mesh.vertices.size());
	size_t shadow_vertices = 0;

	for (size_t i = 0; i < shadow_indices.size(); ++i)
	{
		unsigned int index = shadow_indices[i];
		shadow_vertices += 1 - shadow_flags[index];
		shadow_flags[index] = 1;
	}

	printf("ShadowIB : ACMR %f (%.2fx improvement); %d shadow vertices (%.2fx improvement) in %.2f msec\n",
	    vcss.acmr, double(vcs.vertices_transformed) / double(vcss.vertices_transformed),
	    int(shadow_vertices), double(mesh.vertices.size()) / double(shadow_vertices),
	    (end - start) * 1000);
}

void meshlets(const Mesh& mesh, bool scan)
{
	const size_t max_vertices = 64;
	const size_t max_triangles = 124; // NVidia-recommended 126, rounded down to a multiple of 4
	const float cone_weight = 0.5f;   // note: should be set to 0 unless cone culling is used at runtime!

	// note: input mesh is assumed to be optimized for vertex cache and vertex fetch
	double start = timestamp();
	size_t max_meshlets = meshopt_buildMeshletsBound(mesh.indices.size(), max_vertices, max_triangles);
	std::vector<meshopt_Meshlet> meshlets(max_meshlets);
	std::vector<unsigned int> meshlet_vertices(max_meshlets * max_vertices);
	std::vector<unsigned char> meshlet_triangles(max_meshlets * max_triangles * 3);

	if (scan)
		meshlets.resize(meshopt_buildMeshletsScan(&meshlets[0], &meshlet_vertices[0], &meshlet_triangles[0], &mesh.indices[0], mesh.indices.size(), mesh.vertices.size(), max_vertices, max_triangles));
	else
		meshlets.resize(meshopt_buildMeshlets(&meshlets[0], &meshlet_vertices[0], &meshlet_triangles[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex), max_vertices, max_triangles, cone_weight));

	if (meshlets.size())
	{
		const meshopt_Meshlet& last = meshlets.back();

		// this is an example of how to trim the vertex/triangle arrays when copying data out to GPU storage
		meshlet_vertices.resize(last.vertex_offset + last.vertex_count);
		meshlet_triangles.resize(last.triangle_offset + ((last.triangle_count * 3 + 3) & ~3));
	}

	double end = timestamp();

	double avg_vertices = 0;
	double avg_triangles = 0;
	size_t not_full = 0;

	for (size_t i = 0; i < meshlets.size(); ++i)
	{
		const meshopt_Meshlet& m = meshlets[i];

		avg_vertices += m.vertex_count;
		avg_triangles += m.triangle_count;
		not_full += m.vertex_count < max_vertices;
	}

	avg_vertices /= double(meshlets.size());
	avg_triangles /= double(meshlets.size());

	printf("Meshlets%c: %d meshlets (avg vertices %.1f, avg triangles %.1f, not full %d) in %.2f msec\n",
	    scan ? 'S' : ' ',
	    int(meshlets.size()), avg_vertices, avg_triangles, int(not_full), (end - start) * 1000);

	float camera[3] = {100, 100, 100};

	size_t rejected = 0;
	size_t rejected_s8 = 0;
	size_t rejected_alt = 0;
	size_t rejected_alt_s8 = 0;
	size_t accepted = 0;
	size_t accepted_s8 = 0;

	std::vector<float> radii(meshlets.size());

	double startc = timestamp();
	for (size_t i = 0; i < meshlets.size(); ++i)
	{
		const meshopt_Meshlet& m = meshlets[i];

		meshopt_Bounds bounds = meshopt_computeMeshletBounds(&meshlet_vertices[m.vertex_offset], &meshlet_triangles[m.triangle_offset], m.triangle_count, &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex));

		radii[i] = bounds.radius;

		// trivial accept: we can't ever backface cull this meshlet
		accepted += (bounds.cone_cutoff >= 1);
		accepted_s8 += (bounds.cone_cutoff_s8 >= 127);

		// perspective projection: dot(normalize(cone_apex - camera_position), cone_axis) > cone_cutoff
		float mview[3] = {bounds.cone_apex[0] - camera[0], bounds.cone_apex[1] - camera[1], bounds.cone_apex[2] - camera[2]};
		float mviewlength = sqrtf(mview[0] * mview[0] + mview[1] * mview[1] + mview[2] * mview[2]);

		rejected += mview[0] * bounds.cone_axis[0] + mview[1] * bounds.cone_axis[1] + mview[2] * bounds.cone_axis[2] >= bounds.cone_cutoff * mviewlength;
		rejected_s8 += mview[0] * (bounds.cone_axis_s8[0] / 127.f) + mview[1] * (bounds.cone_axis_s8[1] / 127.f) + mview[2] * (bounds.cone_axis_s8[2] / 127.f) >= (bounds.cone_cutoff_s8 / 127.f) * mviewlength;

		// alternative formulation for perspective projection that doesn't use apex (and uses cluster bounding sphere instead):
		// dot(normalize(center - camera_position), cone_axis) > cone_cutoff + radius / length(center - camera_position)
		float cview[3] = {bounds.center[0] - camera[0], bounds.center[1] - camera[1], bounds.center[2] - camera[2]};
		float cviewlength = sqrtf(cview[0] * cview[0] + cview[1] * cview[1] + cview[2] * cview[2]);

		rejected_alt += cview[0] * bounds.cone_axis[0] + cview[1] * bounds.cone_axis[1] + cview[2] * bounds.cone_axis[2] >= bounds.cone_cutoff * cviewlength + bounds.radius;
		rejected_alt_s8 += cview[0] * (bounds.cone_axis_s8[0] / 127.f) + cview[1] * (bounds.cone_axis_s8[1] / 127.f) + cview[2] * (bounds.cone_axis_s8[2] / 127.f) >= (bounds.cone_cutoff_s8 / 127.f) * cviewlength + bounds.radius;
	}
	double endc = timestamp();

	double radius_mean = 0;

	for (size_t i = 0; i < meshlets.size(); ++i)
		radius_mean += radii[i];

	radius_mean /= double(meshlets.size());

	double radius_variance = 0;

	for (size_t i = 0; i < meshlets.size(); ++i)
		radius_variance += (radii[i] - radius_mean) * (radii[i] - radius_mean);

	radius_variance /= double(meshlets.size() - 1);

	double radius_stddev = sqrt(radius_variance);

	size_t meshlets_std = 0;

	for (size_t i = 0; i < meshlets.size(); ++i)
		meshlets_std += radii[i] < radius_mean + radius_stddev;

	printf("BoundDist: mean %f stddev %f; %.1f%% meshlets are under mean+stddev\n",
	    radius_mean,
	    radius_stddev,
	    double(meshlets_std) / double(meshlets.size()) * 100);

	printf("ConeCull : rejected apex %d (%.1f%%) / center %d (%.1f%%), trivially accepted %d (%.1f%%) in %.2f msec\n",
	    int(rejected), double(rejected) / double(meshlets.size()) * 100,
	    int(rejected_alt), double(rejected_alt) / double(meshlets.size()) * 100,
	    int(accepted), double(accepted) / double(meshlets.size()) * 100,
	    (endc - startc) * 1000);
	printf("ConeCull8: rejected apex %d (%.1f%%) / center %d (%.1f%%), trivially accepted %d (%.1f%%) in %.2f msec\n",
	    int(rejected_s8), double(rejected_s8) / double(meshlets.size()) * 100,
	    int(rejected_alt_s8), double(rejected_alt_s8) / double(meshlets.size()) * 100,
	    int(accepted_s8), double(accepted_s8) / double(meshlets.size()) * 100,
	    (endc - startc) * 1000);
}

void spatialSort(const Mesh& mesh)
{
	typedef PackedVertexOct PV;

	std::vector<PV> pv(mesh.vertices.size());
	packMesh(pv, mesh.vertices);

	double start = timestamp();

	std::vector<unsigned int> remap(mesh.vertices.size());
	meshopt_spatialSortRemap(&remap[0], &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex));

	double end = timestamp();

	meshopt_remapVertexBuffer(&pv[0], &pv[0], mesh.vertices.size(), sizeof(PV), &remap[0]);

	std::vector<unsigned char> vbuf(meshopt_encodeVertexBufferBound(mesh.vertices.size(), sizeof(PV)));
	vbuf.resize(meshopt_encodeVertexBuffer(&vbuf[0], vbuf.size(), &pv[0], mesh.vertices.size(), sizeof(PV)));

	size_t csize = compress(vbuf);

	printf("Spatial  : %.1f bits/vertex (post-deflate %.1f bits/vertex); sort %.2f msec\n",
	    double(vbuf.size() * 8) / double(mesh.vertices.size()),
	    double(csize * 8) / double(mesh.vertices.size()),
	    (end - start) * 1000);
}

void spatialSortTriangles(const Mesh& mesh)
{
	typedef PackedVertexOct PV;

	Mesh copy = mesh;

	double start = timestamp();

	meshopt_spatialSortTriangles(&copy.indices[0], &copy.indices[0], mesh.indices.size(), &copy.vertices[0].px, copy.vertices.size(), sizeof(Vertex));

	double end = timestamp();

	meshopt_optimizeVertexCache(&copy.indices[0], &copy.indices[0], copy.indices.size(), copy.vertices.size());
	meshopt_optimizeVertexFetch(&copy.vertices[0], &copy.indices[0], copy.indices.size(), &copy.vertices[0], copy.vertices.size(), sizeof(Vertex));

	std::vector<PV> pv(mesh.vertices.size());
	packMesh(pv, copy.vertices);

	std::vector<unsigned char> vbuf(meshopt_encodeVertexBufferBound(mesh.vertices.size(), sizeof(PV)));
	vbuf.resize(meshopt_encodeVertexBuffer(&vbuf[0], vbuf.size(), &pv[0], mesh.vertices.size(), sizeof(PV)));

	std::vector<unsigned char> ibuf(meshopt_encodeIndexBufferBound(mesh.indices.size(), mesh.vertices.size()));
	ibuf.resize(meshopt_encodeIndexBuffer(&ibuf[0], ibuf.size(), &copy.indices[0], mesh.indices.size()));

	size_t csizev = compress(vbuf);
	size_t csizei = compress(ibuf);

	printf("SpatialT : %.1f bits/vertex (post-deflate %.1f bits/vertex); %.1f bits/triangle (post-deflate %.1f bits/triangle); sort %.2f msec\n",
	    double(vbuf.size() * 8) / double(mesh.vertices.size()),
	    double(csizev * 8) / double(mesh.vertices.size()),
	    double(ibuf.size() * 8) / double(mesh.indices.size() / 3),
	    double(csizei * 8) / double(mesh.indices.size() / 3),
	    (end - start) * 1000);
}

void tessellationAdjacency(const Mesh& mesh)
{
	double start = timestamp();

	// 12 indices per input triangle
	std::vector<unsigned int> tessib(mesh.indices.size() * 4);
	meshopt_generateTessellationIndexBuffer(&tessib[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex));

	double middle = timestamp();

	// 6 indices per input triangle
	std::vector<unsigned int> adjib(mesh.indices.size() * 2);
	meshopt_generateAdjacencyIndexBuffer(&adjib[0], &mesh.indices[0], mesh.indices.size(), &mesh.vertices[0].px, mesh.vertices.size(), sizeof(Vertex));

	double end = timestamp();

	printf("Tesselltn: %d patches in %.2f msec\n", int(mesh.indices.size() / 3), (middle - start) * 1000);
	printf("Adjacency: %d patches in %.2f msec\n", int(mesh.indices.size() / 3), (end - middle) * 1000);
}

bool loadMesh(Mesh& mesh, const char* path)
{
	double start = timestamp();
	double middle;
	mesh = parseObj(path, middle);
	double end = timestamp();

	if (mesh.vertices.empty())
	{
		printf("Mesh %s is empty, skipping\n", path);
		return false;
	}

	printf("# %s: %d vertices, %d triangles; read in %.2f msec; indexed in %.2f msec\n", path, int(mesh.vertices.size()), int(mesh.indices.size() / 3), (middle - start) * 1000, (end - middle) * 1000);
	return true;
}

void processDeinterleaved(const char* path)
{
	// Most algorithms in the library work out of the box with deinterleaved geometry, but some require slightly special treatment;
	// this code runs a simplified version of complete opt. pipeline using deinterleaved geo. There's no compression performed but you
	// can trivially run it by quantizing all elements and running meshopt_encodeVertexBuffer once for each vertex stream.
	fastObjMesh* obj = fast_obj_read(path);
	if (!obj)
	{
		printf("Error loading %s: file not found\n", path);
		return;
	}

	size_t total_indices = 0;

	for (unsigned int i = 0; i < obj->face_count; ++i)
		total_indices += 3 * (obj->face_vertices[i] - 2);

	std::vector<float> unindexed_pos(total_indices * 3);
	std::vector<float> unindexed_nrm(total_indices * 3);
	std::vector<float> unindexed_uv(total_indices * 2);

	size_t vertex_offset = 0;
	size_t index_offset = 0;

	for (unsigned int i = 0; i < obj->face_count; ++i)
	{
		for (unsigned int j = 0; j < obj->face_vertices[i]; ++j)
		{
			fastObjIndex gi = obj->indices[index_offset + j];

			// triangulate polygon on the fly; offset-3 is always the first polygon vertex
			if (j >= 3)
			{
				memcpy(&unindexed_pos[(vertex_offset + 0) * 3], &unindexed_pos[(vertex_offset - 3) * 3], 3 * sizeof(float));
				memcpy(&unindexed_nrm[(vertex_offset + 0) * 3], &unindexed_nrm[(vertex_offset - 3) * 3], 3 * sizeof(float));
				memcpy(&unindexed_uv[(vertex_offset + 0) * 2], &unindexed_uv[(vertex_offset - 3) * 2], 2 * sizeof(float));
				memcpy(&unindexed_pos[(vertex_offset + 1) * 3], &unindexed_pos[(vertex_offset - 1) * 3], 3 * sizeof(float));
				memcpy(&unindexed_nrm[(vertex_offset + 1) * 3], &unindexed_nrm[(vertex_offset - 1) * 3], 3 * sizeof(float));
				memcpy(&unindexed_uv[(vertex_offset + 1) * 2], &unindexed_uv[(vertex_offset - 1) * 2], 2 * sizeof(float));
				vertex_offset += 2;
			}

			memcpy(&unindexed_pos[vertex_offset * 3], &obj->positions[gi.p * 3], 3 * sizeof(float));
			memcpy(&unindexed_nrm[vertex_offset * 3], &obj->normals[gi.n * 3], 3 * sizeof(float));
			memcpy(&unindexed_uv[vertex_offset * 2], &obj->texcoords[gi.t * 2], 2 * sizeof(float));
			vertex_offset++;
		}

		index_offset += obj->face_vertices[i];
	}

	fast_obj_destroy(obj);

	double start = timestamp();

	meshopt_Stream streams[] = {
	    {&unindexed_pos[0], sizeof(float) * 3, sizeof(float) * 3},
	    {&unindexed_nrm[0], sizeof(float) * 3, sizeof(float) * 3},
	    {&unindexed_uv[0], sizeof(float) * 2, sizeof(float) * 2},
	};

	std::vector<unsigned int> remap(total_indices);

	size_t total_vertices = meshopt_generateVertexRemapMulti(&remap[0], NULL, total_indices, total_indices, streams, sizeof(streams) / sizeof(streams[0]));

	std::vector<unsigned int> indices(total_indices);
	meshopt_remapIndexBuffer(&indices[0], NULL, total_indices, &remap[0]);

	std::vector<float> pos(total_vertices * 3);
	meshopt_remapVertexBuffer(&pos[0], &unindexed_pos[0], total_indices, sizeof(float) * 3, &remap[0]);

	std::vector<float> nrm(total_vertices * 3);
	meshopt_remapVertexBuffer(&nrm[0], &unindexed_nrm[0], total_indices, sizeof(float) * 3, &remap[0]);

	std::vector<float> uv(total_vertices * 2);
	meshopt_remapVertexBuffer(&uv[0], &unindexed_uv[0], total_indices, sizeof(float) * 2, &remap[0]);

	double reindex = timestamp();

	meshopt_optimizeVertexCache(&indices[0], &indices[0], total_indices, total_vertices);

	meshopt_optimizeVertexFetchRemap(&remap[0], &indices[0], total_indices, total_vertices);
	meshopt_remapVertexBuffer(&pos[0], &pos[0], total_vertices, sizeof(float) * 3, &remap[0]);
	meshopt_remapVertexBuffer(&nrm[0], &nrm[0], total_vertices, sizeof(float) * 3, &remap[0]);
	meshopt_remapVertexBuffer(&uv[0], &uv[0], total_vertices, sizeof(float) * 2, &remap[0]);

	double optimize = timestamp();

	// note: since shadow index buffer is computed based on regular vertex/index buffer, the stream points at the indexed data - not unindexed_pos
	meshopt_Stream shadow_stream = {&pos[0], sizeof(float) * 3, sizeof(float) * 3};

	std::vector<unsigned int> shadow_indices(total_indices);
	meshopt_generateShadowIndexBufferMulti(&shadow_indices[0], &indices[0], total_indices, total_vertices, &shadow_stream, 1);

	meshopt_optimizeVertexCache(&shadow_indices[0], &shadow_indices[0], total_indices, total_vertices);

	double shadow = timestamp();

	printf("Deintrlvd: %d vertices, reindexed in %.2f msec, optimized in %.2f msec, generated & optimized shadow indices in %.2f msec\n",
	    int(total_vertices), (reindex - start) * 1000, (optimize - reindex) * 1000, (shadow - optimize) * 1000);
}

void process(const char* path)
{
	Mesh mesh;
	if (!loadMesh(mesh, path))
		return;

	optimize(mesh, "Original", optNone);
	optimize(mesh, "Random", optRandomShuffle);
	optimize(mesh, "Cache", optCache);
	optimize(mesh, "CacheFifo", optCacheFifo);
	optimize(mesh, "CacheStrp", optCacheStrip);
	optimize(mesh, "Overdraw", optOverdraw);
	optimize(mesh, "Fetch", optFetch);
	optimize(mesh, "FetchMap", optFetchRemap);
	optimize(mesh, "Complete", optComplete);

	Mesh copy = mesh;
	meshopt_optimizeVertexCache(&copy.indices[0], &copy.indices[0], copy.indices.size(), copy.vertices.size());
	meshopt_optimizeVertexFetch(&copy.vertices[0], &copy.indices[0], copy.indices.size(), &copy.vertices[0], copy.vertices.size(), sizeof(Vertex));

	Mesh copystrip = mesh;
	meshopt_optimizeVertexCacheStrip(&copystrip.indices[0], &copystrip.indices[0], copystrip.indices.size(), copystrip.vertices.size());
	meshopt_optimizeVertexFetch(&copystrip.vertices[0], &copystrip.indices[0], copystrip.indices.size(), &copystrip.vertices[0], copystrip.vertices.size(), sizeof(Vertex));

	stripify(copy, false, ' ');
	stripify(copy, true, 'R');
	stripify(copystrip, true, 'S');

	meshlets(copy, false);
	meshlets(copy, true);

	shadow(copy);
	tessellationAdjacency(copy);

	encodeIndex(copy, ' ');
	encodeIndex(copystrip, 'S');

	std::vector<unsigned int> strip(meshopt_stripifyBound(copystrip.indices.size()));
	strip.resize(meshopt_stripify(&strip[0], &copystrip.indices[0], copystrip.indices.size(), copystrip.vertices.size(), 0));

	encodeIndexSequence(strip, copystrip.vertices.size(), 'D');

	packVertex<PackedVertex>(copy, "");
	encodeVertex<PackedVertex>(copy, "");
	encodeVertex<PackedVertexOct>(copy, "O");

	simplify(mesh);
	simplifySloppy(mesh);
	simplifyComplete(mesh);
	simplifyPoints(mesh);

	spatialSort(mesh);
	spatialSortTriangles(mesh);

	if (path)
		processDeinterleaved(path);
}

void processDev(const char* path)
{
	Mesh mesh;
	if (!loadMesh(mesh, path))
		return;

	tessellationAdjacency(mesh);
}

int main(int argc, char** argv)
{
	void runTests();

	meshopt_encodeVertexVersion(0);
	meshopt_encodeIndexVersion(1);

	if (argc == 1)
	{
		runTests();
	}
	else
	{
		if (strcmp(argv[1], "-d") == 0)
		{
			for (int i = 2; i < argc; ++i)
			{
				processDev(argv[i]);
			}
		}
		else
		{
			for (int i = 1; i < argc; ++i)
			{
				process(argv[i]);
			}

			runTests();
		}
	}
}
