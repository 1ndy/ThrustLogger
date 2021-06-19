cmake_minimum_required(VERSION 3.5)

project(ThrustLogger VERSION 0.7 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/5.15.1/msvc_2019_64")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt5Charts)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/serialconfigwindow.cpp
        src/dynamicchart.cpp

        include/mainwindow.h
        include/serialconfigwindow.h
        include/dynamicchart.h
        
        ui/mainwindow.ui
        ui/serialconfigwindow.ui

)

add_executable(ThrustLogger ${PROJECT_SOURCES})

target_include_directories(ThrustLogger PUBLIC include ui)

target_include_directories(ThrustLogger PUBLIC 
        lib/SerialDevice
        lib/AveragerQueue        
)

add_library(SerialDevice STATIC 
        lib/SerialDevice/serialdevice.cpp 
        lib/SerialDevice/serialdevice.h
)

add_library(AveragerQueue STATIC 
        lib/AveragerQueue/averagerqueue.h
)

target_link_libraries(ThrustLogger PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts SerialDevice AveragerQueue)

