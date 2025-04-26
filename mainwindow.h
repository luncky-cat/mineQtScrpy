#ifndef MAINWINDOW_H
#define MAINWINDOW_H




class androidManager;

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    androidManager* manger;
   
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
