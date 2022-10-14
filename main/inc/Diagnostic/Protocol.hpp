#include <TCP/client.hpp>
#include <CAN/interface.hpp>

#include <ServiceProtocol.hpp>
#include <Compression.hpp>

#if DEVICE
#define TCP_BUFFER_SIZE 1300
#else
#define TCP_BUFFER_SIZE 4096
#endif

#define TCP_HEADER_SIZE 8
#define TCP_ARG_SIZE (TCP_BUFFER_SIZE - TCP_HEADER_SIZE)

namespace Diagnostic
{
    enum status : uint8_t
    {
        ok,
        inProcess,
        finish,
        
        notification,
        
        error,
    };

    enum command : uint8_t
    {
        // Butterfly/Mother -> Protocol
        uds,
        canraw,
        identify,

        // CAN::Controller -> Protocol
        settingsGET,
        settingsSET,
        settingsSAVE,
        autobaud,
        
        // Protocol
        echo,
        restart,

        // Butterfly/Mother -> Protocol
        diagnosticID,

        amount
    };

    struct Buffer
    {
        int16_t length;
        command service;
        uint8_t condition;
        uint32_t ID;
        uint8_t argument[TCP_ARG_SIZE] = {0};
    };

    class Protocol : TCP::client, public ServiceProtocol<command, Buffer>
    {
    private:
        const uint32_t ID;
        Buffer recvbuff;

        bool notificationReceived = false;

        bool messageIsNotification() const;
        bool messageIsFinished() const;
        bool rxCompressed() const { return rx.condition >> 7; }
    public:
        Protocol(const uint32_t ID);

        void connect(TCP::Address);

        template<class type>
        void transmit(command cmd, const type& data, status st = ok, size_t size = sizeof(type)) {
            tx.condition = st;
            transmit(cmd, &data, size);
        }
        void transmit (command, const void*, size_t) override;
        bool receive() override;
        void confirm();
        void waitConfirm();

        void flush() override;
    };
}