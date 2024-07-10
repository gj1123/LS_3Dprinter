#include "z_window.h"
#include "ui_z_window.h"
#include "mainwindow.h"

extern int fd_motor;
int motor_z_up_flag = 0;
int motor_z_down_flag = 0;

z_window::z_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::z_window)
{
    ui->setupUi(this);
}

z_window::~z_window()
{
    delete ui;
}

void z_window::on_z_close_button_clicked()
{
    this->close();
}


void z_window::on_horizontalSlider_valueChanged(int value)
{
    ui->lineEdit->setText(QString("%1").arg(value));
}



void z_window::on_lineEdit_textChanged(const QString &arg1)
{
    ui->horizontalSlider->setValue(arg1.toUInt());
}


void z_window::on_get_button_clicked()
{
    motor_cmd_block_t motor = {0};

    motor.enable_bits = 4;
    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    ui->lineEdit_2->setText("Z轴已使能");
}


void z_window::on_lose_button_clicked()
{
    motor_cmd_block_t motor = {0};

    motor.enable_bits = 0;
    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    ui->lineEdit_2->setText("Z轴未使能");
}


void z_window::on_step_set_button_clicked()
{
    motor_cmd_block_t motor = {0};

    QString text=ui->lineEdit->text();
    int speed_z = text.toInt();
    int step_num_z=ui->step_spinBox->value();  //获取步数数值
    printf("speed_z is %d, step_num_z is %d.", speed_z, step_num_z);
    motor.enable_bits = 4;
    motor.operation_mode_z = 2;
    motor.steps[2] = step_num_z;
    motor.step_event_count = step_num_z;
    motor.step_speed = speed_z;

    if(motor_z_up_flag == 1)
        motor.direction_bits = 0;
    else if(motor_z_down_flag == 1)
        motor.direction_bits = 4;

    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    printf("open motor_z success.\n");
}


void z_window::on_pushButton_pressed()
{
    motor_cmd_block_t motor = {0};

    QString text=ui->lineEdit->text();
    int speed_z = text.toInt();

    motor.enable_bits = 4;
    motor.operation_mode_z = 1;
    motor.step_speed = speed_z;

    if(motor_z_up_flag == 1)
        motor.direction_bits = 0; // 0为上移 1为下移
    else if(motor_z_down_flag == 1)
        motor.direction_bits = 4;

    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    printf("open motor_z success.\n");
}


void z_window::on_pushButton_released()
{
    motor_cmd_block_t motor = {0};

    motor.operation_mode_z = 0;
    write(fd_motor, &motor, sizeof(motor_cmd_block_t));
    printf("close motor_z success.\n");
}


void z_window::on_up_button_clicked()
{
    motor_z_up_flag = 1;
    motor_z_down_flag = 0;
}


void z_window::on_down_button_clicked()
{
    motor_z_up_flag = 0;
    motor_z_down_flag = 1;
}
