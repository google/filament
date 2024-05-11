#include <stddef.h>
#include <stdint.h>
#include <assert.h>

extern unsigned char __heap_base;
static intptr_t sbrkp = intptr_t(&__heap_base);

static const int WASM_PAGE_SIZE = 64 * 1024;

extern "C" void* sbrk(intptr_t increment)
{
	intptr_t sbrko = sbrkp;

	increment = (increment + 3) & ~3;
	sbrkp += increment;

	size_t heap_size = __builtin_wasm_memory_size(0) * WASM_PAGE_SIZE;

	if (sbrkp > heap_size)
	{
		size_t diff = (sbrkp - heap_size + WASM_PAGE_SIZE - 1) / WASM_PAGE_SIZE;

		if (__builtin_wasm_memory_grow(0, diff) == size_t(-1))
			return (void*)-1;
	}

	return (void*)sbrko;
}

extern "C" void* memcpy(void* destination, const void* source, size_t num)
{
	char* d = (char*)destination;
	const char* s = (const char*)source;

	if (((uintptr_t(d) | uintptr_t(s)) & 3) == 0)
	{
		while (num > 15)
		{
			((uint32_t*)d)[0] = ((uint32_t*)s)[0];
			((uint32_t*)d)[1] = ((uint32_t*)s)[1];
			((uint32_t*)d)[2] = ((uint32_t*)s)[2];
			((uint32_t*)d)[3] = ((uint32_t*)s)[3];
			d += 16;
			s += 16;
			num -= 16;
		}

		while (num > 3)
		{
			((uint32_t*)d)[0] = ((uint32_t*)s)[0];
			d += 4;
			s += 4;
			num -= 4;
		}
	}

	while (num > 0)
	{
		*d++ = *s++;
		num--;
	}

	return destination;
}

extern "C" void* memset(void* ptr, int value, size_t num)
{
	uint32_t v32 = ~0u / 255 * uint8_t(value);

	char* d = (char*)ptr;

	if ((uintptr_t(d) & 3) == 0)
	{
		while (num > 15)
		{
			((uint32_t*)d)[0] = v32;
			((uint32_t*)d)[1] = v32;
			((uint32_t*)d)[2] = v32;
			((uint32_t*)d)[3] = v32;
			d += 16;
			num -= 16;
		}

		while (num > 3)
		{
			((uint32_t*)d)[0] = v32;
			d += 4;
			num -= 4;
		}
	}

	while (num > 0)
	{
		*d++ = char(value);
		num--;
	}

	return ptr;
}

void* operator new(size_t size)
{
	return sbrk((size + 7) & ~7);
}

void operator delete(void* ptr) throw()
{
	void* brk = sbrk(0);
	assert(ptr <= brk);

	sbrk((char*)ptr - (char*)brk);
}
