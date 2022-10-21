QT += serialport core
TARGET = SerialPort
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
#CONFIG -= qt

SOURCES += \
        DataTransmissionBeketov/src/DataTransmissionBeketov.cpp \
        DataTransmissionPC.cpp \
        ISerialPC.cpp \
        main.cpp

HEADERS += \
    DataTransmissionBeketov/src/DataTransmissionBeketov.h \
    DataTransmissionPC.h \
    ISerialPC.h
