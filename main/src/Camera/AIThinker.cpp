#include <Camera/Camera.hpp>
using namespace Camera;

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

AIThinker::AIThinker(pixformat_t format, framesize_t framesize)
{
    config.pin_pwdn = CAM_PIN_PWDN;
    config.pin_reset = CAM_PIN_RESET;
    config.pin_xclk = CAM_PIN_XCLK;
    config.pin_sccb_sda = CAM_PIN_SIOD;
    config.pin_sccb_scl = CAM_PIN_SIOC;

    config.pin_d7 = CAM_PIN_D7;
    config.pin_d6 = CAM_PIN_D6;
    config.pin_d5 = CAM_PIN_D5;
    config.pin_d4 = CAM_PIN_D4;
    config.pin_d3 = CAM_PIN_D3;
    config.pin_d2 = CAM_PIN_D2;
    config.pin_d1 = CAM_PIN_D1;
    config.pin_d0 = CAM_PIN_D0;
    config.pin_vsync = CAM_PIN_VSYNC;
    config.pin_href = CAM_PIN_HREF;
    config.pin_pclk = CAM_PIN_PCLK;

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    config.xclk_freq_hz = 20000000;
    config.ledc_timer = LEDC_TIMER_0;
    config.ledc_channel = LEDC_CHANNEL_0;

    config.pixel_format = format; //YUV422;GRAYSCALE;RGB565;JPEG
    config.frame_size = framesize;    //QQVGA-UXGA; For ESP32; do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot; but JPEG mode always gives better frame rates.

    config.jpeg_quality = 12; //0-63; for OV series camera sensors; lower number means higher quality
    config.fb_count = 1;       //When jpeg mode is used; if fb_count more than one; the driver will work in continuous mode.
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    if(esp_camera_init(&config))
        printf("Camera Init Failed\n");
}