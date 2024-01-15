#pragma once

#include <chrono>
#include <cabl/cabl.h>
#include <RtMidi.h>
#include <boost/asio.hpp>

#define OSC_BUFFER_SIZE 1024

namespace sl {
using namespace cabl;
using namespace boost::asio;

enum class PadState: uint8_t
{
    ON,
    OFF,
    ON_TO_OFF,
    OFF_TO_ON,
};

enum class PadAction: uint8_t
{
    NO_ACTION,
    NOTE_ON,
    NOTE_OFF,
    NOTE_AFTERTOUCH,
};

class DeviceManager : public Client {
public:
    DeviceManager(
        DiscoveryPolicy = {},
        int midiPort = 0,
        std::string oscServerAddress = "",
        int oscServerPort = 0,
        bool invertLEDs = false
    );
    ~DeviceManager();

    void initDevice() override;
    void render() override;

    void buttonChanged(Device::Button button_, bool buttonState_, bool shiftState_) override;
    void encoderChanged(unsigned encoder_, bool valueIncreased_, bool shiftPressed_) override;
    void encoderChangedRaw(unsigned encoder_, double delta_, bool shiftPressed_) override;
    void keyChanged(unsigned index_, double value_, bool shiftPressed) override;
    void keyUpdated(unsigned index_, double value_, bool shiftPressed) override;

private:
    PadAction processPadUpdate(unsigned index_, double value_);

    void sendOSCMessage(std::string address, std::string value);

    PadState padStates[16] = {PadState::OFF};
    double padVelocities[16] = {0.0};
    std::chrono::steady_clock::time_point padTimes[16];

    RtMidiOut *midiOut = 0;
    std::vector<unsigned char> noteMessage;
    std::vector<unsigned char> ccMessage;

    io_service oscIOService;
    ip::udp::endpoint oscRemoteEndpoint;
    ip::udp::socket oscSocket = ip::udp::socket(oscIOService);

    char oscBuffer[OSC_BUFFER_SIZE] = {0};

    bool buttonShiftState[static_cast<unsigned>(Device::Button::Unknown)] = {false};

    bool invertLEDs = false;
};
}
