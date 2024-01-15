#include <iostream>
#include <string>
#include <cstdlib>
#include <cxxopts.hpp>

#include "DeviceManager.h"

using namespace sl;
using namespace sl::cabl;

int main(int argc, char** argv) {
    cxxopts::Options options("mmk1-relay", "Machine MK1 Relay");
    options.add_options()
        ("m,midi-port", "MIDI port number", cxxopts::value<int>()->default_value("0"))
        ("o,osc-server", "OSC server address", cxxopts::value<std::string>()->default_value("127.0.0.1"))
        ("p,osc-port", "OSC server port", cxxopts::value<int>()->default_value("7099"));

    auto result = options.parse(argc, argv);

    int midiPort = result["midi-port"].as<int>();
    std::string oscServerAddress = result["osc-server"].as<std::string>();
    int oscServerPort = result["osc-port"].as<int>();

    DeviceManager deviceManager({}, midiPort, oscServerAddress, oscServerPort);

    std::cout << "MIDI Port: " << midiPort << std::endl;
    std::cout << "OSC Server Address: " << oscServerAddress << std::endl;
    std::cout << "OSC Server Port: " << oscServerPort << std::endl;

    std::cout << "Type 'q' and hit ENTER to quit." << std::endl;

    while (std::cin.get() != 'q') {
        std::this_thread::yield();
    }

    return 0;
}
