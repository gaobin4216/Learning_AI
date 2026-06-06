#ifndef HARDWAREMONITOR_H
#define HARDWAREMONITOR_H

#include <QString>
#include <QVector>
#include <QMetaType>
#include <windows.h>
#include <wbemcli.h>
#include <wbemidl.h>

struct CpuInfo {
    QString name;
    int cores = 0;
    int logicalProcessors = 0;
    double loadPercent = 0.0;
    double temperature = -1.0;
    QString maxClockSpeed;
    QString currentClockSpeed;
};

struct GpuInfo {
    QString name;
    quint64 adapterRam = 0;
    QString driverVersion;
    QString driverDate;
    QString videoProcessor;
    double loadPercent = -1.0;
    double temperature = -1.0;
};

struct MemoryInfo {
    quint64 totalPhys = 0;
    quint64 availPhys = 0;
    quint64 totalVirtual = 0;
    quint64 availVirtual = 0;
    quint64 totalPageFile = 0;
    quint64 availPageFile = 0;
};

struct DiskInfo {
    QString deviceID;
    QString volumeName;
    QString fileSystem;
    quint64 totalSize = 0;
    quint64 freeSpace = 0;
    quint64 usedSpace = 0;
};

struct NetworkInfo {
    QString name;
    QString macAddress;
    QString ipAddress;
    bool isConnected = false;
    quint64 bytesReceived = 0;
    quint64 bytesSent = 0;
    double downloadSpeed = 0.0;
    double uploadSpeed = 0.0;
};

struct SystemInfo {
    QString osName;
    QString osVersion;
    QString computerName;
    QString userName;
    QString systemModel;
    QString systemManufacturer;
};

Q_DECLARE_METATYPE(CpuInfo)
Q_DECLARE_METATYPE(GpuInfo)
Q_DECLARE_METATYPE(MemoryInfo)
Q_DECLARE_METATYPE(DiskInfo)
Q_DECLARE_METATYPE(NetworkInfo)
Q_DECLARE_METATYPE(SystemInfo)
Q_DECLARE_METATYPE(QVector<DiskInfo>)
Q_DECLARE_METATYPE(QVector<NetworkInfo>)

class HardwareMonitor
{
public:
    HardwareMonitor();
    ~HardwareMonitor();

    bool initialize();

    CpuInfo getCpuInfo();
    GpuInfo getGpuInfo();
    MemoryInfo getMemoryInfo();
    QVector<DiskInfo> getDiskInfo();
    QVector<NetworkInfo> getNetworkInfo();
    SystemInfo getSystemInfo();

private:
    IEnumWbemClassObject* executeQuery(const wchar_t* query);
    QString getStringProperty(IWbemClassObject* obj, const wchar_t* name);
    quint64 getUint64Property(IWbemClassObject* obj, const wchar_t* name);
    quint32 getUint32Property(IWbemClassObject* obj, const wchar_t* name);
    double getDoubleProperty(IWbemClassObject* obj, const wchar_t* name);

    IWbemServices* m_services = nullptr;
    IWbemLocator* m_locator = nullptr;
};

#endif // HARDWAREMONITOR_H
