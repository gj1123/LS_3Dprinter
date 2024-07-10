#ifndef E_WINDOW_H
#define E_WINDOW_H

#include <QMainWindow>

namespace Ui {
class E_window;
}

class E_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit E_window(QWidget *parent = nullptr);
    ~E_window();

private slots:
    void on_pushButton_clicked();

    void on_get_button_clicked();

    void on_lose_button_clicked();

private:
    Ui::E_window *ui;
};

#endif // E_WINDOW_H
