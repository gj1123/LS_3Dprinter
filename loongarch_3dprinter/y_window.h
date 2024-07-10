#ifndef Y_WINDOW_H
#define Y_WINDOW_H

#include <QMainWindow>

namespace Ui {
class y_window;
}

class y_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit y_window(QWidget *parent = nullptr);
    ~y_window();

private slots:
    void on_y_close_button_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_get_button_clicked();

    void on_lose_button_clicked();

    void on_step_set_button_clicked();

    void on_pushButton_pressed();

    void on_pushButton_released();

    void on_front_button_clicked();

    void on_back_button_clicked();

private:
    Ui::y_window *ui;
};

#endif // Y_WINDOW_H
