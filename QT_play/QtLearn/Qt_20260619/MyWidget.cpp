#include "MyWidget.h"
#include "ui_MyWidget.h"

#include <QPushButton>
#include <cmath>

MyWidget::MyWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyWidget)
    , m_accumulator(0)
    , m_currentValue(0)
    , m_waitingForOperand(true)
    , m_lastWasEqual(false)
    , m_lastOperand(0)
{
    ui->setupUi(this);

    connect(ui->btn0, &QPushButton::clicked, this, &MyWidget::onDigitClicked);
    connect(ui->btn1, &QPushButton::clicked, this, &MyWidget::onDigitClicked);
    connect(ui->btn2, &QPushButton::clicked, this, &MyWidget::onDigitClicked);
    connect(ui->btn3, &QPushButton::clicked, this, &MyWidget::onDigitClicked);
    connect(ui->btn4, &QPushButton::clicked, this, &MyWidget::onDigitClicked);
    connect(ui->btn5, &QPushButton::clicked, this, &MyWidget::onDigitClicked);
    connect(ui->btn6, &QPushButton::clicked, this, &MyWidget::onDigitClicked);
    connect(ui->btn7, &QPushButton::clicked, this, &MyWidget::onDigitClicked);
    connect(ui->btn8, &QPushButton::clicked, this, &MyWidget::onDigitClicked);
    connect(ui->btn9, &QPushButton::clicked, this, &MyWidget::onDigitClicked);

    connect(ui->btnAdd, &QPushButton::clicked, this, &MyWidget::onOperatorClicked);
    connect(ui->btnSubtract, &QPushButton::clicked, this, &MyWidget::onOperatorClicked);
    connect(ui->btnMultiply, &QPushButton::clicked, this, &MyWidget::onOperatorClicked);
    connect(ui->btnDivide, &QPushButton::clicked, this, &MyWidget::onOperatorClicked);

    connect(ui->btnEqual, &QPushButton::clicked, this, &MyWidget::onEqualClicked);
    connect(ui->btnAC, &QPushButton::clicked, this, &MyWidget::onClearClicked);
    connect(ui->btnSign, &QPushButton::clicked, this, &MyWidget::onSignClicked);
    connect(ui->btnPercent, &QPushButton::clicked, this, &MyWidget::onPercentClicked);
    connect(ui->btnDot, &QPushButton::clicked, this, &MyWidget::onDotClicked);
}

MyWidget::~MyWidget()
{
    delete ui;
}

QString MyWidget::formatNumber(double number)
{
    if (number == static_cast<int>(number) && std::abs(number) < 1e15) {
        return QString::number(static_cast<long long>(number));
    }
    return QString::number(number, 'g', 15);
}

void MyWidget::updateProcessDisplay()
{
    ui->processLabel->setText(m_processString);
}

void MyWidget::onDigitClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;

    QString digit = button->text();

    if (m_waitingForOperand) {
        ui->display->setText(digit);
        m_waitingForOperand = false;
    } else {
        QString currentText = ui->display->text();
        if (currentText == "0" && digit == "0")
            return;
        if (currentText == "0")
            ui->display->setText(digit);
        else
            ui->display->setText(currentText + digit);
    }

    m_lastWasEqual = false;
}

void MyWidget::onOperatorClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;

    QString newOperator = button->text();
    double operand = ui->display->text().toDouble();

    if (!m_pendingOperator.isEmpty() && !m_waitingForOperand) {
        double result = 0;
        if (m_pendingOperator == "+") {
            result = m_accumulator + operand;
        } else if (m_pendingOperator == "-") {
            result = m_accumulator - operand;
        } else if (m_pendingOperator == "×") {
            result = m_accumulator * operand;
        } else if (m_pendingOperator == "÷") {
            if (operand == 0) {
                ui->display->setText("Error");
                m_processString.clear();
                updateProcessDisplay();
                m_pendingOperator.clear();
                m_waitingForOperand = true;
                return;
            }
            result = m_accumulator / operand;
        }
        ui->display->setText(formatNumber(result));
        m_accumulator = result;
        m_processString = formatNumber(result) + " " + newOperator;
    } else {
        m_accumulator = operand;
        m_processString = formatNumber(operand) + " " + newOperator;
    }

    m_pendingOperator = newOperator;
    m_waitingForOperand = true;
    m_lastWasEqual = false;
    updateProcessDisplay();
}

void MyWidget::onEqualClicked()
{
    double operand = ui->display->text().toDouble();
    double result = 0;

    if (m_lastWasEqual && !m_pendingOperator.isEmpty()) {
        operand = m_lastOperand;
    }

    if (!m_pendingOperator.isEmpty()) {
        if (m_pendingOperator == "+") {
            result = m_accumulator + operand;
        } else if (m_pendingOperator == "-") {
            result = m_accumulator - operand;
        } else if (m_pendingOperator == "×") {
            result = m_accumulator * operand;
        } else if (m_pendingOperator == "÷") {
            if (operand == 0) {
                ui->display->setText("Error");
                m_processString.clear();
                updateProcessDisplay();
                m_pendingOperator.clear();
                m_waitingForOperand = true;
                m_lastWasEqual = false;
                return;
            }
            result = m_accumulator / operand;
        }

        if (m_lastWasEqual) {
            m_processString = formatNumber(m_accumulator) + " " + m_lastOperator + " " + formatNumber(m_lastOperand) + " =";
        } else {
            m_processString = formatNumber(m_accumulator) + " " + m_pendingOperator + " " + formatNumber(operand) + " =";
        }

        m_lastOperand = operand;
        m_lastOperator = m_pendingOperator;
        ui->display->setText(formatNumber(result));
        m_accumulator = result;
        m_pendingOperator.clear();
        updateProcessDisplay();
    }

    m_waitingForOperand = true;
    m_lastWasEqual = true;
}

void MyWidget::onClearClicked()
{
    m_accumulator = 0;
    m_currentValue = 0;
    m_pendingOperator.clear();
    m_waitingForOperand = true;
    m_lastWasEqual = false;
    m_lastOperand = 0;
    m_lastOperator.clear();
    m_processString.clear();
    ui->display->setText("0");
    updateProcessDisplay();
}

void MyWidget::onSignClicked()
{
    QString text = ui->display->text();
    double value = text.toDouble();

    if (value != 0) {
        value = -value;
        ui->display->setText(formatNumber(value));
    }
}

void MyWidget::onPercentClicked()
{
    QString text = ui->display->text();
    double value = text.toDouble();

    value = value / 100.0;
    ui->display->setText(formatNumber(value));
}

void MyWidget::onDotClicked()
{
    QString text = ui->display->text();

    if (m_waitingForOperand) {
        ui->display->setText("0.");
        m_waitingForOperand = false;
    } else if (!text.contains('.')) {
        ui->display->setText(text + ".");
    }
}
