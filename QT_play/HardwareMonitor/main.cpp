#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Hardware Monitor");

    qRegisterMetaType<CpuInfo>("CpuInfo");
    qRegisterMetaType<GpuInfo>("GpuInfo");
    qRegisterMetaType<MemoryInfo>("MemoryInfo");
    qRegisterMetaType<DiskInfo>("DiskInfo");
    qRegisterMetaType<NetworkInfo>("NetworkInfo");
    qRegisterMetaType<SystemInfo>("SystemInfo");
    qRegisterMetaType<QVector<DiskInfo>>("QVector<DiskInfo>");
    qRegisterMetaType<QVector<NetworkInfo>>("QVector<NetworkInfo>");

    MainWindow window;
    window.show();

    return app.exec();
}
