#include <QApplication>
#include "FireworkWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FireworkWidget w;
    w.show();
    return app.exec();
}
