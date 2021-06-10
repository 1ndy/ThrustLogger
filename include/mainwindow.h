#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "./dynamicchart.h"
#include "./serialdevice.h"
#include "./serialconfigwindow.h"

#include <thread>
#include <mutex>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void showSerialConfigWindow();

signals:
    void newDataRecorded();

private slots:
    void setSerialDevice(SerialDevice* sd);
    void toggleRecord();
    void updateRecordTime();
    void addDataToPlot(std::queue<std::pair<float,float>> points);
    void selectLogFileLocation();
    void updateFilename();

    void setEnableFileSelection(bool enabled);
    void setEnableRecording(bool enabled);

    void updateTimeAndNumBytes();

private:
    Ui::MainWindow *ui;
    SerialDevice* sd;
    SerialConfigWindow* serialconfigwindow;
    std::chrono::high_resolution_clock _clock;
    std::chrono::time_point<std::chrono::high_resolution_clock> _poll_start_time, _poll_stop_time;
    bool _can_record;
    bool _recording;
    bool _run_dispatch;
    std::thread _dispath_thread;
    size_t _volatile_data_size;

    std::ofstream outputFile;
    std::mutex logMutex;

    float xIndex;

    std::string filename;

    int bytesOfData;

    DynamicChart dc;

    QLabel* statusBarComPortLabel;
    QLabel* statusBarBaudRateLabel;

    void dispatchDataQueue();
    std::queue<std::pair<float, float>> processDataQueue();
    void recordDataChunk(std::queue<std::pair<float, float>>);
    void plotDataFromFile();
    void setupStatusBar();

    std::string formatNumBytes();
};
#endif // MAINWINDOW_H
