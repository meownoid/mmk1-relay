#pragma once

#include <cabl/cabl.h>

namespace sl {
using namespace cabl;

class DeviceManager : public Client {
public:
    void initDevice() override;
    void render() override;

    void buttonChanged(Device::Button button_, bool buttonState_, bool shiftState_) override;
    void encoderChanged(unsigned encoder_, bool valueIncreased_, bool shiftPressed_) override;
    void keyChanged(unsigned index_, double value_, bool shiftPressed) override;
};
}
