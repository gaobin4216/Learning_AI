#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class GameWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    GameWidget *gameWidget;
};

#endif // MAINWINDOW_H
