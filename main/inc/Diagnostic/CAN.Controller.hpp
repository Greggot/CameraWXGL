#include "Protocol.hpp"

namespace Diagnostic
{
    class controller
    {
    private:
        Protocol& api;
        CAN::Interface& can;

        void apiSetup();

        uint16_t autobaud() const;
        bool isCorrect(uint16_t) const;
    public:
        controller(CAN::Interface& can, Protocol& api);
    };
} 
