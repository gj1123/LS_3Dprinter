#ifndef FIRE_E_WINDOW_H
#define FIRE_E_WINDOW_H

#include <QMainWindow>

namespace Ui {
class Fire_e_window;
}

class Fire_e_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Fire_e_window(QWidget *parent = nullptr);
    ~Fire_e_window();

private slots:
    void on_Fire_e_close_button_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_start_button_clicked();

    void on_stop_button_clicked();

private:
    Ui::Fire_e_window *ui;
};

#endif // FIRE_E_WINDOW_H
