/********************************************************************************
** Form generated from reading UI file 'x_window.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_X_WINDOW_H
#define UI_X_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_x_window
{
public:
    QWidget *centralwidget;
    QSlider *horizontalSlider;
    QLabel *label_3;
    QLabel *label_2;
    QPushButton *step_set_button;
    QPushButton *reset_button;
    QPushButton *lose_button;
    QLabel *label_6;
    QSpinBox *step_spinBox;
    QRadioButton *left_button;
    QLabel *label_5;
    QLineEdit *lineEdit_2;
    QPushButton *get_button;
    QLabel *label;
    QLabel *label_4;
    QLineEdit *lineEdit;
    QPushButton *pushButton;
    QPushButton *x_close_button;
    QRadioButton *right_button;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *x_window)
    {
        if (x_window->objectName().isEmpty())
            x_window->setObjectName(QString::fromUtf8("x_window"));
        x_window->resize(420, 460);
        centralwidget = new QWidget(x_window);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalSlider = new QSlider(centralwidget);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(140, 160, 141, 22));
        horizontalSlider->setMaximum(1000);
        horizontalSlider->setPageStep(100);
        horizontalSlider->setOrientation(Qt::Horizontal);
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(30, 200, 121, 41));
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        label_3->setFont(font);
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(30, 110, 81, 21));
        label_2->setFont(font);
        step_set_button = new QPushButton(centralwidget);
        step_set_button->setObjectName(QString::fromUtf8("step_set_button"));
        step_set_button->setGeometry(QRect(220, 210, 41, 28));
        step_set_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);"));
        reset_button = new QPushButton(centralwidget);
        reset_button->setObjectName(QString::fromUtf8("reset_button"));
        reset_button->setGeometry(QRect(160, 270, 81, 41));
        reset_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);"));
        lose_button = new QPushButton(centralwidget);
        lose_button->setObjectName(QString::fromUtf8("lose_button"));
        lose_button->setGeometry(QRect(160, 60, 61, 28));
        lose_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);"));
        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 360, 91, 31));
        step_spinBox = new QSpinBox(centralwidget);
        step_spinBox->setObjectName(QString::fromUtf8("step_spinBox"));
        step_spinBox->setGeometry(QRect(150, 210, 51, 31));
        step_spinBox->setMaximum(10000);
        step_spinBox->setSingleStep(100);
        left_button = new QRadioButton(centralwidget);
        left_button->setObjectName(QString::fromUtf8("left_button"));
        left_button->setGeometry(QRect(130, 110, 118, 23));
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 320, 91, 51));
        label_5->setStyleSheet(QString::fromUtf8("border-image: url(:/one.png);"));
        lineEdit_2 = new QLineEdit(centralwidget);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));
        lineEdit_2->setGeometry(QRect(250, 60, 91, 25));
        get_button = new QPushButton(centralwidget);
        get_button->setObjectName(QString::fromUtf8("get_button"));
        get_button->setGeometry(QRect(70, 60, 61, 28));
        get_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);"));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 160, 81, 21));
        label->setFont(font);
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(120, 10, 151, 51));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\351\273\221\344\275\223"));
        label_4->setFont(font1);
        lineEdit = new QLineEdit(centralwidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(290, 160, 61, 25));
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(40, 270, 81, 41));
        pushButton->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);\n"
""));
        x_close_button = new QPushButton(centralwidget);
        x_close_button->setObjectName(QString::fromUtf8("x_close_button"));
        x_close_button->setGeometry(QRect(300, 330, 81, 41));
        x_close_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(245, 220, 230);"));
        right_button = new QRadioButton(centralwidget);
        right_button->setObjectName(QString::fromUtf8("right_button"));
        right_button->setGeometry(QRect(220, 110, 118, 23));
        x_window->setCentralWidget(centralwidget);
        menubar = new QMenuBar(x_window);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 420, 27));
        x_window->setMenuBar(menubar);
        statusbar = new QStatusBar(x_window);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        x_window->setStatusBar(statusbar);

        retranslateUi(x_window);

        QMetaObject::connectSlotsByName(x_window);
    } // setupUi

    void retranslateUi(QMainWindow *x_window)
    {
        x_window->setWindowTitle(QCoreApplication::translate("x_window", "MainWindow", nullptr));
        label_3->setText(QCoreApplication::translate("x_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:10pt; font-weight:700; color:#000000;\">\347\224\265\346\234\272\350\277\220\345\212\250\346\255\245\346\225\260\357\274\232</span></p></body></html>", nullptr));
        label_2->setText(QCoreApplication::translate("x_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:10pt; font-weight:700; color:#000000;\">\347\224\265\346\234\272\346\226\271\345\220\221\357\274\232</span></p></body></html>", nullptr));
        step_set_button->setText(QCoreApplication::translate("x_window", "\350\256\276\347\275\256", nullptr));
        reset_button->setText(QCoreApplication::translate("x_window", "\347\224\265\346\234\272\345\244\215\344\275\215", nullptr));
        lose_button->setText(QCoreApplication::translate("x_window", "\345\244\261\350\203\275", nullptr));
        label_6->setText(QCoreApplication::translate("x_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:12pt; font-weight:700; color:#ff5500;\">\351\276\231\350\212\257\345\210\233\345\215\260</span></p></body></html>", nullptr));
        left_button->setText(QCoreApplication::translate("x_window", "\345\267\246\347\247\273", nullptr));
        label_5->setText(QString());
        lineEdit_2->setText(QCoreApplication::translate("x_window", "\347\224\265\346\234\272\346\234\252\344\275\277\350\203\275", nullptr));
        get_button->setText(QCoreApplication::translate("x_window", "\344\275\277\350\203\275", nullptr));
        label->setText(QCoreApplication::translate("x_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:10pt; font-weight:700; color:#000000;\">\347\224\265\346\234\272\350\260\203\351\200\237\357\274\232</span></p></body></html>", nullptr));
        label_4->setText(QCoreApplication::translate("x_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:11pt; font-weight:700; color:#aa0000;\">X\350\275\264\347\224\265\346\234\272\350\260\203\350\257\225\350\256\276\347\275\256</span></p></body></html>", nullptr));
        pushButton->setText(QCoreApplication::translate("x_window", "\347\224\265\346\234\272\350\277\220\345\212\250", nullptr));
        x_close_button->setText(QCoreApplication::translate("x_window", "\345\205\263\351\227\255", nullptr));
        right_button->setText(QCoreApplication::translate("x_window", "\345\217\263\347\247\273", nullptr));
    } // retranslateUi

};

namespace Ui {
    class x_window: public Ui_x_window {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_X_WINDOW_H
