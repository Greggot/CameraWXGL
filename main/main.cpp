#include <build.h>

#include <Bluetooth/Server.hpp>
#include <TCP/client.hpp>
#include <UDP/client.hpp>
#include <WiFi/STA.hpp>

#include <Camera/Camera.hpp>
#include <SkyBlue/Module.hpp>
#include <SkyBlue/API.hpp>

extern "C" void app_main(void)
{
    nvs_flash_init();

    static TCP::client TCP;
    static TCP::server tcpserver;
    static UDP::client udpclient;
    static WiFi::STA sta;

    static auto NetReporter = BLE::Characteristic(0x4ED, BLE::permition::Write, BLE::property::Write | BLE::property::Notify);
    static auto NetworkController = BLE::Service(0xDCBC, {&NetReporter});

    std::string name = "CameraWXGL";
    static BLE::Server server(name.c_str(), { &NetworkController });
    BLE::Server::Enable();

    static Camera::AIThinker camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA);
    static SkyBlue::TCPserverModule cameramodule({0, SkyBlue::type_t::camera}, tcpserver);
    static SkyBlue::TCPserverModule theta({0, SkyBlue::type_t::rotorservo}, tcpserver);
    static SkyBlue::TCPserverModule omega({1, SkyBlue::type_t::rotorservo}, tcpserver);
    static SkyBlue::TCPserverModule hand({0, SkyBlue::type_t::linearservo}, tcpserver);

    static SkyBlue::TCPserverAPI api({&cameramodule, &theta, &omega, &hand}, tcpserver);
    
    static uint8_t* ptr;
    static size_t picsize = 0;
    cameramodule.setTX([](const void* data, size_t size){
        struct position {
            size_t start;
            size_t length;
        };
        position pos = *(position*)data;
        if(pos.start + pos.length > picsize)
            return;
        cameramodule.write(&ptr[pos.start], pos.length); 
    });

    static const size_t winlen = 320 * 4;
    static uint8_t tcpwindow[1440];
    cameramodule.setRX([](const void*, size_t){
        camera.TakePicture();
        ptr = (uint8_t*)camera.picture();
        picsize = camera.size();
        printf("Taken picture, size(%u)...\n", picsize);

        struct position {
            size_t start;
            size_t length;
        };
        for(position pos{0, winlen}; pos.start < picsize; pos.start += pos.length )
        {
            memcpy(tcpwindow, &pos, sizeof(pos));
            memcpy(tcpwindow + sizeof(pos), &ptr[pos.start], pos.length);
            udpclient.Send(tcpwindow, sizeof(pos) + pos.length);
        }
        #define testmessage "test"
        cameramodule.write(testmessage, sizeof(testmessage));
    });

    sta.add(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
    [](void*, esp_event_base_t, int32_t, void*){
        // BLE wasn't disabled - it means unsuccessfull IP_EVENT_STA_GOT_IP finish
        // Otherwise, turn off full wifi power to be able to turn on BLE again
        if(BLE::Server::Status())
            NetReporter.Notify(WiFi::Status::STAconnectError);
        else
            sta.TurnOffFullPower();
        BLE::Server::Enable();
    });

    // Tell BLE application that wifi connection established
    sta.add(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED,
    [](void*, esp_event_base_t, int32_t, void*){
        NetReporter.Notify(WiFi::Status::STAconnectSuccessful);
    });

    // Use acquired ip from STA to open socket on port - 4444        
    static TCP::Address address;
    sta.add(IP_EVENT, IP_EVENT_STA_GOT_IP, [](void*, esp_event_base_t, int32_t, void* data){        
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) data;
        memcpy(address.IP, &event->ip_info.ip, 4);
        address.Port = 4444;
        printf("Starting %u.%u.%u.%u:%u\n", address.IP[0],address.IP[1],address.IP[2],address.IP[3], 
            address.Port);

        // Gotta turn off BLE before enabling full power
        BLE::Server::Disable();
        WiFi::STA::TurnOnFullPower();

        // Start server, begin work after first connection
        tcpserver.Start(address);
        tcpserver.Accept();
        printf("  Client connected!\n");
        
        udpclient.Connect({{192, 168, 1, 134}, 5555});
        api.start();
    });

    // Acquire ssid-password ip:port to work with
    NetReporter.setWriteCallback([](BLE::Characteristic*, const uint16_t length, const void* value){
        char* ssid = (char*)value;
        char* password = (char*)value;        

        uint16_t index = 0;
        while(++index < length && *++password != 0);
        ++password;

        memcpy(&address, (uint8_t*)value + (length - sizeof(address)), sizeof(address));
        sta.connect(ssid, password);
    });
}