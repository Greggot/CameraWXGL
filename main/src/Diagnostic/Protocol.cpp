#include <Diagnostic/Protocol.hpp>
using namespace Diagnostic;

Protocol::Protocol(const uint32_t ID)
    : ID(ID)
{
    tx.ID = ID;
}

void Protocol::connect(TCP::Address address)
{
    if(!Connect(address))
        return;
    
    xTaskCreate([](void* arg){
        Protocol api = *(Protocol*)arg;
        while(true)
        {
            if(!api.receive())
                continue;
                
            api.call();
            api.flush();
        }
    }, "MainThread", 7680, this, 3, nullptr);
}

void Protocol::transmit(command service, const void* data, size_t size)
{
    if(size > TCP_ARG_SIZE)
    {    
        tx.length = Compression::RLE(tx.argument, data, size);
        printf("Compression: %.2f%%\n", ((float)tx.length / size) * 100);
        
        static const uint8_t compressionflag = 0x80;
        while(size > TCP_ARG_SIZE)
        {
            tx.condition = inProcess | compressionflag;
            Send(&tx, TCP_ARG_SIZE);
            size -= TCP_ARG_SIZE;
            
            waitConfirm();
        }
        Send(&tx, TCP_ARG_SIZE);
        tx.condition = finish | compressionflag;
    }
    else
    {
        tx.service = service;
        tx.length = size > TCP_ARG_SIZE ? TCP_ARG_SIZE : size;
        memcpy(tx.argument, data, tx.length);
        Send(&tx, tx.length + TCP_HEADER_SIZE);
    }
    printf("  Send %u bytes\n", size);
}

void Protocol::waitConfirm()
{
    while(!notificationReceived)
        vTaskDelay(pdMS_TO_TICKS(10));
    notificationReceived = false;
}

bool Protocol::receive() 
{ 
    rx.length = Receive(&rx, sizeof(rx));
    if(rx.length < 0)
        return false;
    tx.service = rx.service;

    if(rx.condition == notification && rx.length == TCP_HEADER_SIZE)
    {
        notificationReceived = true;
        return true;
    }

    static uint32_t position = 0;

    size_t length  = rx.length;
    if(rxCompressed())
    {
        length = Decompression::RLE(recvbuff.argument, rx.argument, rx.length + TCP_HEADER_SIZE);
        printf("Decompressed %u into %u\n", rx.length, length);
    }

    switch(rx.condition & 0x7F)
    {
    case inProcess:
        if(!rxCompressed())
            memcpy(&recvbuff.argument[position], rx.argument, rx.length);

        recvbuff.length += length;
        position += length;
        confirm();
        break;
    case finish:
        if(!rxCompressed())
            memcpy(&recvbuff.argument[position], rx.argument, rx.length);
        
        recvbuff.length += length;
        position = 0;
        break;
    case ok:
        recvbuff.condition = ok;
        recvbuff.length = length;
        recvbuff.service = rx.service;
        if(!rxCompressed())
            memcpy(recvbuff.argument, rx.argument, rx.length);
        break;
    }

    return true;
}

void Protocol::confirm()
{
    tx.condition = notification;
    tx.length = 0;
    Send(&tx, TCP_HEADER_SIZE);
}

void Protocol::flush() 
{
    for(int i = 0; i < rx.length; ++i)
        rx.argument[i] = 0;
    rx.length = 0;
}