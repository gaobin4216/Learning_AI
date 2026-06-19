#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QWidget>

class MyWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MyWindow(QWidget *parent = nullptr);
    ~MyWindow() override;
};
#endif // MYWINDOW_H
