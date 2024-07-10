/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *X_button;
    QLabel *label;
    QPushButton *Y_button;
    QPushButton *Z_button;
    QPushButton *Extruded_button;
    QPushButton *Fan_button;
    QPushButton *Fire_extruded_button;
    QPushButton *Fire_chuang_button;
    QPushButton *action_Button;
    QPushButton *Stop_button;
    QLabel *label_3;
    QLabel *label_2;
    QLabel *label_4;
    QLabel *label_5;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(658, 427);
        MainWindow->setStyleSheet(QString::fromUtf8("#MainWindow{background-image: url(:/background.png);}"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        X_button = new QPushButton(centralwidget);
        X_button->setObjectName(QString::fromUtf8("X_button"));
        X_button->setGeometry(QRect(90, 150, 101, 41));
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        font.setPointSize(10);
        X_button->setFont(font);
        X_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(230, 240, 245);\n"
""));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 20, 111, 41));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\345\256\213\344\275\223"));
        label->setFont(font1);
        Y_button = new QPushButton(centralwidget);
        Y_button->setObjectName(QString::fromUtf8("Y_button"));
        Y_button->setGeometry(QRect(90, 220, 101, 41));
        Y_button->setFont(font);
        Y_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(230, 240, 245);"));
        Z_button = new QPushButton(centralwidget);
        Z_button->setObjectName(QString::fromUtf8("Z_button"));
        Z_button->setGeometry(QRect(230, 150, 101, 41));
        Z_button->setFont(font);
        Z_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(230, 240, 245);"));
        Extruded_button = new QPushButton(centralwidget);
        Extruded_button->setObjectName(QString::fromUtf8("Extruded_button"));
        Extruded_button->setGeometry(QRect(230, 220, 101, 41));
        Extruded_button->setFont(font);
        Extruded_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(230, 240, 245);"));
        Fan_button = new QPushButton(centralwidget);
        Fan_button->setObjectName(QString::fromUtf8("Fan_button"));
        Fan_button->setGeometry(QRect(170, 290, 91, 41));
        Fan_button->setFont(font);
        Fan_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(210, 230, 250);"));
        Fire_extruded_button = new QPushButton(centralwidget);
        Fire_extruded_button->setObjectName(QString::fromUtf8("Fire_extruded_button"));
        Fire_extruded_button->setGeometry(QRect(50, 290, 91, 41));
        Fire_extruded_button->setFont(font);
        Fire_extruded_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(210, 230, 250);"));
        Fire_chuang_button = new QPushButton(centralwidget);
        Fire_chuang_button->setObjectName(QString::fromUtf8("Fire_chuang_button"));
        Fire_chuang_button->setGeometry(QRect(290, 290, 91, 41));
        Fire_chuang_button->setFont(font);
        Fire_chuang_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(210, 230, 250);"));
        action_Button = new QPushButton(centralwidget);
        action_Button->setObjectName(QString::fromUtf8("action_Button"));
        action_Button->setGeometry(QRect(460, 160, 141, 51));
        action_Button->setStyleSheet(QString::fromUtf8("border:4px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(230,240,220);\n"
""));
        Stop_button = new QPushButton(centralwidget);
        Stop_button->setObjectName(QString::fromUtf8("Stop_button"));
        Stop_button->setGeometry(QRect(460, 270, 141, 51));
        Stop_button->setStyleSheet(QString::fromUtf8("border:4px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(235, 230, 240);\n"
""));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(530, 10, 121, 61));
        label_3->setStyleSheet(QString::fromUtf8("border-image: url(:/one.png);"));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(90, 90, 101, 41));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(130, 20, 381, 41));
        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(20, 90, 61, 51));
        label_5->setStyleSheet(QString::fromUtf8("border-image: url(:/aaaa.png);"));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 658, 27));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        X_button->setText(QCoreApplication::translate("MainWindow", "X\350\275\264", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p align=\"center\"><span style=\" font-size:12pt; font-weight:700; color:#aaffff;\">\351\276\231\350\212\257\345\210\233\345\215\260</span></p></body></html>", nullptr));
        Y_button->setText(QCoreApplication::translate("MainWindow", "Y\350\275\264", nullptr));
        Z_button->setText(QCoreApplication::translate("MainWindow", "Z\350\275\264", nullptr));
        Extruded_button->setText(QCoreApplication::translate("MainWindow", "\346\214\244\345\207\272", nullptr));
        Fan_button->setText(QCoreApplication::translate("MainWindow", "\351\243\216\346\211\207", nullptr));
        Fire_extruded_button->setText(QCoreApplication::translate("MainWindow", "\346\214\244\345\207\272\345\212\240\347\203\255", nullptr));
        Fire_chuang_button->setText(QCoreApplication::translate("MainWindow", "\347\203\255\345\272\212", nullptr));
        action_Button->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213\346\211\223\345\215\260", nullptr));
        Stop_button->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242\346\211\223\345\215\260", nullptr));
        label_3->setText(QString());
        label_2->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p align=\"center\"><span style=\" font-size:12pt; font-weight:700; color:#ffffff;\">\350\260\203\350\257\225\350\256\276\347\275\256\357\274\232</span></p></body></html>", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p align=\"center\"><span style=\" font-size:14pt; font-weight:700; color:#ffffff;\">\345\237\272\344\272\216\351\276\231\350\212\257\346\225\231\350\202\262\346\264\276\347\232\204\345\233\275\344\272\2473D\346\211\223\345\215\260\347\263\273\347\273\237</span></p></body></html>", nullptr));
        label_5->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
