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
	uint8_t* src = (uint8_t*)source;
	uint8_t* dst = (uint8_t*)destination;

	RLEHeader* header;
	size_t index = 0;
	InitializeSequence(&header, dst, index);

	for (size_t i = 0; i < size; ++i)
	{
		if (src[i] != src[i + 1])
		{
			dst[index++] = src[i];
			++header->bit.length;
			if (headerIsOverflowed(*header))
				FinishSequence(&header, dst, index, original);
		}
		else
		{
			if (header->bit.length)
				FinishSequence(&header, dst, index, original);
			
			uint8_t byte = src[i];
			dst[index++] = byte;
			while (src[i++] == byte)
			{
				++header->bit.length;
				if (headerIsOverflowed(*header))
				{
					FinishSequence(&header, dst, index, repeated);
					dst[index++] = byte;
				}
			}
			i -= 2;
			
			if (header->bit.length)
				FinishSequence(&header, dst, index, repeated);
		}

		if (index >= size - sizeof(RLEHeader))
			return index;
	}
	return index - sizeof(RLEHeader);
}

size_t Decompression::RLE(void* destination, const void* source, size_t size)
{
	uint8_t* src = (uint8_t*)source;
	uint8_t* dst = (uint8_t*)destination;

	size_t i = 0;
	size_t j = 0;
	RLEHeader header;

	while (i < size)
	{
		memcpy(&header, &src[i++], sizeof(header));
		uint8_t byte = src[i];
		while (header.bit.length--)
			dst[j++] = header.bit.repeated ? byte : src[i++];
		if(header.bit.repeated)
			++i;
	}
	return j;
}
