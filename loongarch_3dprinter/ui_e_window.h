/********************************************************************************
** Form generated from reading UI file 'e_window.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_E_WINDOW_H
#define UI_E_WINDOW_H

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

class Ui_E_window
{
public:
    QWidget *centralwidget;
    QPushButton *pushButton;
    QLineEdit *lineEdit;
    QLabel *label;
    QLabel *label_4;
    QPushButton *get_button;
    QSlider *horizontalSlider;
    QPushButton *reset_button;
    QPushButton *lose_button;
    QLineEdit *lineEdit_2;
    QPushButton *pushButton_2;
    QLabel *label_2;
    QLabel *label_3;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *E_window)
    {
        if (E_window->objectName().isEmpty())
            E_window->setObjectName(QString::fromUtf8("E_window"));
        E_window->resize(360, 353);
        E_window->setStyleSheet(QString::fromUtf8("#E_window{background-color: rgb(230, 240, 245);}"));
        centralwidget = new QWidget(E_window);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(260, 250, 81, 41));
        pushButton->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(245, 220, 230);"));
        lineEdit = new QLineEdit(centralwidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(290, 150, 61, 25));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 150, 101, 21));
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221"));
        label->setFont(font);
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(100, 10, 151, 51));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\351\273\221\344\275\223"));
        label_4->setFont(font1);
        get_button = new QPushButton(centralwidget);
        get_button->setObjectName(QString::fromUtf8("get_button"));
        get_button->setGeometry(QRect(40, 80, 61, 28));
        get_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);"));
        horizontalSlider = new QSlider(centralwidget);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(140, 150, 141, 22));
        horizontalSlider->setMaximum(1000);
        horizontalSlider->setPageStep(100);
        horizontalSlider->setOrientation(Qt::Horizontal);
        reset_button = new QPushButton(centralwidget);
        reset_button->setObjectName(QString::fromUtf8("reset_button"));
        reset_button->setGeometry(QRect(160, 200, 81, 41));
        reset_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);"));
        lose_button = new QPushButton(centralwidget);
        lose_button->setObjectName(QString::fromUtf8("lose_button"));
        lose_button->setGeometry(QRect(130, 80, 61, 28));
        lose_button->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);"));
        lineEdit_2 = new QLineEdit(centralwidget);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));
        lineEdit_2->setGeometry(QRect(220, 80, 91, 25));
        pushButton_2 = new QPushButton(centralwidget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(40, 200, 81, 41));
        pushButton_2->setStyleSheet(QString::fromUtf8("border:2px groove gray;border-radius:\n"
"4px;padding:2px 4px;\n"
"background-color: rgb(255, 255, 255);"));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(0, 260, 81, 31));
        label_2->setStyleSheet(QString::fromUtf8("border-image: url(:/one.png);"));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 300, 69, 19));
        E_window->setCentralWidget(centralwidget);
        menubar = new QMenuBar(E_window);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 360, 25));
        E_window->setMenuBar(menubar);
        statusbar = new QStatusBar(E_window);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        E_window->setStatusBar(statusbar);

        retranslateUi(E_window);

        QMetaObject::connectSlotsByName(E_window);
    } // setupUi

    void retranslateUi(QMainWindow *E_window)
    {
        E_window->setWindowTitle(QCoreApplication::translate("E_window", "MainWindow", nullptr));
        pushButton->setText(QCoreApplication::translate("E_window", "\345\205\263\351\227\255", nullptr));
        label->setText(QCoreApplication::translate("E_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:10pt; color:#000000;\">\346\214\244\345\207\272\351\207\217\350\256\276\347\275\256\357\274\232</span></p></body></html>", nullptr));
        label_4->setText(QCoreApplication::translate("E_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:11pt; font-weight:700; color:#aa0000;\">\346\214\244\345\207\272\347\224\265\346\234\272\350\260\203\350\257\225\350\256\276\347\275\256</span></p></body></html>", nullptr));
        get_button->setText(QCoreApplication::translate("E_window", "\344\275\277\350\203\275", nullptr));
        reset_button->setText(QCoreApplication::translate("E_window", "\345\201\234\346\255\242\346\214\244\345\207\272", nullptr));
        lose_button->setText(QCoreApplication::translate("E_window", "\345\244\261\350\203\275", nullptr));
        lineEdit_2->setText(QCoreApplication::translate("E_window", "\347\224\265\346\234\272\346\234\252\344\275\277\350\203\275", nullptr));
        pushButton_2->setText(QCoreApplication::translate("E_window", "\345\274\200\345\247\213\346\214\244\345\207\272", nullptr));
        label_2->setText(QString());
        label_3->setText(QCoreApplication::translate("E_window", "<html><head/><body><p align=\"center\"><span style=\" font-size:10pt; font-weight:700; color:#ff5500;\">\351\276\231\350\212\257\345\210\233\345\215\260</span></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class E_window: public Ui_E_window {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_E_WINDOW_H
