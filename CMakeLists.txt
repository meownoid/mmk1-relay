cmake_minimum_required(VERSION 3.5.0)
project(mmk1-relay VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(
    mmk1-relay 
    main.cpp
    DeviceManager.h
    DeviceManager.cpp
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_subdirectory(../cabl ${CMAKE_CURRENT_BINARY_DIR}/cabl)
target_link_libraries(${PROJECT_NAME} PRIVATE cabl-static)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
