#include "../src/meshoptimizer.h"

#include <stdint.h>
#include <stdlib.h>

void fuzzDecoder(const uint8_t* data, size_t size, size_t stride, int (*decode)(void*, size_t, size_t, const unsigned char*, size_t))
{
	size_t count = 66; // must be divisible by 3 for decodeIndexBuffer; should be >=64 to cover large vertex blocks

	void* destination = malloc(count * stride);
	assert(destination);

	int rc = decode(destination, count, stride, reinterpret_cast<const unsigned char*>(data), size);
	(void)rc;

	free(destination);
}

namespace meshopt
{
extern unsigned int cpuid;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
	// decodeIndexBuffer supports 2 and 4-byte indices
	fuzzDecoder(data, size, 2, meshopt_decodeIndexBuffer);
	fuzzDecoder(data, size, 4, meshopt_decodeIndexBuffer);

	// decodeIndexSequence supports 2 and 4-byte indices
	fuzzDecoder(data, size, 2, meshopt_decodeIndexSequence);
	fuzzDecoder(data, size, 4, meshopt_decodeIndexSequence);

	// decodeVertexBuffer supports any strides divisible by 4 in 4-256 interval
	// It's a waste of time to check all of them, so we'll just check a few with different alignment mod 16
	fuzzDecoder(data, size, 4, meshopt_decodeVertexBuffer);
	fuzzDecoder(data, size, 16, meshopt_decodeVertexBuffer);
	fuzzDecoder(data, size, 24, meshopt_decodeVertexBuffer);
	fuzzDecoder(data, size, 32, meshopt_decodeVertexBuffer);

	return 0;
}
