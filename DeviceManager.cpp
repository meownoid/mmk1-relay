#include "DeviceManager.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <fstream>

#include <cabl/gfx/TextDisplay.h>

namespace {
    const sl::Color COLOR_ON{0xff};
    const sl::Color COLOR_OFF{0x00};

    const double PAD_THRESHOLD = 0.05;
    const double VELOCITY_MOMENTUM = 0.1;
    const double VELOCITY_EXPONENT = 0.5;

    const unsigned char START_NOTE = 36;
    const unsigned char MIDI_NOTE_ON = 144;
    const unsigned char MIDI_NOTE_OFF = 128;

    const std::chrono::milliseconds NOTE_ON_DELAY(5);
    const std::chrono::milliseconds NOTE_OFF_DELAY(5);
}

namespace sl {
using namespace cabl;
using namespace std::placeholders;

DeviceManager::DeviceManager(DiscoveryPolicy discoveryPolicy_) : Client(discoveryPolicy_) {
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

    midiOut->openPort(0);

    noteMessage.resize(3, 0);

    logTimes.reserve(10000);
    logValues.reserve(10000);
}

DeviceManager::~DeviceManager() {
    delete midiOut;
}

void DeviceManager::initDevice() {
}

void DeviceManager::render() {
}

void DeviceManager::buttonChanged(Device::Button button_, bool buttonState_, bool shiftState_) {
    typedef std::chrono::milliseconds ms;
    if ((button_ == Device::Button::GroupB) && buttonState_) {
        std::cout << "Writing log file" << std::endl;
        std::cout << "logValues.size() = " << logValues.size() << std::endl;
        std::ofstream outputFile("log.txt", std::ios::out);
        auto prevTime = std::chrono::duration_cast<ms>(logTimes[0].time_since_epoch()).count();
        for (unsigned i = 0; i < logTimes.size(); i++) {
            auto time = std::chrono::duration_cast<ms>(logTimes[i].time_since_epoch()).count() - prevTime;
            prevTime = std::chrono::duration_cast<ms>(logTimes[i].time_since_epoch()).count();
            auto value = logValues[i];
            if (outputFile.is_open()) {
                outputFile << time << "," << value << std::endl;
            } else {
                std::cout << "Failed to open log file!" << std::endl;
            }
        }
        outputFile.flush();
        outputFile.close();
    }

    device()->setButtonLed(button_, buttonState_ ? COLOR_ON : COLOR_OFF);
    requestDeviceUpdate();
}

void DeviceManager::encoderChanged(unsigned encoder_, bool valueIncreased_, bool shiftPressed_) {
    requestDeviceUpdate();
}

void DeviceManager::keyChanged(unsigned index_, double value_, bool shiftPressed_) {
}

void DeviceManager::keyUpdated(unsigned index_, double value_, bool shiftPressed_) {
    PadAction action = processPadUpdate(index_, value_);

    if (action == PadAction::NO_ACTION) {
        return;
    }

    unsigned char note = START_NOTE + static_cast<unsigned char>(index_);
    double velocity = padVelocities[index_] * 127.0;

    if (action == PadAction::NOTE_ON) {
        noteMessage[0] = MIDI_NOTE_ON;
        device()->setKeyLed(index_, {static_cast<uint8_t>(velocity)});
    } else if (action == PadAction::NOTE_OFF) {
        noteMessage[0] = MIDI_NOTE_OFF;
        device()->setKeyLed(index_, {0x00});
    }

    noteMessage[1] = note;
    noteMessage[2] = static_cast<unsigned char>(velocity);

    midiOut->sendMessage(&noteMessage);
    requestDeviceUpdate();
}

PadAction DeviceManager::processPadUpdate(unsigned index_, double value_) {
    bool isPressed = value_ > PAD_THRESHOLD;
    PadState currentState = padStates[index_];

    if (!isPressed) {
        if (currentState == PadState::OFF) {
            return PadAction::NO_ACTION;
        }

        if (currentState == PadState::ON) {
            padStates[index_] = PadState::ON_TO_OFF;
            padTimes[index_] = std::chrono::steady_clock::now();
            return PadAction::NO_ACTION;
        }

        if (currentState == PadState::ON_TO_OFF) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - padTimes[index_]);
            if (elapsed < NOTE_OFF_DELAY) {
                return PadAction::NO_ACTION;
            }

            padStates[index_] = PadState::OFF;
            return PadAction::NOTE_OFF;
        }

        if (currentState == PadState::OFF_TO_ON) {
            padStates[index_] = PadState::OFF;
            return PadAction::NO_ACTION;
        }
    } else {
        if (currentState == PadState::ON) {
            return PadAction::NO_ACTION;
        }

        if (currentState == PadState::OFF) {
            padStates[index_] = PadState::OFF_TO_ON;
            padVelocities[index_] = std::pow(value_, VELOCITY_EXPONENT);;
            padTimes[index_] = std::chrono::steady_clock::now();
            return PadAction::NO_ACTION;
        }

        if (currentState == PadState::ON_TO_OFF) {
            padStates[index_] = PadState::ON;
            return PadAction::NO_ACTION;
        }

        if (currentState == PadState::OFF_TO_ON) {
            padVelocities[index_] = VELOCITY_MOMENTUM * padVelocities[index_] + (1.0 - VELOCITY_MOMENTUM) * std::pow(value_, VELOCITY_EXPONENT);
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - padTimes[index_]);
            if (elapsed < NOTE_ON_DELAY) {
                return PadAction::NO_ACTION;
            }

            padStates[index_] = PadState::ON;
            return PadAction::NOTE_ON;
        }
    }

    return PadAction::NO_ACTION;
}
}
