#include "fan_window.h"
#include "ui_fan_window.h"
#include "mainwindow.h"

extern int fd_heater_fan;

Fan_window::Fan_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Fan_window)
{
    ui->setupUi(this);
}

Fan_window::~Fan_window()
{
    delete ui;
}

void Fan_window::on_fan_close_button_clicked()
{
    this->close();
}


void Fan_window::on_horizontalSlider_valueChanged(int value)
{
    ui->lineEdit->setText(QString("%1").arg(value));
}


void Fan_window::on_lineEdit_textChanged(const QString &arg1)
{
    ui->horizontalSlider->setValue(arg1.toUInt());
}


void Fan_window::on_start_button_clicked()
{
    QString text=ui->lineEdit->text();
    int speed_num = text.toInt();
    heater_fan_work_time_t work_time = {0};

    work_time.extruder_heater_ms     = 0;
    work_time.extruder_heater_fan_ms = speed_num;
    work_time.heater_bed_ms          = 0;
    write(fd_heater_fan, &work_time, sizeof(heater_fan_work_time_t));
}


void Fan_window::on_stop_button_clicked()
{
    heater_fan_work_time_t work_time = {0};

    work_time.extruder_heater_ms     = 0;
    work_time.extruder_heater_fan_ms = 0;
    work_time.heater_bed_ms          = 0;
    write(fd_heater_fan, &work_time, sizeof(heater_fan_work_time_t));
}

