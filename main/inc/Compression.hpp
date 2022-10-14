#pragma once
#include <stdint.h>
#include <cstring>
#include <cstdio>

namespace Compression
{
	typedef uint8_t rle_t;

	union RLEHeader
	{
		rle_t raw;
		struct _bit
		{
			rle_t length : 7;
			rle_t repeated : 1;
		} bit;

		RLEHeader(bool repeated, rle_t length){
			bit.repeated = repeated;
			bit.length = length;
		}
		RLEHeader() { raw = 0; }
		RLEHeader(rle_t value) { raw = value; }

	};

	enum Sequence : uint8_t
	{
		original,
		repeated,
	};

	size_t RLE(void* destination, const void* source, size_t size);
}

namespace Decompression
{
	size_t RLE(void* destination, const void* source, size_t size);
}