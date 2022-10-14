#include <build.h>

#include <Bluetooth/Server.hpp>
#include <TCP/Client.hpp>
#include <WiFi/STA.hpp>

#include "esp_camera.h"

#define BOARD_WROVER_KIT
#define BOARD_ESP32CAM_AITHINKER

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    // .pixel_format = PIXFORMAT_RGB565, //YUV422,GRAYSCALE,RGB565,JPEG
    .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QVGA,    //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

    .jpeg_quality = 12, //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 1,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

#include <string>

static esp_err_t init_camera()
{
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        printf("Camera Init Failed\n");
        return err;
    }

    return ESP_OK;
}

extern "C" void app_main(void)
{
    nvs_flash_init();

    if(ESP_OK != init_camera())
        return;

    static TCP::client TCP;
    static WiFi::STA sta;

    static auto NetReporter = BLE::Characteristic(0x4ED, BLE::permition::Write, BLE::property::Write | BLE::property::Notify);
    static auto NetworkController = BLE::Service(0xDCBC, {&NetReporter});

    std::string name = "CameraWXGL";
    static BLE::Server server(name.c_str(), { &NetworkController });
    BLE::Server::Enable();

    sta.add(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
    [](void*, esp_event_base_t, int32_t, void*){
        if(BLE::Server::Status())
            NetReporter.Notify(WiFi::Status::STAconnectError);
        else
            sta.TurnOffFullPower();
        BLE::Server::Enable();
    });

    sta.add(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED,
    [](void*, esp_event_base_t, int32_t, void*){
        NetReporter.Notify(WiFi::Status::STAconnectSuccessful);
    });

    static TCP::Address address;
    sta.add(IP_EVENT, IP_EVENT_STA_GOT_IP, [](void*, esp_event_base_t, int32_t, void*){
        printf("Connecting to %u.%u.%u.%u:%u\n", address.IP[0],address.IP[1],address.IP[2],address.IP[3], 
            address.Port);
        if(TCP.Connect(address))
        {    
            NetReporter.Notify(WiFi::Status::SocketSuccessful);
            BLE::Server::Disable();
            WiFi::STA::TurnOnFullPower();

            xTaskCreate([](void*){
                while (true)
                {
                    printf("Taking picture...\n");
                    camera_fb_t *pic = esp_camera_fb_get();

                    // use pic->buf to access the image
                    printf("Picture taken! Its size was: %zu bytes\n", pic->len);
                    esp_camera_fb_return(pic);
                
                    // printf("Image data: ");
                    // for(size_t i = 0; i < pic->len; ++i)
                    //     printf("%02X ", pic->buf[i]);
                    // printf("\n");

                    TCP.Send(pic->buf, pic->len);
                    vTaskDelay(5000 / portTICK_RATE_MS);
                }
            }, "CameraThread", 3096, nullptr, 0, nullptr);
        }
        else
        {
            NetReporter.Notify(WiFi::Status::SocketError);
            sta.disconnect();
        }
    });

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