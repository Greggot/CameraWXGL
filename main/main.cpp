#include <build.h>

#include <Bluetooth/Server.hpp>
#include <TCP/client.hpp>
#include <UDP/client.hpp>
#include <WiFi/STA.hpp>

#include <Camera/Camera.hpp>

#include <SkyBlue/Device.hpp>
#include <SkyBlue/Collector.hpp>
#include <SkyBlue/InterfaceTraits.hpp>

extern "C" void app_main(void)
{
    nvs_flash_init();

    static WiFi::STA sta;
    static auto NetReporter = BLE::Characteristic(0x4ED, BLE::permition::Write, BLE::property::Write | BLE::property::Notify);
    static auto NetworkController = BLE::Service(0xDCBC, { &NetReporter });

    std::string name = "CameraWXGL";
    static BLE::Server server(name.c_str(), { &NetworkController });
    BLE::Server::Enable();

    static Camera::AIThinker camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA);
    static SkyBlue::TCPserverDevice device;
    static UDP::client udpclient;
    
    // TODO: add class MyCameraModule : public SkyBlue::Module, which will define R/W callbacks
    //      automatically
    auto cameramodule = new SkyBlue::Module;
    cameramodule->setRead([](const SkyBlue::ID& id, const void*, size_t){
        camera.TakePicture();
        auto ptr = (uint8_t*)camera.picture();
        auto picsize = camera.size();
        printf("Taken picture, size(%u)...\n", picsize);

        static const size_t winlen = 320 * 4;
        static uint8_t tcpwindow[1440];
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
        device.write(id, nullptr, 0);
    });

    auto rotormodule = new SkyBlue::Module;
    rotormodule->setWrite([](const SkyBlue::ID& id, const void* data, size_t){
        struct vertex{
            float x; float y; float z;
        };
        vertex input;
        memcpy(&input, data, sizeof(vertex));
        printf("Rotor %u received vertex x:%.2f, y:%.2f, z:%.2f\n", id.number, 
            input.x, input.y, input.z);
    });

    // TODO: add static array of ids to increment it automatically
    //      and `add(type_t, Module*)` polymorph method
    device.add({0, SkyBlue::type_t::camera}, cameramodule);
    device.add({0, SkyBlue::type_t::rotorservo}, rotormodule);
    device.add({1, SkyBlue::type_t::rotorservo}, rotormodule);

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
        address = {event->ip_info.ip.addr, 4444};
        printf("Starting %u.%u.%u.%u:%u\n", address.IP[0],address.IP[1],address.IP[2],address.IP[3], 
            address.Port);

        // Gotta turn off BLE before enabling full power
        BLE::Server::Disable();
        WiFi::STA::TurnOnFullPower();

        // Start server, begin work after first connection
        device.connect(address);
        device.listen();
        auto client = device.otherside();
        printf("  Client connected! %u.%u.%u.%u\n", client.IP[0], client.IP[1], client.IP[2], client.IP[3]);
        
        udpclient.Connect({client.IP, 5555});
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