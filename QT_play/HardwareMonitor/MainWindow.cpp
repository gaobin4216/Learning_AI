#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QDateTime>
#include <QFrame>
#include <QStatusBar>

// --- MonitorWorker ---

MonitorWorker::MonitorWorker() {}

MonitorWorker::~MonitorWorker() {}

void MonitorWorker::collect()
{
    static bool initialized = false;
    if (!initialized) {
        if (!m_monitor.initialize())
            return;
        initialized = true;
    }

    CpuInfo cpu = m_monitor.getCpuInfo();
    GpuInfo gpu = m_monitor.getGpuInfo();
    MemoryInfo mem = m_monitor.getMemoryInfo();
    QVector<DiskInfo> disks = m_monitor.getDiskInfo();
    QVector<NetworkInfo> nets = m_monitor.getNetworkInfo();
    SystemInfo sys = m_monitor.getSystemInfo();

    emit dataReady(cpu, gpu, mem, disks, nets, sys);
}

// --- MainWindow ---

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUi();
    applyDarkTheme();

    // Background worker thread
    m_workerThread = new QThread(this);
    m_worker = new MonitorWorker();
    m_worker->moveToThread(m_workerThread);

    // Timer in the worker thread triggers data collection
    m_timer = new QTimer();
    m_timer->setInterval(2000);
    m_timer->moveToThread(m_workerThread);
    connect(m_timer, &QTimer::timeout, m_worker, &MonitorWorker::collect);
    connect(m_workerThread, &QThread::started, m_timer, qOverload<>(&QTimer::start));
    connect(m_worker, &MonitorWorker::dataReady, this, &MainWindow::onDataReady, Qt::QueuedConnection);

    m_workerThread->start();

    statusBar()->showMessage("Initializing...");
}

MainWindow::~MainWindow()
{
    m_workerThread->quit();
    m_workerThread->wait();
    delete m_worker;
}

void MainWindow::setupUi()
{
    setWindowTitle("Hardware Monitor");
    setMinimumSize(800, 600);
    resize(900, 650);

    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* mainLayout = new QVBoxLayout(central);

    auto* tabs = new QTabWidget(this);
    mainLayout->addWidget(tabs);

    // Overview tab
    {
        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        auto* widget = new QWidget();
        auto* layout = new QVBoxLayout(widget);

        auto* cpuGroup = new QGroupBox("CPU");
        auto* cpuLayout = new QVBoxLayout(cpuGroup);
        m_overviewCpuLabel = new QLabel("--");
        m_overviewCpuBar = new QProgressBar();
        m_overviewCpuBar->setRange(0, 100);
        cpuLayout->addWidget(m_overviewCpuLabel);
        cpuLayout->addWidget(m_overviewCpuBar);
        layout->addWidget(cpuGroup);

        auto* memGroup = new QGroupBox("Memory");
        auto* memLayout = new QVBoxLayout(memGroup);
        m_overviewMemLabel = new QLabel("--");
        m_overviewMemBar = new QProgressBar();
        m_overviewMemBar->setRange(0, 100);
        memLayout->addWidget(m_overviewMemLabel);
        memLayout->addWidget(m_overviewMemBar);
        layout->addWidget(memGroup);

        m_overviewDiskGroup = new QGroupBox("Disks");
        m_overviewDiskLayout = new QVBoxLayout(m_overviewDiskGroup);
        layout->addWidget(m_overviewDiskGroup);

        layout->addStretch();
        scroll->setWidget(widget);
        tabs->addTab(scroll, "Overview");
    }

    // CPU tab
    {
        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        auto* widget = new QWidget();
        auto* layout = new QVBoxLayout(widget);

        auto* infoGroup = new QGroupBox("CPU Information");
        auto* infoGrid = new QGridLayout(infoGroup);
        m_cpuName = new QLabel("--");
        m_cpuCores = new QLabel("--");
        m_cpuThreads = new QLabel("--");
        m_cpuMaxClock = new QLabel("--");
        m_cpuCurClock = new QLabel("--");
        m_cpuLoad = new QLabel("--");
        m_cpuTemp = new QLabel("--");
        m_cpuLoadBar = new QProgressBar();
        m_cpuLoadBar->setRange(0, 100);

        infoGrid->addWidget(new QLabel("Name:"), 0, 0);
        infoGrid->addWidget(m_cpuName, 0, 1);
        infoGrid->addWidget(new QLabel("Cores:"), 1, 0);
        infoGrid->addWidget(m_cpuCores, 1, 1);
        infoGrid->addWidget(new QLabel("Threads:"), 2, 0);
        infoGrid->addWidget(m_cpuThreads, 2, 1);
        infoGrid->addWidget(new QLabel("Max Clock:"), 3, 0);
        infoGrid->addWidget(m_cpuMaxClock, 3, 1);
        infoGrid->addWidget(new QLabel("Current Clock:"), 4, 0);
        infoGrid->addWidget(m_cpuCurClock, 4, 1);
        infoGrid->addWidget(new QLabel("Temperature:"), 5, 0);
        infoGrid->addWidget(m_cpuTemp, 5, 1);
        infoGrid->addWidget(new QLabel("CPU Load:"), 6, 0);
        infoGrid->addWidget(m_cpuLoad, 6, 1);
        infoGrid->addWidget(m_cpuLoadBar, 7, 0, 1, 2);
        layout->addWidget(infoGroup);

        auto* coreGroup = new QGroupBox("Per-Core Usage");
        auto* coreLayout = new QVBoxLayout(coreGroup);
        layout->addWidget(coreGroup);

        layout->addStretch();
        scroll->setWidget(widget);
        tabs->addTab(scroll, "CPU");
    }

    // GPU tab
    {
        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        auto* widget = new QWidget();
        auto* layout = new QVBoxLayout(widget);

        auto* infoGroup = new QGroupBox("GPU Information");
        auto* infoGrid = new QGridLayout(infoGroup);
        m_gpuName = new QLabel("--");
        m_gpuProcessor = new QLabel("--");
        m_gpuMemory = new QLabel("--");
        m_gpuDriver = new QLabel("--");
        m_gpuDriverDate = new QLabel("--");
        m_gpuMemBar = new QProgressBar();
        m_gpuMemBar->setRange(0, 100);

        infoGrid->addWidget(new QLabel("Name:"), 0, 0);
        infoGrid->addWidget(m_gpuName, 0, 1);
        infoGrid->addWidget(new QLabel("Video Processor:"), 1, 0);
        infoGrid->addWidget(m_gpuProcessor, 1, 1);
        infoGrid->addWidget(new QLabel("Adapter RAM:"), 2, 0);
        infoGrid->addWidget(m_gpuMemory, 2, 1);
        infoGrid->addWidget(new QLabel("Driver Version:"), 3, 0);
        infoGrid->addWidget(m_gpuDriver, 3, 1);
        infoGrid->addWidget(new QLabel("Driver Date:"), 4, 0);
        infoGrid->addWidget(m_gpuDriverDate, 4, 1);
        layout->addWidget(infoGroup);

        layout->addStretch();
        scroll->setWidget(widget);
        tabs->addTab(scroll, "GPU");
    }

    // Memory tab
    {
        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        auto* widget = new QWidget();
        auto* layout = new QVBoxLayout(widget);

        auto* physGroup = new QGroupBox("Physical Memory");
        auto* physGrid = new QGridLayout(physGroup);
        m_memTotalPhys = new QLabel("--");
        m_memAvailPhys = new QLabel("--");
        m_memUsedPhys = new QLabel("--");
        m_memUsagePercent = new QLabel("--");
        m_memBar = new QProgressBar();
        m_memBar->setRange(0, 100);

        physGrid->addWidget(new QLabel("Total:"), 0, 0);
        physGrid->addWidget(m_memTotalPhys, 0, 1);
        physGrid->addWidget(new QLabel("Available:"), 1, 0);
        physGrid->addWidget(m_memAvailPhys, 1, 1);
        physGrid->addWidget(new QLabel("Used:"), 2, 0);
        physGrid->addWidget(m_memUsedPhys, 2, 1);
        physGrid->addWidget(new QLabel("Usage:"), 3, 0);
        physGrid->addWidget(m_memUsagePercent, 3, 1);
        physGrid->addWidget(m_memBar, 4, 0, 1, 2);
        layout->addWidget(physGroup);

        auto* virtGroup = new QGroupBox("Virtual Memory");
        auto* virtGrid = new QGridLayout(virtGroup);
        m_memTotalVirt = new QLabel("--");
        m_memAvailVirt = new QLabel("--");
        m_memTotalPage = new QLabel("--");
        m_memAvailPage = new QLabel("--");

        virtGrid->addWidget(new QLabel("Total Virtual:"), 0, 0);
        virtGrid->addWidget(m_memTotalVirt, 0, 1);
        virtGrid->addWidget(new QLabel("Available Virtual:"), 1, 0);
        virtGrid->addWidget(m_memAvailVirt, 1, 1);
        virtGrid->addWidget(new QLabel("Total Page File:"), 2, 0);
        virtGrid->addWidget(m_memTotalPage, 2, 1);
        virtGrid->addWidget(new QLabel("Available Page File:"), 3, 0);
        virtGrid->addWidget(m_memAvailPage, 3, 1);
        layout->addWidget(virtGroup);

        layout->addStretch();
        scroll->setWidget(widget);
        tabs->addTab(scroll, "Memory");
    }

    // Disk tab
    {
        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        auto* widget = new QWidget();
        auto* layout = new QVBoxLayout(widget);
        layout->addStretch();
        scroll->setWidget(widget);
        tabs->addTab(scroll, "Disk");
    }

    // Network tab
    {
        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        auto* widget = new QWidget();
        auto* layout = new QVBoxLayout(widget);
        layout->addStretch();
        scroll->setWidget(widget);
        tabs->addTab(scroll, "Network");
    }

    // System tab
    {
        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        auto* widget = new QWidget();
        auto* layout = new QVBoxLayout(widget);

        auto* infoGroup = new QGroupBox("System Information");
        auto* infoGrid = new QGridLayout(infoGroup);
        m_sysOsName = new QLabel("--");
        m_sysOsVersion = new QLabel("--");
        m_sysComputer = new QLabel("--");
        m_sysUser = new QLabel("--");
        m_sysModel = new QLabel("--");
        m_sysManufacturer = new QLabel("--");

        infoGrid->addWidget(new QLabel("OS Name:"), 0, 0);
        infoGrid->addWidget(m_sysOsName, 0, 1);
        infoGrid->addWidget(new QLabel("OS Version:"), 1, 0);
        infoGrid->addWidget(m_sysOsVersion, 1, 1);
        infoGrid->addWidget(new QLabel("Computer Name:"), 2, 0);
        infoGrid->addWidget(m_sysComputer, 2, 1);
        infoGrid->addWidget(new QLabel("User:"), 3, 0);
        infoGrid->addWidget(m_sysUser, 3, 1);
        infoGrid->addWidget(new QLabel("Manufacturer:"), 4, 0);
        infoGrid->addWidget(m_sysManufacturer, 4, 1);
        infoGrid->addWidget(new QLabel("Model:"), 5, 0);
        infoGrid->addWidget(m_sysModel, 5, 1);
        layout->addWidget(infoGroup);

        layout->addStretch();
        scroll->setWidget(widget);
        tabs->addTab(scroll, "System");
    }
}

void MainWindow::applyDarkTheme()
{
    QString style = R"(
        QMainWindow, QWidget {
            background-color: #1e1e2e;
            color: #cdd6f4;
            font-family: "Segoe UI", "Microsoft YaHei", sans-serif;
            font-size: 13px;
        }
        QTabWidget::pane {
            border: 1px solid #45475a;
            background-color: #1e1e2e;
        }
        QTabBar::tab {
            background-color: #313244;
            color: #a6adc8;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
        }
        QTabBar::tab:selected {
            background-color: #45475a;
            color: #cdd6f4;
        }
        QTabBar::tab:hover {
            background-color: #585b70;
        }
        QGroupBox {
            border: 1px solid #45475a;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 16px;
            font-weight: bold;
            color: #89b4fa;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
        }
        QLabel {
            color: #cdd6f4;
            padding: 2px;
        }
        QProgressBar {
            border: 1px solid #45475a;
            border-radius: 4px;
            text-align: center;
            background-color: #313244;
            color: #cdd6f4;
            height: 22px;
        }
        QProgressBar::chunk {
            background-color: #89b4fa;
            border-radius: 3px;
        }
        QScrollArea {
            border: none;
            background-color: #1e1e2e;
        }
        QStatusBar {
            background-color: #181825;
            color: #a6adc8;
            border-top: 1px solid #313244;
        }
    )";
    setStyleSheet(style);
}

QString MainWindow::formatBytes(quint64 bytes)
{
    if (bytes >= 1073741824ULL) {
        return QString::number(bytes / 1073741824.0, 'f', 2) + " GiB";
    } else if (bytes >= 1048576ULL) {
        return QString::number(bytes / 1048576.0, 'f', 2) + " MiB";
    } else if (bytes >= 1024ULL) {
        return QString::number(bytes / 1024.0, 'f', 2) + " KiB";
    }
    return QString::number(bytes) + " B";
}

void MainWindow::onDataReady(CpuInfo cpu, GpuInfo gpu, MemoryInfo mem,
                              QVector<DiskInfo> disks, QVector<NetworkInfo> nets, SystemInfo sys)
{
    updateOverview(cpu, mem, disks);
    updateCpu(cpu);
    updateGpu(gpu);
    updateMemory(mem);
    updateDisk(disks);
    updateNetwork(nets);
    updateSystem(sys);

    double memPercent = mem.totalPhys > 0
        ? (1.0 - (double)mem.availPhys / mem.totalPhys) * 100.0 : 0.0;
    statusBar()->showMessage(
        QString("Last updated: %1 | CPU: %2% | Memory: %3%")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
            .arg(cpu.loadPercent, 0, 'f', 1)
            .arg(memPercent, 0, 'f', 1));
}

void MainWindow::updateOverview(const CpuInfo& cpu, const MemoryInfo& mem, const QVector<DiskInfo>& disks)
{
    m_overviewCpuLabel->setText(
        QString("%1 | Load: %2%")
            .arg(cpu.name.isEmpty() ? "N/A" : cpu.name)
            .arg(cpu.loadPercent, 0, 'f', 1));
    m_overviewCpuBar->setValue((int)cpu.loadPercent);

    double memPercent = mem.totalPhys > 0
        ? (1.0 - (double)mem.availPhys / mem.totalPhys) * 100.0 : 0.0;
    m_overviewMemLabel->setText(
        QString("%1 / %2 (%3%)")
            .arg(formatBytes(mem.totalPhys - mem.availPhys))
            .arg(formatBytes(mem.totalPhys))
            .arg(memPercent, 0, 'f', 1));
    m_overviewMemBar->setValue((int)memPercent);

    // Color bars
    int cpuLoad = (int)cpu.loadPercent;
    QString cpuColor = cpuLoad < 60 ? "#a6e3a1" : (cpuLoad < 85 ? "#f9e2af" : "#f38ba8");
    m_overviewCpuBar->setStyleSheet(
        QString("QProgressBar::chunk { background-color: %1; border-radius: 3px; }").arg(cpuColor));

    QString memColor = memPercent < 70 ? "#a6e3a1" : (memPercent < 90 ? "#f9e2af" : "#f38ba8");
    m_overviewMemBar->setStyleSheet(
        QString("QProgressBar::chunk { background-color: %1; border-radius: 3px; }").arg(memColor));

    // Update disk overview bars
    if (m_overviewDiskLayout) {
        while (m_overviewDiskLayout->count() > 0) {
            auto* item = m_overviewDiskLayout->takeAt(0);
            if (item->widget()) delete item->widget();
            delete item;
        }

        for (const auto& disk : disks) {
            double percent = disk.totalSize > 0
                ? (double)disk.usedSpace / disk.totalSize * 100.0 : 0.0;
            auto* row = new QWidget();
            auto* rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(0, 0, 0, 0);
            auto* label = new QLabel(QString("%1 (%2) - %3 / %4")
                .arg(disk.deviceID,
                     disk.volumeName.isEmpty() ? "Local Disk" : disk.volumeName,
                     formatBytes(disk.usedSpace),
                     formatBytes(disk.totalSize)));
            auto* bar = new QProgressBar();
            bar->setRange(0, 100);
            bar->setValue((int)percent);
            QString color = percent < 80 ? "#a6e3a1" : (percent < 95 ? "#f9e2af" : "#f38ba8");
            bar->setStyleSheet(
                QString("QProgressBar::chunk { background-color: %1; border-radius: 3px; }").arg(color));
            rowLayout->addWidget(label, 1);
            rowLayout->addWidget(bar, 2);
            m_overviewDiskLayout->addWidget(row);
        }
    }
}

void MainWindow::updateCpu(const CpuInfo& cpu)
{
    m_cpuName->setText(cpu.name.isEmpty() ? "N/A" : cpu.name);
    m_cpuCores->setText(QString::number(cpu.cores));
    m_cpuThreads->setText(QString::number(cpu.logicalProcessors));
    m_cpuMaxClock->setText(cpu.maxClockSpeed.isEmpty() ? "N/A" : cpu.maxClockSpeed);
    m_cpuCurClock->setText(cpu.currentClockSpeed.isEmpty() ? "N/A" : cpu.currentClockSpeed);
    m_cpuLoad->setText(QString("%1%").arg(cpu.loadPercent, 0, 'f', 1));
    m_cpuLoadBar->setValue((int)cpu.loadPercent);

    if (cpu.temperature > 0)
        m_cpuTemp->setText(QString("%1 °C").arg(cpu.temperature, 0, 'f', 1));
    else
        m_cpuTemp->setText("N/A (requires admin)");

    int load = (int)cpu.loadPercent;
    QString chunkColor = load < 60 ? "#a6e3a1" : (load < 85 ? "#f9e2af" : "#f38ba8");
    m_cpuLoadBar->setStyleSheet(
        QString("QProgressBar::chunk { background-color: %1; border-radius: 3px; }").arg(chunkColor));
}

void MainWindow::updateGpu(const GpuInfo& gpu)
{
    m_gpuName->setText(gpu.name.isEmpty() ? "N/A" : gpu.name);
    m_gpuProcessor->setText(gpu.videoProcessor.isEmpty() ? "N/A" : gpu.videoProcessor);
    m_gpuMemory->setText(gpu.adapterRam > 0 ? formatBytes(gpu.adapterRam) : "N/A");
    m_gpuDriver->setText(gpu.driverVersion.isEmpty() ? "N/A" : gpu.driverVersion);
    m_gpuDriverDate->setText(gpu.driverDate.isEmpty() ? "N/A" : gpu.driverDate);
}

void MainWindow::updateMemory(const MemoryInfo& mem)
{
    m_memTotalPhys->setText(formatBytes(mem.totalPhys));
    m_memAvailPhys->setText(formatBytes(mem.availPhys));
    quint64 usedPhys = mem.totalPhys - mem.availPhys;
    m_memUsedPhys->setText(formatBytes(usedPhys));

    double memPercent = mem.totalPhys > 0
        ? (double)usedPhys / mem.totalPhys * 100.0 : 0.0;
    m_memUsagePercent->setText(QString("%1%").arg(memPercent, 0, 'f', 1));
    m_memBar->setValue((int)memPercent);

    QString chunkColor = memPercent < 70 ? "#a6e3a1" : (memPercent < 90 ? "#f9e2af" : "#f38ba8");
    m_memBar->setStyleSheet(
        QString("QProgressBar::chunk { background-color: %1; border-radius: 3px; }").arg(chunkColor));

    m_memTotalVirt->setText(formatBytes(mem.totalVirtual));
    m_memAvailVirt->setText(formatBytes(mem.availVirtual));
    m_memTotalPage->setText(formatBytes(mem.totalPageFile));
    m_memAvailPage->setText(formatBytes(mem.availPageFile));
}

void MainWindow::updateDisk(const QVector<DiskInfo>& disks)
{
    QTabWidget* tabs = findChild<QTabWidget*>();
    if (!tabs) return;
    QWidget* diskPage = tabs->widget(4);
    if (!diskPage) return;
    QScrollArea* scroll = qobject_cast<QScrollArea*>(diskPage);
    if (!scroll) return;
    QWidget* content = scroll->widget();
    if (!content) return;

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(content->layout());
    if (!layout) return;

    while (layout->count() > 0) {
        auto* item = layout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    m_diskDeviceLabels.clear();
    m_diskNameLabels.clear();
    m_diskFsLabels.clear();
    m_diskTotalLabels.clear();
    m_diskUsedLabels.clear();
    m_diskFreeLabels.clear();
    m_diskBars.clear();

    for (const auto& disk : disks) {
        auto* group = new QGroupBox(QString("Disk %1 - %2")
            .arg(disk.deviceID, disk.volumeName.isEmpty() ? "Local Disk" : disk.volumeName));
        auto* grid = new QGridLayout(group);

        double percent = disk.totalSize > 0
            ? (double)disk.usedSpace / disk.totalSize * 100.0 : 0.0;

        auto* devLabel = new QLabel(disk.deviceID);
        auto* nameLabel = new QLabel(disk.volumeName.isEmpty() ? "Local Disk" : disk.volumeName);
        auto* fsLabel = new QLabel(disk.fileSystem);
        auto* totalLabel = new QLabel(formatBytes(disk.totalSize));
        auto* usedLabel = new QLabel(formatBytes(disk.usedSpace));
        auto* freeLabel = new QLabel(formatBytes(disk.freeSpace));
        auto* bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setValue((int)percent);

        QString chunkColor = percent < 80 ? "#a6e3a1" : (percent < 95 ? "#f9e2af" : "#f38ba8");
        bar->setStyleSheet(
            QString("QProgressBar::chunk { background-color: %1; border-radius: 3px; }").arg(chunkColor));

        grid->addWidget(new QLabel("Device:"), 0, 0);
        grid->addWidget(devLabel, 0, 1);
        grid->addWidget(new QLabel("Name:"), 0, 2);
        grid->addWidget(nameLabel, 0, 3);
        grid->addWidget(new QLabel("File System:"), 1, 0);
        grid->addWidget(fsLabel, 1, 1);
        grid->addWidget(new QLabel("Total:"), 1, 2);
        grid->addWidget(totalLabel, 1, 3);
        grid->addWidget(new QLabel("Used:"), 2, 0);
        grid->addWidget(usedLabel, 2, 1);
        grid->addWidget(new QLabel("Free:"), 2, 2);
        grid->addWidget(freeLabel, 2, 3);
        grid->addWidget(bar, 3, 0, 1, 4);

        layout->addWidget(group);

        m_diskDeviceLabels.append(devLabel);
        m_diskNameLabels.append(nameLabel);
        m_diskFsLabels.append(fsLabel);
        m_diskTotalLabels.append(totalLabel);
        m_diskUsedLabels.append(usedLabel);
        m_diskFreeLabels.append(freeLabel);
        m_diskBars.append(bar);
    }
    layout->addStretch();
}

void MainWindow::updateNetwork(const QVector<NetworkInfo>& nets)
{
    QTabWidget* tabs = findChild<QTabWidget*>();
    if (!tabs) return;
    QWidget* netPage = tabs->widget(5);
    if (!netPage) return;
    QScrollArea* scroll = qobject_cast<QScrollArea*>(netPage);
    if (!scroll) return;
    QWidget* content = scroll->widget();
    if (!content) return;

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(content->layout());
    if (!layout) return;

    while (layout->count() > 0) {
        auto* item = layout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    m_netNameLabels.clear();
    m_netIpLabels.clear();
    m_netMacLabels.clear();
    m_netDownLabels.clear();
    m_netUpLabels.clear();

    if (nets.isEmpty()) {
        auto* label = new QLabel("No active network connections found.");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        layout->addStretch();
        return;
    }

    for (const auto& net : nets) {
        auto* group = new QGroupBox(net.name);
        auto* grid = new QGridLayout(group);

        auto* ipLabel = new QLabel(net.ipAddress.isEmpty() ? "N/A" : net.ipAddress);
        auto* macLabel = new QLabel(net.macAddress.isEmpty() ? "N/A" : net.macAddress);
        auto* downLabel = new QLabel(formatBytes(net.bytesReceived) + "/s");
        auto* upLabel = new QLabel(formatBytes(net.bytesSent) + "/s");

        grid->addWidget(new QLabel("IP Address:"), 0, 0);
        grid->addWidget(ipLabel, 0, 1);
        grid->addWidget(new QLabel("MAC Address:"), 0, 2);
        grid->addWidget(macLabel, 0, 3);
        grid->addWidget(new QLabel("Download:"), 1, 0);
        grid->addWidget(downLabel, 1, 1);
        grid->addWidget(new QLabel("Upload:"), 1, 2);
        grid->addWidget(upLabel, 1, 3);

        layout->addWidget(group);

        m_netNameLabels.append(new QLabel(net.name));
        m_netIpLabels.append(ipLabel);
        m_netMacLabels.append(macLabel);
        m_netDownLabels.append(downLabel);
        m_netUpLabels.append(upLabel);
    }
    layout->addStretch();
}

void MainWindow::updateSystem(const SystemInfo& sys)
{
    m_sysOsName->setText(sys.osName.isEmpty() ? "N/A" : sys.osName);
    m_sysOsVersion->setText(sys.osVersion.isEmpty() ? "N/A" : sys.osVersion);
    m_sysComputer->setText(sys.computerName.isEmpty() ? "N/A" : sys.computerName);
    m_sysUser->setText(sys.userName.isEmpty() ? "N/A" : sys.userName);
    m_sysModel->setText(sys.systemModel.isEmpty() ? "N/A" : sys.systemModel);
    m_sysManufacturer->setText(sys.systemManufacturer.isEmpty() ? "N/A" : sys.systemManufacturer);
}
