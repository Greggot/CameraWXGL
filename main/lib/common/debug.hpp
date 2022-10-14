#pragma once

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

template<class ptr>
void outAddress(const char* Description, ptr* Data)
{
    printf("%s - 0x%08X\n", Description, (unsigned int)Data);
}

template <class type>
void outRaw(const char* Description, const type& Data, size_t Size = sizeof(type))
{
    printf("%s:\n", Description);
    
    uint8_t* data = (uint8_t*)&Data;
    for(size_t i = 0; i < Size; ++i)
        printf("%02X ", data[i]);
    
    printf("\n");
}

void out(const char* Description, const void* Data, size_t Size);
void outFreeSpace(const char* Description);

void FreeMeasureStart();
void FreeMeasureEnd(const char* Description);