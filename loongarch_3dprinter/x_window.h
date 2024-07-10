#ifndef X_WINDOW_H
#define X_WINDOW_H

#include <QMainWindow>

namespace Ui {
class x_window;
}

class x_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit x_window(QWidget *parent = nullptr);
    ~x_window();

private slots:
    void on_x_close_button_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_get_button_clicked();

    void on_lose_button_clicked();

    void on_step_set_button_clicked();

    void on_pushButton_pressed();

    void on_pushButton_released();

    void on_left_button_clicked();

    void on_right_button_clicked();

private:
    Ui::x_window *ui;
};

#endif // X_WINDOW_H
