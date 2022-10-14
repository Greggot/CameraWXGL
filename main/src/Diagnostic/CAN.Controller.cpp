#include <Diagnostic/CAN.Controller.hpp>
using namespace Diagnostic;

controller::controller(CAN::Interface& can, Protocol& api)
    : api(api), can(can)
{
    can.load();
    can.start();

    apiSetup();
}   

void controller::apiSetup()
{
    api[command::settingsGET] = [this](const Buffer&){
        api.transmit(settingsGET, can.getSettings());
    };

    api[command::settingsSAVE] = [this](const Buffer&) {
        can.save(); 
        api.confirm();
    };

    api[command::settingsSET] = [this](const Buffer& request){
        can.set(*(CAN::Settings*)request.argument); 
        api.confirm();
    };

    api[command::autobaud] = [this](const Buffer&){
        api.transmit(command::autobaud, autobaud());
    };
}


uint16_t controller::autobaud() const
{
    static const std::list<uint16_t> Speeds = { 125, 250, 500, 800, 1000 };

    for(auto speed : Speeds)
        if(isCorrect(speed))
            return speed;

    return 0;
}

inline bool controller::isCorrect(uint16_t speed) const
{
    can.set(speed);
    for(size_t i = 0; i < TX_FIFO_LEN * 2; ++i)
    {
        if(can.send(0x18FFFE00, &speed, sizeof(speed)))
            return false;
        vTaskDelay(pdMS_TO_TICKS(25));
    }
    return true;
}