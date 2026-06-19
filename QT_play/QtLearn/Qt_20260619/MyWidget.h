#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MyWidget;
}
QT_END_NAMESPACE

class MyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyWidget(QWidget *parent = nullptr);
    ~MyWidget() override;

private slots:
    void onDigitClicked();
    void onOperatorClicked();
    void onEqualClicked();
    void onClearClicked();
    void onSignClicked();
    void onPercentClicked();
    void onDotClicked();

private:
    Ui::MyWidget *ui;

    double m_accumulator;
    double m_currentValue;
    QString m_pendingOperator;
    bool m_waitingForOperand;
    bool m_lastWasEqual;
    double m_lastOperand;
    QString m_lastOperator;
    QString m_processString;

    void updateProcessDisplay();
    QString formatNumber(double number);
};

#endif // MYWIDGET_H
