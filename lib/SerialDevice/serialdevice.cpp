#include <iostream>
#include <csignal>

#include "serialdevice.h"

SerialDevice::SerialDevice(std::string com_port, DWORD baud_rate)
{
    this->_run_poll = false;

    DWORD result;
    this->_com_port = com_port;
    this->_hSerial = CreateFile(com_port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    result = GetCommState(_hSerial, &_serialParams);
    if(!result) {
        throw SerialDeviceCouldNotConnect();
    }

    _serialParams.DCBlength = sizeof(_serialParams);
    _serialParams.BaudRate = baud_rate;
    _serialParams.ByteSize = 8;
    _serialParams.StopBits = ONESTOPBIT;
    _serialParams.Parity = NOPARITY;
    result = SetCommState(_hSerial, &_serialParams);
    if(!result) {
        std::cout << "throwing error " << GetLastError() << std::endl;
        throw SerialDeviceCouldNotConfigure();
    }

    _timeouts = {0};
    _timeouts.ReadIntervalTimeout = 25;
    _timeouts.ReadTotalTimeoutConstant = 50;
    _timeouts.ReadTotalTimeoutMultiplier = 10;
    _timeouts.WriteTotalTimeoutConstant = 50;
    _timeouts.WriteTotalTimeoutMultiplier = 10;
    result = SetCommTimeouts(_hSerial, &_timeouts);
    if(!result) {
        throw SerialDeviceCouldNotSetTimeouts();
    }
}

SerialDevice::~SerialDevice() {
    this->stopPolling();
    CloseHandle(_hSerial);
}

void SerialDevice::start() {
    this->_run_poll = true;
    this->_poll_thread = std::thread(&SerialDevice::poll, this, 32);
}

int SerialDevice::getBaudRate() {
    switch(this->_serialParams.BaudRate) {
        case CBR_9600:
            return 9600; break;
        case CBR_19200:
            return 19200; break;
        case CBR_38400:
            return 38400; break;
        case CBR_57600:
            return 57600; break;
        case CBR_115200:
            return 115200; break;
        default:
            std::cout << this->_baud_rate << std::endl;
            return 0;
    }
}

DCB SerialDevice::getParams() {
    return this->_serialParams;
}

//device prints 4 byte floats, read them and convert
void SerialDevice::poll(int buffsize) {
    char* buff = new char[buffsize + 1];
    float* nums = new float[buffsize / ARDUINO_FLOAT_SIZE_BYTES];
    DWORD bytesRead = 0;
    DWORD result;
    this->_poll_start_time = this->_clock.now();
    while(this->_run_poll) {
        result = ReadFile(_hSerial, buff, buffsize, &bytesRead, NULL);
        if(!result) {
            throw SerialDeviceReadError();
        } else {
            memcpy(nums, buff, buffsize);
            this->_queueMutex.lock();
                for(DWORD i = 0; i < bytesRead / ARDUINO_FLOAT_SIZE_BYTES; i++) {
                    this->_dataQueue.push(std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, float>(this->_clock.now(), nums[i]));
                }
            this->_queueMutex.unlock();
            //std::cout << std::endl;
        }
    }
    delete[] buff;
    delete[] nums;
}

void SerialDevice::waitForJoin() {
    this->_poll_thread.join();
}

void SerialDevice::stopPolling() {
    if(this->_run_poll) {
        this->_run_poll = false;
        this->_poll_thread.join();
        this->_poll_stop_time = this->_clock.now();
        this->_record_duration = std::chrono::duration_cast<std::chrono::milliseconds>(this->_poll_stop_time - this->_poll_start_time).count() / 1000.0f;
    }
}

float SerialDevice::getDuration() {
    return this->_record_duration;
}

std::chrono::time_point<std::chrono::high_resolution_clock> SerialDevice::getStartTime() {
    return this->_poll_start_time;
}

//creat and empty queue and swap it for the one that is full
std::queue<std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, float>> SerialDevice::getQueue() {
    std::queue<std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, float>> newData;
    this->_queueMutex.lock();
    this->_dataQueue.swap(newData);
    this->_queueMutex.unlock();
    return newData;
}

/*
int main() {
    try {
        SerialDevice loadcell("COM4", 115200);
        std::cout << "Created serial device on COM4" << std::endl;
        loadcell.waitForJoin();
    } catch(SerialDeviceException e) {
        std::cerr << e.what() << std::endl;
        std::exit(1);
    } catch(...) {
        std::cout << "encountered unexpected exception" << std::endl;
    }

    return 0;
}
*/
