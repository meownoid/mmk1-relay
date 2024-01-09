#include "DeviceManager.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include <cabl/gfx/TextDisplay.h>

namespace {
const sl::Color COLOR_OFF{0};
const sl::Color COLOR_ON{0xff, 0xff, 0xff, 0xff};
}

namespace sl {
using namespace cabl;
using namespace std::placeholders;

void DeviceManager::initDevice() {
    for(unsigned i  = 0; i< device()->numOfGraphicDisplays(); i++) {
        device()->graphicDisplay(i)->black();
    }
}

void DeviceManager::render() {
    for(unsigned i  = 0; i< device()->numOfGraphicDisplays(); i++) {
        unsigned w = device()->graphicDisplay(i)->width();
        unsigned h = device()->graphicDisplay(i)->height();
        for (unsigned x = 0; x < w; x++) {
            for (unsigned y = 0; y < h; y++) {
                device()->graphicDisplay(i)->setPixel(x, y, std::rand() % 2 == 0 ? COLOR_ON : COLOR_OFF);
            }
        }
  }
}

void DeviceManager::buttonChanged(Device::Button button_, bool buttonState_, bool shiftState_) {
  std::cout << "Button Changed (" << std::to_string((uint8_t)button_) << ", " << std::to_string(buttonState_) << ", " << std::to_string(shiftState_) << ")" << std::endl;
  device()->setButtonLed(
    button_, buttonState_ ? COLOR_ON : COLOR_OFF);
}

void DeviceManager::encoderChanged(unsigned encoder_, bool valueIncreased_, bool shiftPressed_) {
  std::cout << "Encoder Changed (" << std::to_string(encoder_) << ", " << std::to_string(valueIncreased_) << ", " << std::to_string(shiftPressed_) << ")" << std::endl;
  std::string value = "Enc#" + std::to_string(static_cast<int>(encoder_)) + ( valueIncreased_ ? " increased" : " decreased" );

  device()->textDisplay(0)->putText(value.c_str(), 0);

  device()->graphicDisplay(0)->black();
  device()->graphicDisplay(0)->putText(10, 10, value.c_str(), {0xff});
}

void DeviceManager::keyChanged(unsigned index_, double value_, bool shiftPressed_) {
  float keyThreshold = 0.1;

  if (value_ < keyThreshold) {
    value_ = 0.0;
  }

  std::cout << "Key Changed (" << std::to_string(index_) << ", " << std::to_string(value_) << ", " << std::to_string(shiftPressed_) << ")" << std::endl;
  device()->setKeyLed(index_, {static_cast<uint8_t>(value_ * 127)});
}
}
