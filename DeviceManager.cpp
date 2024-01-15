#include "DeviceManager.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <fstream>

#include <cabl/gfx/TextDisplay.h>

namespace {
    const sl::Color COLOR_ON{0xff};
    const sl::Color COLOR_OFF{0x00};

    const double PAD_THRESHOLD_ON = 0.04;
    const double PAD_THRESHOLD_OFF = 0.01;
    const double PAD_VELOCITY_MOMENTUM = 0.8;
    const double PAD_VELOCITY_EXPONENT = 0.6;
    const std::chrono::milliseconds PAD_NOTE_ON_DELAY(5);
    const std::chrono::milliseconds PAD_NOTE_OFF_DELAY(10);

    const double ENCODER_MULTIPLIER = 32.0;

    const unsigned char START_NOTE = 36;
    const unsigned char MIDI_NOTE_ON = 144;
    const unsigned char MIDI_NOTE_OFF = 128;
    const unsigned char MIDI_NOTE_AFTER_TOUCH = 160;
    const unsigned char MIDI_NOTE_CC = 176;

    const unsigned char CC[11] = {3, 9, 14, 15, 20, 21, 22, 23, 24, 25, 26};
}

namespace sl {
using namespace cabl;
using namespace std::placeholders;
using namespace boost::asio;

DeviceManager::DeviceManager(
        DiscoveryPolicy discoveryPolicy_,
        int midiPort,
        std::string oscServerAddress,
        int oscServerPort
    ) : Client(discoveryPolicy_) {
    try {
        midiOut = new RtMidiOut();
    }
    catch(RtMidiError &error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }

    unsigned nPorts = midiOut->getPortCount();
    if (nPorts == 0) {
        std::cout << "No MIDI output ports available!" << std::endl;
        std::cout << "On macOS, open Audio MIDI Setup.app and enable the IAC Driver." << std::endl;
        exit(EXIT_FAILURE);
    } else {
        std::cout << "Number of available MIDI output ports: " << nPorts << std::endl;
    }

    if (midiPort >= nPorts) {
        std::cout << "Invalid MIDI port number!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Opening port " << midiPort << std::endl;
    midiOut->openPort(midiPort);

    noteMessage.resize(3, 0);
    ccMessage.resize(3, 0);
}

DeviceManager::~DeviceManager() {
    delete midiOut;
}

void DeviceManager::initDevice() {
}

void DeviceManager::render() {
}

void DeviceManager::buttonChanged(Device::Button button_, bool buttonState_, bool shiftState_) {
    device()->setButtonLed(button_, buttonState_ ? COLOR_ON : COLOR_OFF);
    requestDeviceUpdate();
}

void DeviceManager::encoderChanged(unsigned encoder_, bool valueIncreased_, bool shiftPressed_) {
    // do nothing
}

void DeviceManager::encoderChangedRaw(unsigned encoder_, double delta_, bool shiftPressed_) {
    ccMessage[0] = MIDI_NOTE_CC;
    ccMessage[1] = CC[encoder_];

    if (delta_ > 0) {
        ccMessage[2] = static_cast<unsigned char>(
            std::max(65.0, std::min(127.0, 65.0 + delta_ * ENCODER_MULTIPLIER))
        );
    } else {
        ccMessage[2] = static_cast<unsigned char>(
            std::max(1.0, std::min(64.0, -delta_ * ENCODER_MULTIPLIER))
        );
    }

    midiOut->sendMessage(&ccMessage);
}

void DeviceManager::keyChanged(unsigned index_, double value_, bool shiftPressed_) {
    // do nothing
}

void DeviceManager::keyUpdated(unsigned index_, double value_, bool shiftPressed_) {
    PadAction action = processPadUpdate(index_, value_);

    if (action == PadAction::NO_ACTION) {
        return;
    }

    unsigned col = index_ % 4;
    unsigned row = index_ / 4;
    unsigned note = START_NOTE + 4 * (3 - row) + col;
    double velocity = std::pow(padVelocities[index_], PAD_VELOCITY_EXPONENT) * 127.0;

    if (action == PadAction::NOTE_AFTERTOUCH) {
        noteMessage[0] = MIDI_NOTE_AFTER_TOUCH;
        device()->setKeyLed(index_, {static_cast<uint8_t>(velocity)});
    } else if (action == PadAction::NOTE_ON) {
        noteMessage[0] = MIDI_NOTE_ON;
        device()->setKeyLed(index_, {static_cast<uint8_t>(velocity)});
    } else if (action == PadAction::NOTE_OFF) {
        noteMessage[0] = MIDI_NOTE_OFF;
        device()->setKeyLed(index_, {0x00});
    }

    noteMessage[1] = static_cast<unsigned char>(note);
    noteMessage[2] = static_cast<unsigned char>(velocity);

    midiOut->sendMessage(&noteMessage);
    requestDeviceUpdate();
}

PadAction DeviceManager::processPadUpdate(unsigned index_, double value_) {
    PadState currentState = padStates[index_];

    padVelocities[index_] = PAD_VELOCITY_MOMENTUM * padVelocities[index_] + (1.0 - PAD_VELOCITY_MOMENTUM) * value_;

    if (padVelocities[index_] < PAD_THRESHOLD_OFF) {
        if (currentState == PadState::OFF) {
            return PadAction::NO_ACTION;
        }

        if (currentState == PadState::ON) {
            padStates[index_] = PadState::ON_TO_OFF;
            padTimes[index_] = std::chrono::steady_clock::now();
            return PadAction::NOTE_AFTERTOUCH;
        }

        if (currentState == PadState::ON_TO_OFF) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - padTimes[index_]);
            if (elapsed < PAD_NOTE_OFF_DELAY) {
                return PadAction::NOTE_AFTERTOUCH;
            }

            padStates[index_] = PadState::OFF;
            return PadAction::NOTE_OFF;
        }

        if (currentState == PadState::OFF_TO_ON) {
            padStates[index_] = PadState::OFF;
            return PadAction::NO_ACTION;
        }
    }
    
    if (padVelocities[index_] > PAD_THRESHOLD_ON) {
        if (currentState == PadState::ON) {
            return PadAction::NOTE_AFTERTOUCH;
        }

        if (currentState == PadState::OFF) {
            padStates[index_] = PadState::OFF_TO_ON;
            padTimes[index_] = std::chrono::steady_clock::now();
            return PadAction::NO_ACTION;
        }

        if (currentState == PadState::ON_TO_OFF) {
            padStates[index_] = PadState::ON;
            return PadAction::NOTE_AFTERTOUCH;
        }

        if (currentState == PadState::OFF_TO_ON) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - padTimes[index_]);
            if (elapsed < PAD_NOTE_ON_DELAY) {
                return PadAction::NO_ACTION;
            }

            padStates[index_] = PadState::ON;
            return PadAction::NOTE_ON;
        }
    }

    return PadAction::NO_ACTION;
}
}
