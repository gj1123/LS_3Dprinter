#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "x_window.h"
#include "y_window.h"
#include "z_window.h"
#include "e_window.h"
#include "fan_window.h"
#include "fire_e_window.h"
#include "fire_c_window.h"

// user include
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

// 结构体格式要和驱动一样
typedef struct
{
    // 直线轨迹设定的参数
    unsigned char direction_bits;               // 各轴步进电机的方向  从最低位开始为XYZE
    unsigned char enable_bits;                  // 各轴步进电机的使能
    long steps[4];                              // 各轴步进电机的步数
    unsigned long step_event_count;             // 此block的最大的步数
    unsigned char check_endstop;                // 是否检查限位开关状态
    unsigned char step_speed;                   // 工作速度 最大值为1000 一秒钟1000次脉冲 和定时器中断同速

    unsigned char operation_mode_x;             // 工作模式 0为停止 1为速度移动 2为位置移动
    unsigned char operation_mode_y;
    unsigned char operation_mode_z;
    unsigned char motor_pulse_start_flag;       // 每一个block进来第一件事就是改PWM
} motor_cmd_block_t;


typedef struct {
    unsigned int extruder_heater_ms;        // 鎸ゅ嚭鏈哄姞鐑殑鏃堕暱(ms)
    unsigned int extruder_heater_fan_ms;    // 鎸ゅ嚭鏈洪鎵囧伐浣滅殑鏃堕暱(ms)
    unsigned int heater_bed_ms;             // 鐑簥鍔犵儹鐨勬椂闀?ms)
} heater_fan_work_time_t;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_X_button_clicked();

    void on_Y_button_clicked();

    void on_Z_button_clicked();

    void on_Extruded_button_clicked();

    void on_Fan_button_clicked();

    void on_Fire_extruded_button_clicked();

    void on_Fire_chuang_button_clicked();

private:
    Ui::MainWindow *ui;
    x_window *x_win;
    y_window *y_win;
    z_window *z_win;
    E_window *e_win;
    Fan_window *fan_win;
    Fire_e_window *fire_e_win;
    Fire_c_window *fire_c_win;
};
#endif // MAINWINDOW_H
