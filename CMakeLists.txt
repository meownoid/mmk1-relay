cmake_minimum_required(VERSION 3.5.0)
project(mmk1-relay VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

find_package(cxxopts REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread system)

add_subdirectory(../cabl ${CMAKE_CURRENT_BINARY_DIR}/cabl)

add_executable(
    mmk1-relay 
    main.cpp
    DeviceManager.h
    DeviceManager.cpp
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(${PROJECT_NAME} cxxopts::cxxopts)
target_link_libraries(${PROJECT_NAME} cabl-static)
target_link_libraries(${PROJECT_NAME} Boost::boost)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
