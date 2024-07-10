#include "e_window.h"
#include "ui_e_window.h"
#include "mainwindow.h"


E_window::E_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::E_window)
{
    ui->setupUi(this);
}

E_window::~E_window()
{
    delete ui;
}

void E_window::on_pushButton_clicked()
{
    this->close();
}


void E_window::on_get_button_clicked()
{
    ui->lineEdit_2->setText("已使能！！");
}


void E_window::on_lose_button_clicked()
{
    ui->lineEdit_2->setText("电机未使能");
}

