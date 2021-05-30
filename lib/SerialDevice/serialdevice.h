#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <windows.h>
#include <string>
#include <exception>
#include <thread>
#include <mutex>
#include <chrono>
#include <queue>
#include <utility>

#define ARDUINO_FLOAT_SIZE_BYTES 4

struct SerialDeviceException : public std::exception {
    virtual const char* what() const throw() {
        return "Error with Serial Device";
    }
};

struct SerialDeviceCouldNotConnect : SerialDeviceException {
    const char* what() const throw() {
        return "Could not create serial device";
    }
};

struct SerialDeviceCouldNotConfigure : SerialDeviceException {
    const char* what() const throw() {
        return "Could not configure serial port";
    }
};

struct SerialDeviceCouldNotSetTimeouts : SerialDeviceException {
    const char* what() const throw() {
        return "Could not set device timeouts";
    }
};

struct SerialDeviceReadError : SerialDeviceException {
    const char* what() const throw() {
        return "Failed to read from device";
    }
};

class SerialDevice
{
public:
    SerialDevice(std::string com_port, DWORD baud_rate);
    ~SerialDevice();

    void start();
    DCB getParams();
    void waitForJoin();
    void stopPolling();

    float getDuration();
    std::chrono::time_point<std::chrono::high_resolution_clock> getStartTime();
    std::queue<std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, float>> getQueue();

    std::string getComPort() {return _com_port;};
    int getBaudRate();


private:

    void poll(int buffsize);

    std::string _com_port;
    int _baud_rate;
    bool _run_poll;
    std::chrono::high_resolution_clock _clock;
    std::chrono::time_point<std::chrono::high_resolution_clock> _poll_start_time, _poll_stop_time;
    float _record_duration;

    std::mutex _queueMutex;
    std::queue<std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, float>> _dataQueue;

    HANDLE _hSerial;
    DCB _serialParams;
    COMMTIMEOUTS _timeouts;
    std::thread _poll_thread;
};

#endif // SERIALDEVICE_H
