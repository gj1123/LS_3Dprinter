#include "mainwindow.h"
#include "ui_mainwindow.h"

// 温控驱动设备文件描述符
int fd_heater_fan;
// 步进电机驱动设备文件描述符
int fd_motor;

// 电机初始化
void motor_init(void)
{
    fd_motor = open("/dev/ls2k1000la_3dprinter_motor", O_WRONLY);
}

// 温控初始化
void temp_init(void)
{
    fd_heater_fan = open("/dev/3dPrinter_heater_fan", O_WRONLY);
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

//调试按键界面的切换
void MainWindow::on_X_button_clicked()
{
    x_win= new x_window(this);
    x_win->show();
    motor_init();
}


void MainWindow::on_Y_button_clicked()
{
    y_win= new y_window(this);
    y_win->show();
    motor_init();
}


void MainWindow::on_Z_button_clicked()
{
    z_win= new z_window(this);
    z_win->show();
    motor_init();
}



void MainWindow::on_Extruded_button_clicked()
{
    e_win= new E_window(this);
    e_win->show();
}


void MainWindow::on_Fan_button_clicked()
{
    fan_win= new Fan_window(this);
    fan_win->show();
    temp_init();
}


void MainWindow::on_Fire_extruded_button_clicked()
{
    fire_e_win= new Fire_e_window(this);
    fire_e_win->show();
    temp_init();
}


void MainWindow::on_Fire_chuang_button_clicked()
{
    fire_c_win= new Fire_c_window(this);
    fire_c_win->show();
    temp_init();
}



