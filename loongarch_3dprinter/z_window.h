#ifndef Z_WINDOW_H
#define Z_WINDOW_H

#include <QMainWindow>

namespace Ui {
class z_window;
}

class z_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit z_window(QWidget *parent = nullptr);
    ~z_window();

private slots:
    void on_z_close_button_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_get_button_clicked();

    void on_lose_button_clicked();

    void on_step_set_button_clicked();

    void on_pushButton_pressed();

    void on_pushButton_released();

    void on_up_button_clicked();

    void on_down_button_clicked();

private:
    Ui::z_window *ui;
};

#endif // Z_WINDOW_H
