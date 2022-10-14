#include <debug.hpp>
void outFreeSpace(const char* Description)
{
    printf("%s - %u\n", Description, xPortGetFreeHeapSize());
}

void out(const char* Description, const void* Data, size_t Size)
{
    printf("%s:\n", Description);
    
    uint8_t* data = (uint8_t*)Data;
    for(size_t i = 0; i < Size; ++i)
        printf("%02X ", data[i]);
    
    printf("\n");
}

static int freespace = 0;
void FreeMeasureStart()
{
    freespace = xPortGetFreeHeapSize();
}
void FreeMeasureEnd(const char* Description)
{
    static int measureEndSpace = xPortGetFreeHeapSize();
    printf("%s: %u - %u = %i\n", Description, freespace, measureEndSpace, freespace - measureEndSpace);
    
}