#pragma once
#include <cstdint>
#include <esp_camera.h>

namespace Camera
{
    class interface
    {
    public:
        virtual void TakePicture() = 0;
        
        virtual const size_t size() const = 0;
        virtual const void* picture(size_t = 0) const = 0;
    };

    class AIThinker : public interface
    {
    private:
        camera_config_t config;
        camera_fb_t *pic;
    public:
        AIThinker(pixformat_t, framesize_t);

        void TakePicture() override {
            pic = esp_camera_fb_get();
            esp_camera_fb_return(pic);
        }
        
        const size_t size() const override { return pic->len; }
        const void* picture(size_t start = 0) const override { return &pic->buf[start]; }
    };
}