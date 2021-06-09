#include <QTimer>

#include "mainwindow.h"
#include "../ui/ui_mainwindow.h"
#include "serialdevice.h"
#include "averagerqueue.h"
#include "serialconfigwindow.h"

#include <windows.h>
#include <Lmcons.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(QString("Thrust Logger"));
    this->setupStatusBar();

    this->setFixedSize(this->width(), this->height());
    this->statusBar()->setSizeGripEnabled(false);

    this->serialconfigwindow = new SerialConfigWindow();
    this->ui->recordButton->setEnabled(false);
    this->_can_record = false;
    this->_recording = false;
    this->_volatile_data_size = 0;

    this->sd = NULL;

    this->filename = "";

    bytesOfData = 0;

    xIndex = 0;

    ui->plot->setChart(dc.getChart());
    ui->plot->setRenderHint(QPainter::Antialiasing);

    //buttons n things
    QObject::connect(ui->configureCOMButton, &QPushButton::clicked, this, &MainWindow::showSerialConfigWindow);
    QObject::connect(this->serialconfigwindow, &SerialConfigWindow::configParams, this, &MainWindow::setSerialDevice);
    QObject::connect(ui->recordButton, &QPushButton::clicked, this, &MainWindow::toggleRecord);
    QObject::connect(ui->logFileSelectionButton, &QPushButton::clicked, this, &MainWindow::selectLogFileLocation);
    QObject::connect(ui->filenameLineEdit, &QLineEdit::editingFinished, this, &MainWindow::updateFilename);

    //menu bar
    QObject::connect(ui->actionConfigure_Serial, &QAction::triggered, this, &MainWindow::showSerialConfigWindow);

    QTimer *timer = new QTimer(this);
    QObject::connect(timer, &QTimer::timeout, this, &MainWindow::updateRecordTime);
    timer->start(1);

}

MainWindow::~MainWindow()
{
    if(sd) {
        sd->~SerialDevice();
    }
    this->_run_dispatch = false;
    if(this->_dispath_thread.joinable()) {
        this->_dispath_thread.join();
    }
    delete ui;
}

void MainWindow::setupStatusBar() {
    ui->statusbar->showMessage("Ready");
    statusBarComPortLabel = new QLabel();
    statusBarComPortLabel->setText("COM port: None");
    statusBarComPortLabel->setMargin(5);

    statusBarBaudRateLabel = new QLabel();
    statusBarBaudRateLabel->setText("Baud Rate: Not set");
    statusBarBaudRateLabel->setMargin(5);

    ui->statusbar->addPermanentWidget(statusBarComPortLabel, 0);
    ui->statusbar->addPermanentWidget(statusBarBaudRateLabel, 0);
}

void MainWindow::showSerialConfigWindow() {
    ui->statusbar->showMessage("Configuring Serial Device");
    if(this->filename == "") {
        QMessageBox qbx(this);
        qbx.setWindowTitle("Warning");
        qbx.setText("No log file selected. Data can not be recorded. Continue?");
        qbx.setIcon(QMessageBox::Warning);
        qbx.addButton(QMessageBox::Yes);
        qbx.addButton(QMessageBox::No);
        qbx.exec();
        if(qbx.result() == QMessageBox::No) {
            ui->statusbar->showMessage("Ready");
            return;
        }
    }

    // clean up an old connection
    this->_run_dispatch = false;
    if(this->_dispath_thread.joinable()) {
        this->_dispath_thread.join();
        this->sd->stopPolling();
        this->sd->~SerialDevice();
        this->sd = NULL;
        statusBarComPortLabel->setText("COM port: None");
        statusBarBaudRateLabel->setText("Baud Rate: Not set");
    }
    // collect params for a new connection
    this->serialconfigwindow->setModal(true);
    this->serialconfigwindow->exec();
    // sd will get created from a slot
    if(this->sd) {
        // start dispatching data
        this->_run_dispatch = true;
        this->_dispath_thread = std::thread(&MainWindow::dispatchDataQueue, this);
    }
    ui->statusbar->showMessage("Ready");
}

void MainWindow::setSerialDevice(SerialDevice* sd) {
    if(sd) {
        this->sd = sd;
        sd->start();
        this->statusBarComPortLabel->setText(QString::fromStdString("COM port: " + sd->getComPort()));
        statusBarBaudRateLabel->setText(QString::fromStdString("Baud Rate: " + std::to_string(sd->getBaudRate())));
        ui->statusbar->showMessage("Started serial device", 1000);
        if(this->filename != "") {
            setEnableRecording(true);
        }
        //this->setEnableFileSelection(false);
    }
}

void MainWindow::updateFilename() {
    this->filename = ui->filenameLineEdit->text().toStdString();
    this->bytesOfData = 0;
}

void MainWindow::toggleRecord() {
    if(this->ui->recordButton->isChecked()) {
        ui->configureCOMButton->setEnabled(false);
        this->setEnableFileSelection(false);
        this->ui->recordButton->setText("Recording");
        ui->statusbar->showMessage("Recording");
        std::cout << "recording" << std::endl;
        this->_recording = true;
        this->_poll_start_time = this->_clock.now();
    } else {
        this->_recording = false;
        ui->configureCOMButton->setEnabled(true);
        this->ui->recordButton->setText("Record");
        ui->statusbar->clearMessage();
    }
}

void MainWindow::updateRecordTime() {
    if(this->_recording) {
        float duration_so_far = std::chrono::duration_cast<std::chrono::milliseconds>(this->_clock.now() - this->_poll_start_time).count() / 1000.0f;
        QString time;
        time.setNum(duration_so_far, 'f', 3);
        this->ui->recordTimeLabel->setText(time);
    }
}

void MainWindow::selectLogFileLocation() {
    QFileDialog qfd(this);

    time_t rawtime;
    struct tm * timeinfo;
    char buffer[64];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 64, "%F_%H-%M.log", timeinfo);

    TCHAR username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    GetUserName((TCHAR*)username, &size);

    auto proposedFilename = std::string(buffer);
    std::string proposedPath = "C:\\Users\\" + std::string(username) + "\\Desktop";
    qfd.setDirectory(QString::fromStdString(proposedPath));
    qfd.selectFile(QString::fromStdString(proposedFilename));
    if(qfd.exec()) {
        this->filename = qfd.selectedFiles().at(0).toStdString();
        this->outputFile.open(this->filename);
        if(this->outputFile.is_open()) {
            ui->filenameLineEdit->setText(QString::fromStdString(this->filename));
            this->_can_record = true;
            this->setEnableRecording(true);
        } else {
            QMessageBox qbx(QMessageBox::Icon::Warning, QString::fromStdString("File Error"), QString::fromStdString("Filecould not be opened"));
            qbx.exec();
            this->filename = "";
        }
    }
}

void MainWindow::setEnableFileSelection(bool enabled) {
    ui->filenameLineEdit->setEnabled(enabled);
    ui->logFileSelectionButton->setEnabled(enabled);
}

void MainWindow::setEnableRecording(bool enabled) {
    ui->recordButton->setEnabled(enabled);
    ui->recordTimeLabel->setEnabled(enabled);
}

std::queue<std::pair<float, float>> MainWindow::processDataQueue() {
    auto data = this->sd->getQueue();
    std::queue<std::pair<float, float>> processedData;
    //localize time
    size_t size = data.size();
    for(size_t i = 0; i < size; i++) {
        float delta = std::chrono::duration_cast<std::chrono::microseconds>(data.front().first - this->_poll_start_time).count();
        auto new_element = std::pair<float, float>(xIndex, data.front().second*-1);
        processedData.push(new_element);
        xIndex += 0.00125;
        data.pop();
    }
    return processedData;
}

std::string MainWindow::formatNumBytes() {
    std::string suffixes[4] = {"B","KiB","MiB","GiB"};
    float count = this->bytesOfData;
    int i = 0;
    char buf[16];
    while(count >= 1024 && i < 4) {
        count /= 1024;
        i++;
    }
    if(i == 0) {
        sprintf(buf, "%.0f %s", count, suffixes[i].c_str());
    } else if(i == 1){
        sprintf(buf, "%.2f %s", count, suffixes[i].c_str());
    } else {
       sprintf(buf, "%.3f %s", count, suffixes[i].c_str());
    }
    return std::string(buf);

}

void MainWindow::recordDataChunk(std::queue<std::pair<float, float>> processedData) {
    std::ostringstream chunk;
    while(!processedData.empty()) {
        chunk << processedData.front().second << std::endl;
        processedData.pop();
        this->_volatile_data_size++;
    }
    outputFile << chunk.str();
    this->bytesOfData += chunk.str().size();
    ui->numBytesLabel->setText(QString::fromStdString(this->formatNumBytes()));
    if(this->_volatile_data_size >= 400) {
        outputFile.flush();
        std::cout << "saved " << this->_volatile_data_size << " points" << std::endl;
        this->_volatile_data_size = 0;
    }
}

void MainWindow::addDataToPlot(std::queue<std::pair<float,float>> points) {
    dc.appendNewData(points);
}

void MainWindow::dispatchDataQueue() {
    logMutex.lock();
    AveragerQueue<size_t> aq(10, 0);
    while(this->_run_dispatch) {
        if(this->sd) {
            // prevent system from sleeping while polling
            SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto processedData = this->processDataQueue();
            if(processedData.size() > 0) {
                aq.add(processedData.size());
                int projectedDataRate = aq.average() * 10;
                QString dataRate = QString::number(projectedDataRate);
                QString sampleValue;
                sampleValue.setNum(processedData.front().second, 'f', 3);
                ui->dataRateLabel->setText(dataRate + QString::fromStdString(" Hz"));
                ui->sampleValueLabel->setText(sampleValue);
                //plot data, save if recording
                if(this->_can_record && this->_recording) {
                    this->recordDataChunk(processedData);
                }
                //this->addDataToPlot(processedData);
            }
        }
    }
    outputFile.close();
    logMutex.unlock();
    SetThreadExecutionState(ES_CONTINUOUS);
    ui->statusbar->showMessage("Saved log file");
}

void MainWindow::plotDataFromFile() {
    //plot data
    std::ifstream datafile;
    logMutex.lock();
    datafile.open(this->filename);
    float x = 0.0f;
    float y;
    std::queue<std::pair<float,float>> data;
    if(!datafile.is_open()) {
        std::cout << "could not read datafile" << std::endl;
    } else {
        std::cout << "reading from " << this->filename << std::endl;
    }
    while(datafile >> y) {
        //datafile >> y;
        std::cout << x << " " << y << std::endl;
        data.push(std::pair<float,float>(x, -y));
        x += 0.00125;
    }
    datafile.close();
    logMutex.unlock();
    dc.appendNewData(data);

}
