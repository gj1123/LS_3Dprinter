#ifndef FIRE_C_WINDOW_H
#define FIRE_C_WINDOW_H

#include <QMainWindow>

namespace Ui {
class Fire_c_window;
}

class Fire_c_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Fire_c_window(QWidget *parent = nullptr);
    ~Fire_c_window();

private slots:
    void on_pushButton_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_start_button_clicked();

    void on_stop_button_clicked();

private:
    Ui::Fire_c_window *ui;
};

#endif // FIRE_C_WINDOW_H
