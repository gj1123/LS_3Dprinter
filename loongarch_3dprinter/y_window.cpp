#include "y_window.h"
#include "ui_y_window.h"
#include "mainwindow.h"

extern int fd_motor;
int motor_y_front_flag = 0;
int motor_y_back_flag = 0;

y_window::y_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::y_window)
{
    ui->setupUi(this);
}

y_window::~y_window()
{
    delete ui;
}

void y_window::on_y_close_button_clicked()
{
    this->close();
}


void y_window::on_horizontalSlider_valueChanged(int value)
{
    ui->lineEdit->setText(QString("%1").arg(value));
}



void y_window::on_lineEdit_textChanged(const QString &arg1)
{
    ui->horizontalSlider->setValue(arg1.toUInt());
}


void y_window::on_get_button_clicked()
{
    motor_cmd_block_t motor = {0};

    motor.enable_bits = 2;
    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    ui->lineEdit_2->setText("Y轴已使能");
}


void y_window::on_lose_button_clicked()
{
    motor_cmd_block_t motor = {0};

    motor.enable_bits = 0;
    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    ui->lineEdit_2->setText("Y轴未使能");
}


void y_window::on_step_set_button_clicked()
{
    motor_cmd_block_t motor = {0};

    QString text=ui->lineEdit->text();
    int speed_y = text.toInt();
    int step_num_y=ui->step_spinBox->value();  //获取步数数值
    printf("speed_y is %d, step_num_y is %d.", speed_y, step_num_y);
    motor.enable_bits = 2;
    motor.operation_mode_y = 2;
    motor.steps[1] = step_num_y;
    motor.step_event_count = step_num_y;
    motor.step_speed = speed_y;

    if(motor_y_front_flag == 1)
        motor.direction_bits = 0;
    else if(motor_y_back_flag == 1)
        motor.direction_bits = 2;

    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    printf("open motor_y success.\n");
}


void y_window::on_pushButton_pressed()
{
    motor_cmd_block_t motor = {0};

    QString text=ui->lineEdit->text();
    int speed_y = text.toInt();

    motor.enable_bits = 2;
    motor.operation_mode_y = 1;
    motor.step_speed = speed_y;

    if(motor_y_front_flag == 1) // 1为前移 0为后移
        motor.direction_bits = 0;
    else if(motor_y_back_flag == 1)
        motor.direction_bits = 2;

    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    printf("open motor_y success.\n");
}


void y_window::on_pushButton_released()
{
    motor_cmd_block_t motor = {0};

    motor.operation_mode_y = 0;
    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    printf("close motor_y success.\n");
}


void y_window::on_front_button_clicked()
{
    motor_y_front_flag = 1;
    motor_y_back_flag = 0;
}


void y_window::on_back_button_clicked()
{
    motor_y_front_flag = 0;
    motor_y_back_flag = 1;
}

