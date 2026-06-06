#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QVector>
#include <QThread>
#include "HardwareMonitor.h"

class QGroupBox;
class QVBoxLayout;

// Collects hardware data in a background thread
class MonitorWorker : public QObject
{
    Q_OBJECT

public:
    MonitorWorker();
    ~MonitorWorker();

public slots:
    void collect();

signals:
    void dataReady(CpuInfo cpu, GpuInfo gpu, MemoryInfo mem,
                   QVector<DiskInfo> disks, QVector<NetworkInfo> nets, SystemInfo sys);

private:
    HardwareMonitor m_monitor;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDataReady(CpuInfo cpu, GpuInfo gpu, MemoryInfo mem,
                     QVector<DiskInfo> disks, QVector<NetworkInfo> nets, SystemInfo sys);

private:
    void setupUi();
    void applyDarkTheme();

    void updateOverview(const CpuInfo& cpu, const MemoryInfo& mem, const QVector<DiskInfo>& disks);
    void updateCpu(const CpuInfo& cpu);
    void updateGpu(const GpuInfo& gpu);
    void updateMemory(const MemoryInfo& mem);
    void updateDisk(const QVector<DiskInfo>& disks);
    void updateNetwork(const QVector<NetworkInfo>& nets);
    void updateSystem(const SystemInfo& sys);

    static QString formatBytes(quint64 bytes);

    QThread* m_workerThread;
    MonitorWorker* m_worker;
    QTimer* m_timer;

    // Overview tab widgets
    QLabel* m_overviewCpuLabel;
    QProgressBar* m_overviewCpuBar;
    QLabel* m_overviewMemLabel;
    QProgressBar* m_overviewMemBar;
    QVector<QProgressBar*> m_overviewDiskBars;
    QVector<QLabel*> m_overviewDiskLabels;
    QGroupBox* m_overviewDiskGroup = nullptr;
    QVBoxLayout* m_overviewDiskLayout = nullptr;

    // CPU tab widgets
    QLabel* m_cpuName;
    QLabel* m_cpuCores;
    QLabel* m_cpuThreads;
    QLabel* m_cpuLoad;
    QLabel* m_cpuTemp;
    QLabel* m_cpuMaxClock;
    QLabel* m_cpuCurClock;
    QProgressBar* m_cpuLoadBar;
    QVector<QProgressBar*> m_coreBars;
    QVector<QLabel*> m_coreLabels;

    // GPU tab widgets
    QLabel* m_gpuName;
    QLabel* m_gpuProcessor;
    QLabel* m_gpuMemory;
    QLabel* m_gpuDriver;
    QLabel* m_gpuDriverDate;
    QProgressBar* m_gpuMemBar;

    // Memory tab widgets
    QLabel* m_memTotalPhys;
    QLabel* m_memAvailPhys;
    QLabel* m_memUsedPhys;
    QLabel* m_memUsagePercent;
    QProgressBar* m_memBar;
    QLabel* m_memTotalVirt;
    QLabel* m_memAvailVirt;
    QLabel* m_memTotalPage;
    QLabel* m_memAvailPage;

    // Disk tab widgets
    QVector<QLabel*> m_diskDeviceLabels;
    QVector<QLabel*> m_diskNameLabels;
    QVector<QLabel*> m_diskFsLabels;
    QVector<QLabel*> m_diskTotalLabels;
    QVector<QLabel*> m_diskUsedLabels;
    QVector<QLabel*> m_diskFreeLabels;
    QVector<QProgressBar*> m_diskBars;

    // Network tab widgets
    QVector<QLabel*> m_netNameLabels;
    QVector<QLabel*> m_netIpLabels;
    QVector<QLabel*> m_netMacLabels;
    QVector<QLabel*> m_netDownLabels;
    QVector<QLabel*> m_netUpLabels;

    // System tab widgets
    QLabel* m_sysOsName;
    QLabel* m_sysOsVersion;
    QLabel* m_sysComputer;
    QLabel* m_sysUser;
    QLabel* m_sysModel;
    QLabel* m_sysManufacturer;
};

#endif // MAINWINDOW_H
