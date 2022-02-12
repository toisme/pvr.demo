cmake_minimum_required(VERSION 3.5)
project(pvr.demo.json)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

find_package(Kodi REQUIRED)
find_package(JsonCpp REQUIRED)

include_directories(${KODI_INCLUDE_DIR}/.. # Hack way with "/..", need bigger Kodi cmake rework to match right include ways
                    ${JSONCPP_INCLUDE_DIRS})

set(DEPLIBS ${JSONCPP_LIBRARIES})

set(PVRDEMOJSON_SOURCES src/PVRDemo.cpp)

set(PVRDEMOJSON_HEADERS src/PVRDemo.h)

build_addon(pvr.demo.json PVRDEMOJSON DEPLIBS)

include(CPack)
