#include "../src/meshoptimizer.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

// This file uses assert() to verify algorithm correctness
#undef NDEBUG
#include <assert.h>

struct PV
{
	unsigned short px, py, pz;
	unsigned char nu, nv; // octahedron encoded normal, aliases .pw
	unsigned short tx, ty;
};

// note: 4 6 5 triangle here is a combo-breaker:
// we encode it without rotating, a=next, c=next - this means we do *not* bump next to 6
// which means that the next triangle can't be encoded via next sequencing!
static const unsigned int kIndexBuffer[] = {0, 1, 2, 2, 1, 3, 4, 6, 5, 7, 8, 9};

static const unsigned char kIndexDataV0[] = {
    0xe0, 0xf0, 0x10, 0xfe, 0xff, 0xf0, 0x0c, 0xff, 0x02, 0x02, 0x02, 0x00, 0x76, 0x87, 0x56, 0x67,
    0x78, 0xa9, 0x86, 0x65, 0x89, 0x68, 0x98, 0x01, 0x69, 0x00, 0x00, // clang-format :-/
};

// note: this exercises two features of v1 format, restarts (0 1 2) and last
static const unsigned int kIndexBufferTricky[] = {0, 1, 2, 2, 1, 3, 0, 1, 2, 2, 1, 5, 2, 1, 4};

static const unsigned char kIndexDataV1[] = {
    0xe1, 0xf0, 0x10, 0xfe, 0x1f, 0x3d, 0x00, 0x0a, 0x00, 0x76, 0x87, 0x56, 0x67, 0x78, 0xa9, 0x86,
    0x65, 0x89, 0x68, 0x98, 0x01, 0x69, 0x00, 0x00, // clang-format :-/
};

static const unsigned int kIndexSequence[] = {0, 1, 51, 2, 49, 1000};

static const unsigned char kIndexSequenceV1[] = {
    0xd1, 0x00, 0x04, 0xcd, 0x01, 0x04, 0x07, 0x98, 0x1f, 0x00, 0x00, 0x00, 0x00, // clang-format :-/
};

static const PV kVertexBuffer[] = {
    {0, 0, 0, 0, 0, 0, 0},
    {300, 0, 0, 0, 0, 500, 0},
    {0, 300, 0, 0, 0, 0, 500},
    {300, 300, 0, 0, 0, 500, 500},
};

static const unsigned char kVertexDataV0[] = {
    0xa0, 0x01, 0x3f, 0x00, 0x00, 0x00, 0x58, 0x57, 0x58, 0x01, 0x26, 0x00, 0x00, 0x00, 0x01,
    0x0c, 0x00, 0x00, 0x00, 0x58, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x3f, 0x00, 0x00, 0x00, 0x17, 0x18, 0x17, 0x01, 0x26, 0x00, 0x00, 0x00, 0x01, 0x0c, 0x00,
    0x00, 0x00, 0x17, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // clang-format :-/
};

static void decodeIndexV0()
{
	const size_t index_count = sizeof(kIndexBuffer) / sizeof(kIndexBuffer[0]);

	std::vector<unsigned char> buffer(kIndexDataV0, kIndexDataV0 + sizeof(kIndexDataV0));

	unsigned int decoded[index_count];
	assert(meshopt_decodeIndexBuffer(decoded, index_count, &buffer[0], buffer.size()) == 0);
	assert(memcmp(decoded, kIndexBuffer, sizeof(kIndexBuffer)) == 0);
}

static void decodeIndexV1()
{
	const size_t index_count = sizeof(kIndexBufferTricky) / sizeof(kIndexBufferTricky[0]);

	std::vector<unsigned char> buffer(kIndexDataV1, kIndexDataV1 + sizeof(kIndexDataV1));

	unsigned int decoded[index_count];
	assert(meshopt_decodeIndexBuffer(decoded, index_count, &buffer[0], buffer.size()) == 0);
	assert(memcmp(decoded, kIndexBufferTricky, sizeof(kIndexBufferTricky)) == 0);
}

static void decodeIndex16()
{
	const size_t index_count = sizeof(kIndexBuffer) / sizeof(kIndexBuffer[0]);
	const size_t vertex_count = 10;

	std::vector<unsigned char> buffer(meshopt_encodeIndexBufferBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexBuffer(&buffer[0], buffer.size(), kIndexBuffer, index_count));

	unsigned short decoded[index_count];
	assert(meshopt_decodeIndexBuffer(decoded, index_count, &buffer[0], buffer.size()) == 0);

	for (size_t i = 0; i < index_count; ++i)
		assert(decoded[i] == kIndexBuffer[i]);
}

static void encodeIndexMemorySafe()
{
	const size_t index_count = sizeof(kIndexBuffer) / sizeof(kIndexBuffer[0]);
	const size_t vertex_count = 10;

	std::vector<unsigned char> buffer(meshopt_encodeIndexBufferBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexBuffer(&buffer[0], buffer.size(), kIndexBuffer, index_count));

	// check that encode is memory-safe; note that we reallocate the buffer for each try to make sure ASAN can verify buffer access
	for (size_t i = 0; i <= buffer.size(); ++i)
	{
		std::vector<unsigned char> shortbuffer(i);
		size_t result = meshopt_encodeIndexBuffer(i == 0 ? 0 : &shortbuffer[0], i, kIndexBuffer, index_count);

		if (i == buffer.size())
			assert(result == buffer.size());
		else
			assert(result == 0);
	}
}

static void decodeIndexMemorySafe()
{
	const size_t index_count = sizeof(kIndexBuffer) / sizeof(kIndexBuffer[0]);
	const size_t vertex_count = 10;

	std::vector<unsigned char> buffer(meshopt_encodeIndexBufferBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexBuffer(&buffer[0], buffer.size(), kIndexBuffer, index_count));

	// check that decode is memory-safe; note that we reallocate the buffer for each try to make sure ASAN can verify buffer access
	unsigned int decoded[index_count];

	for (size_t i = 0; i <= buffer.size(); ++i)
	{
		std::vector<unsigned char> shortbuffer(buffer.begin(), buffer.begin() + i);
		int result = meshopt_decodeIndexBuffer(decoded, index_count, i == 0 ? 0 : &shortbuffer[0], i);

		if (i == buffer.size())
			assert(result == 0);
		else
			assert(result < 0);
	}
}

static void decodeIndexRejectExtraBytes()
{
	const size_t index_count = sizeof(kIndexBuffer) / sizeof(kIndexBuffer[0]);
	const size_t vertex_count = 10;

	std::vector<unsigned char> buffer(meshopt_encodeIndexBufferBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexBuffer(&buffer[0], buffer.size(), kIndexBuffer, index_count));

	// check that decoder doesn't accept extra bytes after a valid stream
	std::vector<unsigned char> largebuffer(buffer);
	largebuffer.push_back(0);

	unsigned int decoded[index_count];
	assert(meshopt_decodeIndexBuffer(decoded, index_count, &largebuffer[0], largebuffer.size()) < 0);
}

static void decodeIndexRejectMalformedHeaders()
{
	const size_t index_count = sizeof(kIndexBuffer) / sizeof(kIndexBuffer[0]);
	const size_t vertex_count = 10;

	std::vector<unsigned char> buffer(meshopt_encodeIndexBufferBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexBuffer(&buffer[0], buffer.size(), kIndexBuffer, index_count));

	// check that decoder doesn't accept malformed headers
	std::vector<unsigned char> brokenbuffer(buffer);
	brokenbuffer[0] = 0;

	unsigned int decoded[index_count];
	assert(meshopt_decodeIndexBuffer(decoded, index_count, &brokenbuffer[0], brokenbuffer.size()) < 0);
}

static void decodeIndexRejectInvalidVersion()
{
	const size_t index_count = sizeof(kIndexBuffer) / sizeof(kIndexBuffer[0]);
	const size_t vertex_count = 10;

	std::vector<unsigned char> buffer(meshopt_encodeIndexBufferBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexBuffer(&buffer[0], buffer.size(), kIndexBuffer, index_count));

	// check that decoder doesn't accept invalid version
	std::vector<unsigned char> brokenbuffer(buffer);
	brokenbuffer[0] |= 0x0f;

	unsigned int decoded[index_count];
	assert(meshopt_decodeIndexBuffer(decoded, index_count, &brokenbuffer[0], brokenbuffer.size()) < 0);
}

static void decodeIndexMalformedVByte()
{
	const unsigned char input[] = {
	    0xe1, 0x20, 0x20, 0x20, 0xff, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	    0xff, 0xff, 0xff, 0xff, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	    0x20, 0x20, 0x20, // clang-format :-/
	};

	unsigned int decoded[66];
	assert(meshopt_decodeIndexBuffer(decoded, 66, input, sizeof(input)) < 0);
}

static void roundtripIndexTricky()
{
	const size_t index_count = sizeof(kIndexBufferTricky) / sizeof(kIndexBufferTricky[0]);
	const size_t vertex_count = 6;

	std::vector<unsigned char> buffer(meshopt_encodeIndexBufferBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexBuffer(&buffer[0], buffer.size(), kIndexBufferTricky, index_count));

	unsigned int decoded[index_count];
	assert(meshopt_decodeIndexBuffer(decoded, index_count, &buffer[0], buffer.size()) == 0);
	assert(memcmp(decoded, kIndexBufferTricky, sizeof(kIndexBufferTricky)) == 0);
}

static void encodeIndexEmpty()
{
	std::vector<unsigned char> buffer(meshopt_encodeIndexBufferBound(0, 0));
	buffer.resize(meshopt_encodeIndexBuffer(&buffer[0], buffer.size(), NULL, 0));

	assert(meshopt_decodeIndexBuffer(static_cast<unsigned int*>(NULL), 0, &buffer[0], buffer.size()) == 0);
}

static void decodeIndexSequence()
{
	const size_t index_count = sizeof(kIndexSequence) / sizeof(kIndexSequence[0]);

	std::vector<unsigned char> buffer(kIndexSequenceV1, kIndexSequenceV1 + sizeof(kIndexSequenceV1));

	unsigned int decoded[index_count];
	assert(meshopt_decodeIndexSequence(decoded, index_count, &buffer[0], buffer.size()) == 0);
	assert(memcmp(decoded, kIndexSequence, sizeof(kIndexSequence)) == 0);
}

static void decodeIndexSequence16()
{
	const size_t index_count = sizeof(kIndexSequence) / sizeof(kIndexSequence[0]);
	const size_t vertex_count = 1001;

	std::vector<unsigned char> buffer(meshopt_encodeIndexSequenceBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexSequence(&buffer[0], buffer.size(), kIndexSequence, index_count));

	unsigned short decoded[index_count];
	assert(meshopt_decodeIndexSequence(decoded, index_count, &buffer[0], buffer.size()) == 0);

	for (size_t i = 0; i < index_count; ++i)
		assert(decoded[i] == kIndexSequence[i]);
}

static void encodeIndexSequenceMemorySafe()
{
	const size_t index_count = sizeof(kIndexSequence) / sizeof(kIndexSequence[0]);
	const size_t vertex_count = 1001;

	std::vector<unsigned char> buffer(meshopt_encodeIndexSequenceBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexSequence(&buffer[0], buffer.size(), kIndexSequence, index_count));

	// check that encode is memory-safe; note that we reallocate the buffer for each try to make sure ASAN can verify buffer access
	for (size_t i = 0; i <= buffer.size(); ++i)
	{
		std::vector<unsigned char> shortbuffer(i);
		size_t result = meshopt_encodeIndexSequence(i == 0 ? 0 : &shortbuffer[0], i, kIndexSequence, index_count);

		if (i == buffer.size())
			assert(result == buffer.size());
		else
			assert(result == 0);
	}
}

static void decodeIndexSequenceMemorySafe()
{
	const size_t index_count = sizeof(kIndexSequence) / sizeof(kIndexSequence[0]);
	const size_t vertex_count = 1001;

	std::vector<unsigned char> buffer(meshopt_encodeIndexSequenceBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexSequence(&buffer[0], buffer.size(), kIndexSequence, index_count));

	// check that decode is memory-safe; note that we reallocate the buffer for each try to make sure ASAN can verify buffer access
	unsigned int decoded[index_count];

	for (size_t i = 0; i <= buffer.size(); ++i)
	{
		std::vector<unsigned char> shortbuffer(buffer.begin(), buffer.begin() + i);
		int result = meshopt_decodeIndexSequence(decoded, index_count, i == 0 ? 0 : &shortbuffer[0], i);

		if (i == buffer.size())
			assert(result == 0);
		else
			assert(result < 0);
	}
}

static void decodeIndexSequenceRejectExtraBytes()
{
	const size_t index_count = sizeof(kIndexSequence) / sizeof(kIndexSequence[0]);
	const size_t vertex_count = 1001;

	std::vector<unsigned char> buffer(meshopt_encodeIndexSequenceBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexSequence(&buffer[0], buffer.size(), kIndexSequence, index_count));

	// check that decoder doesn't accept extra bytes after a valid stream
	std::vector<unsigned char> largebuffer(buffer);
	largebuffer.push_back(0);

	unsigned int decoded[index_count];
	assert(meshopt_decodeIndexSequence(decoded, index_count, &largebuffer[0], largebuffer.size()) < 0);
}

static void decodeIndexSequenceRejectMalformedHeaders()
{
	const size_t index_count = sizeof(kIndexSequence) / sizeof(kIndexSequence[0]);
	const size_t vertex_count = 1001;

	std::vector<unsigned char> buffer(meshopt_encodeIndexSequenceBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexSequence(&buffer[0], buffer.size(), kIndexSequence, index_count));

	// check that decoder doesn't accept malformed headers
	std::vector<unsigned char> brokenbuffer(buffer);
	brokenbuffer[0] = 0;

	unsigned int decoded[index_count];
	assert(meshopt_decodeIndexSequence(decoded, index_count, &brokenbuffer[0], brokenbuffer.size()) < 0);
}

static void decodeIndexSequenceRejectInvalidVersion()
{
	const size_t index_count = sizeof(kIndexSequence) / sizeof(kIndexSequence[0]);
	const size_t vertex_count = 1001;

	std::vector<unsigned char> buffer(meshopt_encodeIndexSequenceBound(index_count, vertex_count));
	buffer.resize(meshopt_encodeIndexSequence(&buffer[0], buffer.size(), kIndexSequence, index_count));

	// check that decoder doesn't accept invalid version
	std::vector<unsigned char> brokenbuffer(buffer);
	brokenbuffer[0] |= 0x0f;

	unsigned int decoded[index_count];
	assert(meshopt_decodeIndexSequence(decoded, index_count, &brokenbuffer[0], brokenbuffer.size()) < 0);
}

static void encodeIndexSequenceEmpty()
{
	std::vector<unsigned char> buffer(meshopt_encodeIndexSequenceBound(0, 0));
	buffer.resize(meshopt_encodeIndexSequence(&buffer[0], buffer.size(), NULL, 0));

	assert(meshopt_decodeIndexSequence(static_cast<unsigned int*>(NULL), 0, &buffer[0], buffer.size()) == 0);
}

static void decodeVertexV0()
{
	const size_t vertex_count = sizeof(kVertexBuffer) / sizeof(kVertexBuffer[0]);

	std::vector<unsigned char> buffer(kVertexDataV0, kVertexDataV0 + sizeof(kVertexDataV0));

	PV decoded[vertex_count];
	assert(meshopt_decodeVertexBuffer(decoded, vertex_count, sizeof(PV), &buffer[0], buffer.size()) == 0);
	assert(memcmp(decoded, kVertexBuffer, sizeof(kVertexBuffer)) == 0);
}

static void encodeVertexMemorySafe()
{
	const size_t vertex_count = sizeof(kVertexBuffer) / sizeof(kVertexBuffer[0]);

	std::vector<unsigned char> buffer(meshopt_encodeVertexBufferBound(vertex_count, sizeof(PV)));
	buffer.resize(meshopt_encodeVertexBuffer(&buffer[0], buffer.size(), kVertexBuffer, vertex_count, sizeof(PV)));

	// check that encode is memory-safe; note that we reallocate the buffer for each try to make sure ASAN can verify buffer access
	for (size_t i = 0; i <= buffer.size(); ++i)
	{
		std::vector<unsigned char> shortbuffer(i);
		size_t result = meshopt_encodeVertexBuffer(i == 0 ? 0 : &shortbuffer[0], i, kVertexBuffer, vertex_count, sizeof(PV));

		if (i == buffer.size())
			assert(result == buffer.size());
		else
			assert(result == 0);
	}
}

static void decodeVertexMemorySafe()
{
	const size_t vertex_count = sizeof(kVertexBuffer) / sizeof(kVertexBuffer[0]);

	std::vector<unsigned char> buffer(meshopt_encodeVertexBufferBound(vertex_count, sizeof(PV)));
	buffer.resize(meshopt_encodeVertexBuffer(&buffer[0], buffer.size(), kVertexBuffer, vertex_count, sizeof(PV)));

	// check that decode is memory-safe; note that we reallocate the buffer for each try to make sure ASAN can verify buffer access
	PV decoded[vertex_count];

	for (size_t i = 0; i <= buffer.size(); ++i)
	{
		std::vector<unsigned char> shortbuffer(buffer.begin(), buffer.begin() + i);
		int result = meshopt_decodeVertexBuffer(decoded, vertex_count, sizeof(PV), i == 0 ? 0 : &shortbuffer[0], i);
		(void)result;

		if (i == buffer.size())
			assert(result == 0);
		else
			assert(result < 0);
	}
}

static void decodeVertexRejectExtraBytes()
{
	const size_t vertex_count = sizeof(kVertexBuffer) / sizeof(kVertexBuffer[0]);

	std::vector<unsigned char> buffer(meshopt_encodeVertexBufferBound(vertex_count, sizeof(PV)));
	buffer.resize(meshopt_encodeVertexBuffer(&buffer[0], buffer.size(), kVertexBuffer, vertex_count, sizeof(PV)));

	// check that decoder doesn't accept extra bytes after a valid stream
	std::vector<unsigned char> largebuffer(buffer);
	largebuffer.push_back(0);

	PV decoded[vertex_count];
	assert(meshopt_decodeVertexBuffer(decoded, vertex_count, sizeof(PV), &largebuffer[0], largebuffer.size()) < 0);
}

static void decodeVertexRejectMalformedHeaders()
{
	const size_t vertex_count = sizeof(kVertexBuffer) / sizeof(kVertexBuffer[0]);

	std::vector<unsigned char> buffer(meshopt_encodeVertexBufferBound(vertex_count, sizeof(PV)));
	buffer.resize(meshopt_encodeVertexBuffer(&buffer[0], buffer.size(), kVertexBuffer, vertex_count, sizeof(PV)));

	// check that decoder doesn't accept malformed headers
	std::vector<unsigned char> brokenbuffer(buffer);
	brokenbuffer[0] = 0;

	PV decoded[vertex_count];
	assert(meshopt_decodeVertexBuffer(decoded, vertex_count, sizeof(PV), &brokenbuffer[0], brokenbuffer.size()) < 0);
}

static void decodeVertexBitGroups()
{
	unsigned char data[16 * 4];

	// this tests 0/2/4/8 bit groups in one stream
	for (size_t i = 0; i < 16; ++i)
	{
		data[i * 4 + 0] = 0;
		data[i * 4 + 1] = (unsigned char)(i * 1);
		data[i * 4 + 2] = (unsigned char)(i * 2);
		data[i * 4 + 3] = (unsigned char)(i * 8);
	}

	std::vector<unsigned char> buffer(meshopt_encodeVertexBufferBound(16, 4));
	buffer.resize(meshopt_encodeVertexBuffer(&buffer[0], buffer.size(), data, 16, 4));

	unsigned char decoded[16 * 4];
	assert(meshopt_decodeVertexBuffer(decoded, 16, 4, &buffer[0], buffer.size()) == 0);
	assert(memcmp(decoded, data, sizeof(data)) == 0);
}

static void decodeVertexBitGroupSentinels()
{
	unsigned char data[16 * 4];

	// this tests 0/2/4/8 bit groups and sentinels in one stream
	for (size_t i = 0; i < 16; ++i)
	{
		if (i == 7 || i == 13)
		{
			data[i * 4 + 0] = 42;
			data[i * 4 + 1] = 42;
			data[i * 4 + 2] = 42;
			data[i * 4 + 3] = 42;
		}
		else
		{
			data[i * 4 + 0] = 0;
			data[i * 4 + 1] = (unsigned char)(i * 1);
			data[i * 4 + 2] = (unsigned char)(i * 2);
			data[i * 4 + 3] = (unsigned char)(i * 8);
		}
	}

	std::vector<unsigned char> buffer(meshopt_encodeVertexBufferBound(16, 4));
	buffer.resize(meshopt_encodeVertexBuffer(&buffer[0], buffer.size(), data, 16, 4));

	unsigned char decoded[16 * 4];
	assert(meshopt_decodeVertexBuffer(decoded, 16, 4, &buffer[0], buffer.size()) == 0);
	assert(memcmp(decoded, data, sizeof(data)) == 0);
}

static void decodeVertexLarge()
{
	unsigned char data[128 * 4];

	// this tests 0/2/4/8 bit groups in one stream
	for (size_t i = 0; i < 128; ++i)
	{
		data[i * 4 + 0] = 0;
		data[i * 4 + 1] = (unsigned char)(i * 1);
		data[i * 4 + 2] = (unsigned char)(i * 2);
		data[i * 4 + 3] = (unsigned char)(i * 8);
	}

	std::vector<unsigned char> buffer(meshopt_encodeVertexBufferBound(128, 4));
	buffer.resize(meshopt_encodeVertexBuffer(&buffer[0], buffer.size(), data, 128, 4));

	unsigned char decoded[128 * 4];
	assert(meshopt_decodeVertexBuffer(decoded, 128, 4, &buffer[0], buffer.size()) == 0);
	assert(memcmp(decoded, data, sizeof(data)) == 0);
}

static void encodeVertexEmpty()
{
	std::vector<unsigned char> buffer(meshopt_encodeVertexBufferBound(0, 16));
	buffer.resize(meshopt_encodeVertexBuffer(&buffer[0], buffer.size(), NULL, 0, 16));

	assert(meshopt_decodeVertexBuffer(NULL, 0, 16, &buffer[0], buffer.size()) == 0);
}

static void decodeFilterOct8()
{
	const unsigned char data[4 * 4] = {
	    0, 1, 127, 0,
	    0, 187, 127, 1,
	    255, 1, 127, 0,
	    14, 130, 127, 1, // clang-format :-/
	};

	const unsigned char expected[4 * 4] = {
	    0, 1, 127, 0,
	    0, 159, 82, 1,
	    255, 1, 127, 0,
	    1, 130, 241, 1, // clang-format :-/
	};

	// Aligned by 4
	unsigned char full[4 * 4];
	memcpy(full, data, sizeof(full));
	meshopt_decodeFilterOct(full, 4, 4);
	assert(memcmp(full, expected, sizeof(full)) == 0);

	// Tail processing for unaligned data
	unsigned char tail[3 * 4];
	memcpy(tail, data, sizeof(tail));
	meshopt_decodeFilterOct(tail, 3, 4);
	assert(memcmp(tail, expected, sizeof(tail)) == 0);
}

static void decodeFilterOct12()
{
	const unsigned short data[4 * 4] = {
	    0, 1, 2047, 0,
	    0, 1870, 2047, 1,
	    2017, 1, 2047, 0,
	    14, 1300, 2047, 1, // clang-format :-/
	};

	const unsigned short expected[4 * 4] = {
	    0, 16, 32767, 0,
	    0, 32621, 3088, 1,
	    32764, 16, 471, 0,
	    307, 28541, 16093, 1, // clang-format :-/
	};

	// Aligned by 4
	unsigned short full[4 * 4];
	memcpy(full, data, sizeof(full));
	meshopt_decodeFilterOct(full, 4, 8);
	assert(memcmp(full, expected, sizeof(full)) == 0);

	// Tail processing for unaligned data
	unsigned short tail[3 * 4];
	memcpy(tail, data, sizeof(tail));
	meshopt_decodeFilterOct(tail, 3, 8);
	assert(memcmp(tail, expected, sizeof(tail)) == 0);
}

static void decodeFilterQuat12()
{
	const unsigned short data[4 * 4] = {
	    0, 1, 0, 0x7fc,
	    0, 1870, 0, 0x7fd,
	    2017, 1, 0, 0x7fe,
	    14, 1300, 0, 0x7ff, // clang-format :-/
	};

	const unsigned short expected[4 * 4] = {
	    32767, 0, 11, 0,
	    0, 25013, 0, 21166,
	    11, 0, 23504, 22830,
	    158, 14715, 0, 29277, // clang-format :-/
	};

	// Aligned by 4
	unsigned short full[4 * 4];
	memcpy(full, data, sizeof(full));
	meshopt_decodeFilterQuat(full, 4, 8);
	assert(memcmp(full, expected, sizeof(full)) == 0);

	// Tail processing for unaligned data
	unsigned short tail[3 * 4];
	memcpy(tail, data, sizeof(tail));
	meshopt_decodeFilterQuat(tail, 3, 8);
	assert(memcmp(tail, expected, sizeof(tail)) == 0);
}

static void decodeFilterExp()
{
	const unsigned int data[4] = {
	    0,
	    0xff000003,
	    0x02fffff7,
	    0xfe7fffff, // clang-format :-/
	};

	const unsigned int expected[4] = {
	    0,
	    0x3fc00000,
	    0xc2100000,
	    0x49fffffe, // clang-format :-/
	};

	// Aligned by 4
	unsigned int full[4];
	memcpy(full, data, sizeof(full));
	meshopt_decodeFilterExp(full, 4, 4);
	assert(memcmp(full, expected, sizeof(full)) == 0);

	// Tail processing for unaligned data
	unsigned int tail[3];
	memcpy(tail, data, sizeof(tail));
	meshopt_decodeFilterExp(tail, 3, 4);
	assert(memcmp(tail, expected, sizeof(tail)) == 0);
}

void encodeFilterOct8()
{
	const float data[4 * 4] = {
	    1, 0, 0, 0,
	    0, -1, 0, 0,
	    0.7071068f, 0, 0.707168f, 1,
	    -0.7071068f, 0, -0.707168f, 1, // clang-format :-/
	};

	const unsigned char expected[4 * 4] = {
	    0x7f, 0, 0x7f, 0,
	    0, 0x81, 0x7f, 0,
	    0x3f, 0, 0x7f, 0x7f,
	    0x81, 0x40, 0x7f, 0x7f, // clang-format :-/
	};

	unsigned char encoded[4 * 4];
	meshopt_encodeFilterOct(encoded, 4, 4, 8, data);

	assert(memcmp(encoded, expected, sizeof(expected)) == 0);

	signed char decoded[4 * 4];
	memcpy(decoded, encoded, sizeof(decoded));
	meshopt_decodeFilterOct(decoded, 4, 4);

	for (size_t i = 0; i < 4 * 4; ++i)
		assert(fabsf(decoded[i] / 127.f - data[i]) < 1e-2f);
}

void encodeFilterOct12()
{
	const float data[4 * 4] = {
	    1, 0, 0, 0,
	    0, -1, 0, 0,
	    0.7071068f, 0, 0.707168f, 1,
	    -0.7071068f, 0, -0.707168f, 1, // clang-format :-/
	};

	const unsigned short expected[4 * 4] = {
	    0x7ff, 0, 0x7ff, 0,
	    0x0, 0xf801, 0x7ff, 0,
	    0x3ff, 0, 0x7ff, 0x7fff,
	    0xf801, 0x400, 0x7ff, 0x7fff, // clang-format :-/
	};

	unsigned short encoded[4 * 4];
	meshopt_encodeFilterOct(encoded, 4, 8, 12, data);

	assert(memcmp(encoded, expected, sizeof(expected)) == 0);

	short decoded[4 * 4];
	memcpy(decoded, encoded, sizeof(decoded));
	meshopt_decodeFilterOct(decoded, 4, 8);

	for (size_t i = 0; i < 4 * 4; ++i)
		assert(fabsf(decoded[i] / 32767.f - data[i]) < 1e-3f);
}

void encodeFilterQuat12()
{
	const float data[4 * 4] = {
	    1, 0, 0, 0,
	    0, -1, 0, 0,
	    0.7071068f, 0, 0, 0.707168f,
	    -0.7071068f, 0, 0, -0.707168f, // clang-format :-/
	};

	const unsigned short expected[4 * 4] = {
	    0, 0, 0, 0x7fc,
	    0, 0, 0, 0x7fd,
	    0x7ff, 0, 0, 0x7ff,
	    0x7ff, 0, 0, 0x7ff, // clang-format :-/
	};

	unsigned short encoded[4 * 4];
	meshopt_encodeFilterQuat(encoded, 4, 8, 12, data);

	assert(memcmp(encoded, expected, sizeof(expected)) == 0);

	short decoded[4 * 4];
	memcpy(decoded, encoded, sizeof(decoded));
	meshopt_decodeFilterQuat(decoded, 4, 8);

	for (size_t i = 0; i < 4; ++i)
	{
		float dx = decoded[i * 4 + 0] / 32767.f;
		float dy = decoded[i * 4 + 1] / 32767.f;
		float dz = decoded[i * 4 + 2] / 32767.f;
		float dw = decoded[i * 4 + 3] / 32767.f;

		float dp =
		    data[i * 4 + 0] * dx +
		    data[i * 4 + 1] * dy +
		    data[i * 4 + 2] * dz +
		    data[i * 4 + 3] * dw;

		assert(fabsf(fabsf(dp) - 1.f) < 1e-4f);
	}
}

void encodeFilterExp()
{
	const float data[3] = {
	    1,
	    -23.4f,
	    -0.1f,
	};

	const unsigned int expected[3] = {
	    0xf7000200,
	    0xf7ffd133,
	    0xf7ffffcd,
	};

	unsigned int encoded[3];
	meshopt_encodeFilterExp(encoded, 1, 12, 15, data);

	assert(memcmp(encoded, expected, sizeof(expected)) == 0);

	float decoded[3];
	memcpy(decoded, encoded, sizeof(decoded));
	meshopt_decodeFilterExp(decoded, 3, 4);

	for (size_t i = 0; i < 3; ++i)
		assert(fabsf(decoded[i] - data[i]) < 1e-3f);
}

static void clusterBoundsDegenerate()
{
	const float vbd[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	const unsigned int ibd[] = {0, 0, 0};
	const unsigned int ib1[] = {0, 1, 2};

	// all of the bounds below are degenerate as they use 0 triangles, one topology-degenerate triangle and one position-degenerate triangle respectively
	meshopt_Bounds bounds0 = meshopt_computeClusterBounds(0, 0, 0, 0, 12);
	meshopt_Bounds boundsd = meshopt_computeClusterBounds(ibd, 3, vbd, 3, 12);
	meshopt_Bounds bounds1 = meshopt_computeClusterBounds(ib1, 3, vbd, 3, 12);

	assert(bounds0.center[0] == 0 && bounds0.center[1] == 0 && bounds0.center[2] == 0 && bounds0.radius == 0);
	assert(boundsd.center[0] == 0 && boundsd.center[1] == 0 && boundsd.center[2] == 0 && boundsd.radius == 0);
	assert(bounds1.center[0] == 0 && bounds1.center[1] == 0 && bounds1.center[2] == 0 && bounds1.radius == 0);

	const float vb1[] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
	const unsigned int ib2[] = {0, 1, 2, 0, 2, 1};

	// these bounds have a degenerate cone since the cluster has two triangles with opposite normals
	meshopt_Bounds bounds2 = meshopt_computeClusterBounds(ib2, 6, vb1, 3, 12);

	assert(bounds2.cone_apex[0] == 0 && bounds2.cone_apex[1] == 0 && bounds2.cone_apex[2] == 0);
	assert(bounds2.cone_axis[0] == 0 && bounds2.cone_axis[1] == 0 && bounds2.cone_axis[2] == 0);
	assert(bounds2.cone_cutoff == 1);
	assert(bounds2.cone_axis_s8[0] == 0 && bounds2.cone_axis_s8[1] == 0 && bounds2.cone_axis_s8[2] == 0);
	assert(bounds2.cone_cutoff_s8 == 127);

	// however, the bounding sphere needs to be in tact (here we only check bbox for simplicity)
	assert(bounds2.center[0] - bounds2.radius <= 0 && bounds2.center[0] + bounds2.radius >= 1);
	assert(bounds2.center[1] - bounds2.radius <= 0 && bounds2.center[1] + bounds2.radius >= 1);
	assert(bounds2.center[2] - bounds2.radius <= 0 && bounds2.center[2] + bounds2.radius >= 1);
}

static size_t allocCount;
static size_t freeCount;

static void* customAlloc(size_t size)
{
	allocCount++;

	return malloc(size);
}

static void customFree(void* ptr)
{
	freeCount++;

	free(ptr);
}

static void customAllocator()
{
	meshopt_setAllocator(customAlloc, customFree);

	assert(allocCount == 0 && freeCount == 0);

	float vb[] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
	unsigned int ib[] = {0, 1, 2};
	unsigned short ibs[] = {0, 1, 2};

	// meshopt_computeClusterBounds doesn't allocate
	meshopt_computeClusterBounds(ib, 3, vb, 3, 12);
	assert(allocCount == 0 && freeCount == 0);

	// ... unless IndexAdapter is used
	meshopt_computeClusterBounds(ibs, 3, vb, 3, 12);
	assert(allocCount == 1 && freeCount == 1);

	// meshopt_optimizeVertexFetch allocates internal remap table and temporary storage for in-place remaps
	meshopt_optimizeVertexFetch(vb, ib, 3, vb, 3, 12);
	assert(allocCount == 3 && freeCount == 3);

	// ... plus one for IndexAdapter
	meshopt_optimizeVertexFetch(vb, ibs, 3, vb, 3, 12);
	assert(allocCount == 6 && freeCount == 6);

	meshopt_setAllocator(operator new, operator delete);

	// customAlloc & customFree should not get called anymore
	meshopt_optimizeVertexFetch(vb, ib, 3, vb, 3, 12);
	assert(allocCount == 6 && freeCount == 6);

	allocCount = freeCount = 0;
}

static void emptyMesh()
{
	meshopt_optimizeVertexCache(0, 0, 0, 0);
	meshopt_optimizeVertexCacheFifo(0, 0, 0, 0, 16);
	meshopt_optimizeOverdraw(0, 0, 0, 0, 0, 12, 1.f);
}

static void simplifyStuck()
{
	// tetrahedron can't be simplified due to collapse error restrictions
	float vb1[] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1};
	unsigned int ib1[] = {0, 1, 2, 0, 2, 3, 0, 3, 1, 2, 1, 3};

	assert(meshopt_simplify(ib1, ib1, 12, vb1, 4, 12, 6, 1e-3f) == 12);

	// 5-vertex strip can't be simplified due to topology restriction since middle triangle has flipped winding
	float vb2[] = {0, 0, 0, 1, 0, 0, 2, 0, 0, 0.5f, 1, 0, 1.5f, 1, 0};
	unsigned int ib2[] = {0, 1, 3, 3, 1, 4, 1, 2, 4}; // ok
	unsigned int ib3[] = {0, 1, 3, 1, 3, 4, 1, 2, 4}; // flipped

	assert(meshopt_simplify(ib2, ib2, 9, vb2, 5, 12, 6, 1e-3f) == 6);
	assert(meshopt_simplify(ib3, ib3, 9, vb2, 5, 12, 6, 1e-3f) == 9);

	// 4-vertex quad with a locked corner can't be simplified due to border error-induced restriction
	float vb4[] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0};
	unsigned int ib4[] = {0, 1, 3, 0, 3, 2};

	assert(meshopt_simplify(ib4, ib4, 6, vb4, 4, 12, 3, 1e-3f) == 6);

	// 4-vertex quad with a locked corner can't be simplified due to border error-induced restriction
	float vb5[] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0};
	unsigned int ib5[] = {0, 1, 4, 0, 3, 2};

	assert(meshopt_simplify(ib5, ib5, 6, vb5, 5, 12, 3, 1e-3f) == 6);
}

static void simplifySloppyStuck()
{
	const float vb[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	const unsigned int ib[] = {0, 1, 2, 0, 1, 2};

	unsigned int* target = NULL;

	// simplifying down to 0 triangles results in 0 immediately
	assert(meshopt_simplifySloppy(target, ib, 3, vb, 3, 12, 0, 0.f) == 0);

	// simplifying down to 2 triangles given that all triangles are degenerate results in 0 as well
	assert(meshopt_simplifySloppy(target, ib, 6, vb, 3, 12, 6, 0.f) == 0);
}

static void simplifyPointsStuck()
{
	const float vb[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	// simplifying down to 0 points results in 0 immediately
	assert(meshopt_simplifyPoints(0, vb, 3, 12, 0) == 0);
}

static void simplifyFlip()
{
	// this mesh has been constructed by taking a tessellated irregular grid with a square cutout
	// and progressively collapsing edges until the only ones left violate border or flip constraints.
	// there is only one valid non-flip collapse, so we validate that we take it; when flips are allowed,
	// the wrong collapse is picked instead.
	float vb[] = {
	    1.000000f, 1.000000f, -1.000000f,
	    1.000000f, 1.000000f, 1.000000f,
	    1.000000f, -1.000000f, 1.000000f,
	    1.000000f, -0.200000f, -0.200000f,
	    1.000000f, 0.200000f, -0.200000f,
	    1.000000f, -0.200000f, 0.200000f,
	    1.000000f, 0.200000f, 0.200000f,
	    1.000000f, 0.500000f, -0.500000f,
	    1.000000f, -1.000000f, 0.000000f, // clang-format :-/
	};

	// the collapse we expect is 7 -> 0
	unsigned int ib[] = {
	    7, 4, 3,
	    1, 2, 5,
	    7, 1, 6,
	    7, 8, 0, // gets removed
	    7, 6, 4,
	    8, 5, 2,
	    8, 7, 3,
	    8, 3, 5,
	    5, 6, 1,
	    7, 0, 1, // gets removed
	};

	unsigned int expected[] = {
	    0, 4, 3,
	    1, 2, 5,
	    0, 1, 6,
	    0, 6, 4,
	    8, 5, 2,
	    8, 0, 3,
	    8, 3, 5,
	    5, 6, 1, // clang-format :-/
	};

	assert(meshopt_simplify(ib, ib, 30, vb, 9, 12, 3, 1e-3f) == 24);
	assert(memcmp(ib, expected, sizeof(expected)) == 0);
}

static void simplifyScale()
{
	const float vb[] = {0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3};

	assert(meshopt_simplifyScale(vb, 4, 12) == 3.f);
}

static void simplifyDegenerate()
{
	float vb[] = {
	    0.000000f, 0.000000f, 0.000000f,
	    0.000000f, 1.000000f, 0.000000f,
	    0.000000f, 2.000000f, 0.000000f,
	    1.000000f, 0.000000f, 0.000000f,
	    2.000000f, 0.000000f, 0.000000f,
	    1.000000f, 1.000000f, 0.000000f, // clang-format :-/
	};

	// 0 1 2
	// 3 5
	// 4

	unsigned int ib[] = {
	    0, 1, 3,
	    3, 1, 5,
	    1, 2, 5,
	    3, 5, 4,
	    1, 0, 1, // these two degenerate triangles create a fake reverse edge
	    0, 3, 0, // which breaks border classification
	};

	unsigned int expected[] = {
	    0, 1, 4,
	    4, 1, 2, // clang-format :-/
	};

	assert(meshopt_simplify(ib, ib, 18, vb, 6, 12, 3, 1e-3f) == 6);
	assert(memcmp(ib, expected, sizeof(expected)) == 0);
}

static void simplifyLockBorder()
{
	float vb[] = {
	    0.000000f, 0.000000f, 0.000000f,
	    0.000000f, 1.000000f, 0.000000f,
	    0.000000f, 2.000000f, 0.000000f,
	    1.000000f, 0.000000f, 0.000000f,
	    1.000000f, 1.000000f, 0.000000f,
	    1.000000f, 2.000000f, 0.000000f,
	    2.000000f, 0.000000f, 0.000000f,
	    2.000000f, 1.000000f, 0.000000f,
	    2.000000f, 2.000000f, 0.000000f, // clang-format :-/
	};

	// 0 1 2
	// 3 4 5
	// 6 7 8

	unsigned int ib[] = {
	    0, 1, 3,
	    3, 1, 4,
	    1, 2, 4,
	    4, 2, 5,
	    3, 4, 6,
	    6, 4, 7,
	    4, 5, 7,
	    7, 5, 8, // clang-format :-/
	};

	unsigned int expected[] = {
	    0, 1, 3,
	    1, 2, 3,
	    3, 2, 5,
	    6, 3, 7,
	    3, 5, 7,
	    7, 5, 8, // clang-format :-/
	};

	assert(meshopt_simplify(ib, ib, 24, vb, 9, 12, 3, 1e-3f, meshopt_SimplifyLockBorder) == 18);
	assert(memcmp(ib, expected, sizeof(expected)) == 0);
}

static void adjacency()
{
	// 0 1/4
	// 2/5 3
	const float vb[] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0};
	const unsigned int ib[] = {0, 1, 2, 5, 4, 3};

	unsigned int adjib[12];
	meshopt_generateAdjacencyIndexBuffer(adjib, ib, 6, vb, 6, 12);

	unsigned int expected[] = {
	    // patch 0
	    0, 0,
	    1, 3,
	    2, 2,

	    // patch 1
	    5, 0,
	    4, 4,
	    3, 3,

	    // clang-format :-/
	};

	assert(memcmp(adjib, expected, sizeof(expected)) == 0);
}

static void tessellation()
{
	// 0 1/4
	// 2/5 3
	const float vb[] = {0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0};
	const unsigned int ib[] = {0, 1, 2, 5, 4, 3};

	unsigned int tessib[24];
	meshopt_generateTessellationIndexBuffer(tessib, ib, 6, vb, 6, 12);

	unsigned int expected[] = {
	    // patch 0
	    0, 1, 2,
	    0, 1,
	    4, 5,
	    2, 0,
	    0, 1, 2,

	    // patch 1
	    5, 4, 3,
	    2, 1,
	    4, 3,
	    3, 5,
	    2, 1, 3,

	    // clang-format :-/
	};

	assert(memcmp(tessib, expected, sizeof(expected)) == 0);
}

void runTests()
{
	decodeIndexV0();
	decodeIndexV1();
	decodeIndex16();
	encodeIndexMemorySafe();
	decodeIndexMemorySafe();
	decodeIndexRejectExtraBytes();
	decodeIndexRejectMalformedHeaders();
	decodeIndexRejectInvalidVersion();
	decodeIndexMalformedVByte();
	roundtripIndexTricky();
	encodeIndexEmpty();

	decodeIndexSequence();
	decodeIndexSequence16();
	encodeIndexSequenceMemorySafe();
	decodeIndexSequenceMemorySafe();
	decodeIndexSequenceRejectExtraBytes();
	decodeIndexSequenceRejectMalformedHeaders();
	decodeIndexSequenceRejectInvalidVersion();
	encodeIndexSequenceEmpty();

	decodeVertexV0();
	encodeVertexMemorySafe();
	decodeVertexMemorySafe();
	decodeVertexRejectExtraBytes();
	decodeVertexRejectMalformedHeaders();
	decodeVertexBitGroups();
	decodeVertexBitGroupSentinels();
	decodeVertexLarge();
	encodeVertexEmpty();

	decodeFilterOct8();
	decodeFilterOct12();
	decodeFilterQuat12();
	decodeFilterExp();

	encodeFilterOct8();
	encodeFilterOct12();
	encodeFilterQuat12();
	encodeFilterExp();

	clusterBoundsDegenerate();

	customAllocator();

	emptyMesh();

	simplifyStuck();
	simplifySloppyStuck();
	simplifyPointsStuck();
	simplifyFlip();
	simplifyScale();
	simplifyDegenerate();
	simplifyLockBorder();

	adjacency();
	tessellation();
}
