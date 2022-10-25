#include "Compression.hpp"
using namespace Compression;

static inline void InitializeSequence(RLEHeader** headerptr, uint8_t* destination, size_t& index)
{
	RLEHeader* header = *headerptr;

	header = (RLEHeader*)&destination[index];
	header->bit.length = 0;
	index += sizeof(RLEHeader);

	*headerptr = header;
}

static inline void FinishSequence(RLEHeader** headerptr, uint8_t* destination, size_t& index, bool repeated)
{
	(*headerptr)->bit.repeated = repeated;
		// printf(repeated ? "\tRepeated finish with length %u\n" : "\tOriginal finish with length %u\n", (*headerptr)->bit.length);
	InitializeSequence(headerptr, destination, index);
}

static inline bool headerIsOverflowed(const RLEHeader& header)
{
	static const RLEHeader cmp(-1);
	return header.bit.length == cmp.bit.length;
}

/**
* @brief Single pass RLE compression
* Copy source with parallel RLE header change via
* pointer to an area inside destination array.
* After copying part of array, RLEHeader pointer moves along
*/
size_t Compression::RLE(void* destination, const void* source, size_t size)
{
	rle_t* src = (rle_t*)source;
	uint8_t* dst = (uint8_t*)destination;

	RLEHeader* header;
	size_t index = 0;
	InitializeSequence(&header, dst, index);

	for (size_t i = 0; i < size; ++i)
	{
		if (src[i] != src[i + 1])
		{
			memcpy(&dst[index], &src[i], sizeof(rle_t));
			index += sizeof(rle_t);
			++header->bit.length;
			if (headerIsOverflowed(*header))
				FinishSequence(&header, dst, index, original);
		}
		else
		{
			if (header->bit.length)
				FinishSequence(&header, dst, index, original);
			
			rle_t value = src[i];
			memcpy(&dst[index], &value, sizeof(rle_t));
			index += sizeof(rle_t);
			while (src[i++] == value)
			{
				++header->bit.length;
				if (headerIsOverflowed(*header))
				{
					FinishSequence(&header, dst, index, repeated);
					memcpy(&dst[index], &value, sizeof(rle_t));
					index += sizeof(rle_t);
				}
			}
			i -= 2;
			
			if (header->bit.length)
				FinishSequence(&header, dst, index, repeated);
		}
	}
	return index - sizeof(rle_t);
}

size_t Decompression::RLE(void* destination, const void* source, size_t size)
{
	uint8_t* src = (uint8_t*)source;
	rle_t* dst = (rle_t*)destination;

	size_t i = 0;
	size_t j = 0;
	RLEHeader header;

	while (i < size)
	{
		memcpy(&header, &src[i++], sizeof(header));
		while (header.bit.length--)
		{
			memcpy(&dst[j++], &src[i], sizeof(rle_t));
			if(!header.bit.repeated)
				i += sizeof(rle_t);
		}
		if(header.bit.repeated)
			i += sizeof(rle_t);
	}
	return j;
}
