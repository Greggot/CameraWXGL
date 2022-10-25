#pragma once
#include <stdint.h>
#include <cstring>
#include <cstdio>

namespace Compression
{
	using rle_t = uint16_t;

	union RLEHeader
	{
		uint8_t raw;
		struct _bit
		{
			uint8_t length : 7;
			uint8_t repeated : 1;
		} bit;

		RLEHeader(bool repeated, uint8_t length){
			bit.repeated = repeated;
			bit.length = length;
		}
		RLEHeader() { raw = 0; }
		RLEHeader(uint8_t value) { raw = value; }

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