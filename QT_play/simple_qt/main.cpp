#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("简单 Qt 应用");
    window.setFixedSize(400, 300);

    auto *layout = new QVBoxLayout(&window);

    auto *label = new QLabel("你好，Qt！");
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");

    auto *button = new QPushButton("点我");
    button->setStyleSheet(
        "QPushButton {"
        "  font-size: 16px;"
        "  padding: 10px 30px;"
        "  background-color: #3498db;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "}"
        "QPushButton:hover { background-color: #2980b9; }"
    );

    auto *quitButton = new QPushButton("退出");
    quitButton->setStyleSheet(
        "QPushButton {"
        "  font-size: 14px;"
        "  padding: 8px 25px;"
        "  background-color: #e74c3c;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "}"
        "QPushButton:hover { background-color: #c0392b; }"
    );

    QObject::connect(button, &QPushButton::clicked, [label]() {
        label->setText("按钮被点击了！");
    });

    QObject::connect(quitButton, &QPushButton::clicked, &window, &QWidget::close);

    layout->addStretch();
    layout->addWidget(label);
    layout->addWidget(button, 0, Qt::AlignCenter);
    layout->addWidget(quitButton, 0, Qt::AlignCenter);
    layout->addStretch();

    window.show();
    return app.exec();
}
