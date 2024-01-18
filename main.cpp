#include <iostream>
#include <string>
#include <cstdlib>
#include <cxxopts.hpp>

#include <cabl/devices/Coordinator.h>

#include "DeviceManager.h"

using namespace sl;
using namespace sl::cabl;

int main(int argc, char** argv) {
    cxxopts::Options options("mmk1-relay", "Maschine MK1 Relay");
    options.add_options()
        ("m,midi-port", "MIDI port number", cxxopts::value<int>()->default_value("0"))
        ("o,osc-server", "OSC server address", cxxopts::value<std::string>()->default_value("127.0.0.1"))
        ("p,osc-port", "OSC server port", cxxopts::value<int>()->default_value("7099"))
        ("l,invert-leds", "Invert LEDs", cxxopts::value<bool>()->default_value("false"))
        ("s,invert-shift", "Invert Shift", cxxopts::value<bool>()->default_value("false"))
        ("t,text", "Text to display on second screen", cxxopts::value<std::string>()->default_value("MMK1 RELAY"))
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    int midiPort = result["midi-port"].as<int>();
    std::string oscServerAddress = result["osc-server"].as<std::string>();
    int oscServerPort = result["osc-port"].as<int>();
    bool invertLEDs = result["invert-leds"].as<bool>();
    bool invertShift = result["invert-shift"].as<bool>();
    std::string text = result["text"].as<std::string>();

    DeviceManager deviceManager({}, midiPort, oscServerAddress, oscServerPort, invertLEDs, invertShift, text);
    Coordinator coordinator(&deviceManager);
    coordinator.scan();
    coordinator.run();

    std::cout << "Type 'q' and hit ENTER to quit." << std::endl;

    while (std::cin.get() != 'q') {
        std::this_thread::yield();
    }

    return 0;
}
