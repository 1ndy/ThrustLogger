#ifndef SERIALCONFIGWINDOW_H
#define SERIALCONFIGWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <string>

#include <windows.h>
#include <winbase.h>

#include "./serialdevice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SerialConfigWindow; }
QT_END_NAMESPACE

class SerialConfigWindow : public QDialog
{
    Q_OBJECT

public:
    SerialConfigWindow(QWidget *parent = nullptr);
    ~SerialConfigWindow();

    void emitConfigParams();

signals:
        void configParams(SerialDevice* sd);

private slots:
    void updateCOMPort(int index);
    void updateBaudRate(QString text);
    void rescanForComPorts();

private:
    Ui::SerialConfigWindow *ui;
    std::vector<std::string> _available_com_ports;
    std::string _com_port;
    int _baud_rate_int;
    DWORD _baud_rate_define;

    void getListOfComPorts();
    void updateComPortComboBox();

};

#endif // SERIALCONFIGWINDOW_H
