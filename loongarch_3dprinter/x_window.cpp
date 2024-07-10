#include "x_window.h"
#include "ui_x_window.h"
#include "mainwindow.h"

extern int fd_motor;
int motor_x_right_flag = 0;
int motor_x_left_flag = 0;

x_window::x_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::x_window)
{
    ui->setupUi(this);
}

x_window::~x_window()
{
    delete ui;
}

void x_window::on_x_close_button_clicked()
{
    this->close();
}


void x_window::on_horizontalSlider_valueChanged(int value)
{
    ui->lineEdit->setText(QString("%1").arg(value));
}


void x_window::on_lineEdit_textChanged(const QString &arg1)
{
    ui->horizontalSlider->setValue(arg1.toUInt());
}


void x_window::on_get_button_clicked()
{
    motor_cmd_block_t motor = {0};

    motor.enable_bits = 1;
    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    ui->lineEdit_2->setText("X轴已使能");
}


void x_window::on_lose_button_clicked()
{
    motor_cmd_block_t motor = {0};

    motor.enable_bits = 0;
    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    ui->lineEdit_2->setText("X轴未使能");
}


void x_window::on_step_set_button_clicked()
{
    motor_cmd_block_t motor = {0};

    QString text=ui->lineEdit->text();
    int speed_x = text.toInt();
    int step_num_x=ui->step_spinBox->value();  //获取步数数值
    printf("speed_x is %d, step_num_x is %d.", speed_x, step_num_x);
    motor.enable_bits = 1;
    motor.operation_mode_x = 2;
    motor.steps[0] = step_num_x;
    motor.step_event_count = step_num_x;
    motor.step_speed = speed_x;

    if(motor_x_right_flag == 1)
        motor.direction_bits = 1;
    else if(motor_x_left_flag == 1)
        motor.direction_bits = 0;

    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    printf("open motor_x success.\n");
}


void x_window::on_pushButton_pressed()
{
    motor_cmd_block_t motor = {0};

    QString text=ui->lineEdit->text();
    int speed_x = text.toInt();

    motor.enable_bits = 1;
    motor.operation_mode_x = 1;
    motor.step_speed = speed_x;

    if(motor_x_right_flag == 1)
        motor.direction_bits = 1; // 1为右移 0为左移
    else if(motor_x_left_flag == 1)
        motor.direction_bits = 0;

    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    printf("open motor_x success.\n");
}

void x_window::on_pushButton_released()
{
    motor_cmd_block_t motor = {0};

    motor.operation_mode_x = 0;
    motor.enable_bits = 1;
    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    printf("close motor_x success.\n");
}


void x_window::on_left_button_clicked()
{
    motor_x_right_flag = 0;
    motor_x_left_flag = 1;
}


void x_window::on_right_button_clicked()
{
    motor_x_right_flag = 1;
    motor_x_left_flag = 0;
}
