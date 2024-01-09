#include <iostream>

#include "DeviceManager.h"

using namespace sl;
using namespace sl::cabl;

int main(int, char**){
    DeviceManager DeviceManager;

    std::cout << "Type 'q' and hit ENTER to quit." << std::endl;

    while (std::cin.get() != 'q')
    {
        std::this_thread::yield();
    }

    return 0;
}
