#ifndef FAN_WINDOW_H
#define FAN_WINDOW_H

#include <QMainWindow>

namespace Ui {
class Fan_window;
}

class Fan_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Fan_window(QWidget *parent = nullptr);
    ~Fan_window();

private slots:
    void on_fan_close_button_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_start_button_clicked();

    void on_stop_button_clicked();

private:
    Ui::Fan_window *ui;
};

#endif // FAN_WINDOW_H
