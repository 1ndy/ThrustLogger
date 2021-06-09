#include "serialconfigwindow.h"
#include "../ui/ui_serialconfigwindow.h"

#include <iostream>

SerialConfigWindow::SerialConfigWindow(QWidget *parent)
    : QDialog(parent, Qt::WindowCloseButtonHint | Qt::WindowTitleHint)
    , ui(new Ui::SerialConfigWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Configure Serial Device");
    ui->configureButton->setEnabled(false);

    ui->baudRateComboBox->addItem("9600", QVariant((qulonglong)CBR_9600));
    ui->baudRateComboBox->addItem("19200", QVariant((qulonglong)CBR_19200));
    ui->baudRateComboBox->addItem("38400", QVariant((qulonglong)CBR_38400));
    ui->baudRateComboBox->addItem("57600", QVariant((qulonglong)CBR_57600));
    ui->baudRateComboBox->addItem("115200", QVariant((qulonglong)CBR_115200));

    this->_baud_rate_int = 9600;
    this->_baud_rate_define = CBR_9600;

    QObject::connect(ui->cancelButton, &QPushButton::clicked, this, &SerialConfigWindow::close);
    QObject::connect(ui->configureButton, &QPushButton::clicked, this, &SerialConfigWindow::emitConfigParams);
    QObject::connect(ui->comPortComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &SerialConfigWindow::updateCOMPort);
    QObject::connect(ui->baudRateComboBox, &QComboBox::currentTextChanged, this, &SerialConfigWindow::updateBaudRate);
    QObject::connect(ui->refreshButton, &QPushButton::clicked, this, &SerialConfigWindow::rescanForComPorts);
    this->updateComPortComboBox();
}

SerialConfigWindow::~SerialConfigWindow() {
    delete ui;
}

void SerialConfigWindow::updateCOMPort(int index) {
    if(index < this->_available_com_ports.size()) {
        this->_com_port = this->_available_com_ports[index];
    }
}

void SerialConfigWindow::updateBaudRate(QString text) {
    int r = text.toUInt();
    this->_baud_rate_int = r;
    this->_baud_rate_define = (DWORD)this->ui->baudRateComboBox->currentData().toULongLong();
    std::cout << "set baud rate define to " << this->_baud_rate_define << std::endl;
}

void SerialConfigWindow::emitConfigParams() {
    try {
        SerialDevice* loadcell = new SerialDevice(this->_com_port, this->_baud_rate_define);
        emit configParams(loadcell);
        this->close();
    } catch(SerialDeviceException e) {
        QMessageBox mbx(QMessageBox::Warning, QString("Serial device error"), QString(e.what()));
        mbx.exec();
    }
}

void SerialConfigWindow::getListOfComPorts() {
    std::vector<ULONG> numbers(255);
    ULONG count = 255;
    ULONG found;

    using LPGETCOMMPORTS = ULONG (__stdcall *)(PULONG, ULONG, PULONG);
    HMODULE hDLL = LoadLibrary(TEXT("api-ms-win-core-comm-l1-1-0.dll"));
    auto pGetCommPorts = reinterpret_cast<LPGETCOMMPORTS>(GetProcAddress(hDLL, "GetCommPorts"));
    DWORD result = pGetCommPorts(numbers.data(), count, &found);
    if(!result) {
        QMessageBox(QMessageBox::Critical, QString("Serial device error"), QString("GetCommPorts failed"));
    }

    this->_available_com_ports = std::vector<std::string>(found);

    //store them for future lookups
    for(int i = 0; i < (int)found; i++) {
        this->_available_com_ports.push_back(std::string("COM") + std::to_string(numbers[i]));
    }
}

void SerialConfigWindow::updateComPortComboBox() {
    this->getListOfComPorts();
    this->ui->comPortComboBox->setCurrentIndex(0);
    ui->comPortComboBox->clear();
    for(std::string name : this->_available_com_ports) {
        this->ui->comPortComboBox->addItem(QString::fromStdString(name));
    }
    if(this->_available_com_ports.size() > 0) {
        this->ui->comPortComboBox->setCurrentIndex(1);
    }
    if(this->ui->comPortComboBox->currentIndex() > 0) {
        this->ui->configureButton->setEnabled(true);
    } else {
        this->ui->configureButton->setEnabled(false);
    }
}

void SerialConfigWindow::rescanForComPorts() {
    this->updateComPortComboBox();
}
