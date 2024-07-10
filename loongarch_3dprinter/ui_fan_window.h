/********************************************************************************
** Form generated from reading UI file 'fan_window.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FAN_WINDOW_H
#define UI_FAN_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Fan_window
{
public:
    QWidget *centralwidget;
    QPushButton *fan_close_button;
    QSlider *horizontalSlider;
    QLabel *label_4;
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *start_button;
    QPushButton *stop_button;
    QLabel *label_2;
    QLabel *label_3;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Fan_window)
    {
        if (Fan_window->objectName().isEmpty())
            Fan_window->setObjectName(QString::fromUtf8("Fan_window"));
        Fan_window->resize(355, 325);
        Fan_window->setStyleSheet(QString::fromUtf8("#Fan_window{background-color: rgb(230, 240, 245);}"));
        centralwidget = new QWidget(Fan_window);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        fan_close_button = new QPushButton(centralwidget);
        fan_close_button->setObjectName(QString::fromUtf8("fan_close_button"));
        fan_close_button->setGeometry(QRect(250, 220, 81, 41));
        fan_close_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(245, 220, 230);"));
        horizontalSlider = new QSlider(centralwidget);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(130, 90, 141, 22));
        horizontalSlider->setMaximum(200);
        horizontalSlider->setPageStep(10);
        horizontalSlider->setOrientation(Qt::Horizontal);
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(100, 10, 151, 51));
        QFont font;
        font.setFamily(QString::fromUtf8("\351\273\221\344\275\223"));
        label_4->setFont(font);
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 90, 81, 21));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        label->setFont(font1);
        lineEdit = new QLineEdit(centralwidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(280, 90, 61, 25));
        start_button = new QPushButton(centralwidget);
        start_button->setObjectName(QString::fromUtf8("start_button"));
        start_button->setGeometry(QRect(70, 150, 81, 41));
        start_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);"));
        stop_button = new QPushButton(centralwidget);
        stop_button->setObjectName(QString::fromUtf8("stop_button"));
        stop_button->setGeometry(QRect(180, 150, 81, 41));
        stop_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);"));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(0, 210, 81, 51));
        label_2->setStyleSheet(QString::fromUtf8("border-image: url(:/one.png);"));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 260, 69, 19));
        Fan_window->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Fan_window);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 355, 25));
        Fan_window->setMenuBar(menubar);
        statusbar = new QStatusBar(Fan_window);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        Fan_window->setStatusBar(statusbar);

        retranslateUi(Fan_window);

        QMetaObject::connectSlotsByName(Fan_window);
    } // setupUi

    void retranslateUi(QMainWindow *Fan_window)
    {
        Fan_window->setWindowTitle(QCoreApplication::translate("Fan_window", "MainWindow", nullptr));
        fan_close_button->setText(QCoreApplication::translate("Fan_window", "\345\205\263\351\227\255", nullptr));
        label_4->setText(QCoreApplication::translate("Fan_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:11pt; font-weight:700; color:#00007f;\">\351\243\216\346\211\207\350\260\203\350\257\225\350\256\276\347\275\256</span></p></body></html>", nullptr));
        label->setText(QCoreApplication::translate("Fan_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:10pt; color:#000000;\">\351\243\216\346\211\207\350\260\203\351\200\237\357\274\232</span></p></body></html>", nullptr));
        start_button->setText(QCoreApplication::translate("Fan_window", "\350\256\276\345\256\232", nullptr));
        stop_button->setText(QCoreApplication::translate("Fan_window", "\351\243\216\346\211\207\345\201\234\346\255\242", nullptr));
        label_2->setText(QString());
        label_3->setText(QCoreApplication::translate("Fan_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:10pt; font-weight:700; color:#ff5500;\">\351\276\231\350\212\257\345\210\233\345\215\260</span></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Fan_window: public Ui_Fan_window {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FAN_WINDOW_H
