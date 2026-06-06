#include "HardwareMonitor.h"
#include <comdef.h>
#include <cstring>

HardwareMonitor::HardwareMonitor() {}

HardwareMonitor::~HardwareMonitor()
{
    if (m_services) m_services->Release();
    if (m_locator) m_locator->Release();
    CoUninitialize();
}

bool HardwareMonitor::initialize()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        return false;

    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, reinterpret_cast<void**>(&m_locator));
    if (FAILED(hr))
        return false;

    hr = m_locator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr,
        0, nullptr, nullptr, &m_services);
    if (FAILED(hr))
        return false;

    hr = CoSetProxyBlanket(
        m_services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr, EOAC_NONE);
    if (FAILED(hr))
        return false;

    return true;
}

IEnumWbemClassObject* HardwareMonitor::executeQuery(const wchar_t* query)
{
    IEnumWbemClassObject* enumerator = nullptr;
    HRESULT hr = m_services->ExecQuery(
        _bstr_t(L"WQL"), _bstr_t(query),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr, &enumerator);
    if (FAILED(hr))
        return nullptr;
    return enumerator;
}

QString HardwareMonitor::getStringProperty(IWbemClassObject* obj, const wchar_t* name)
{
    VARIANT vt;
    VariantInit(&vt);
    HRESULT hr = obj->Get(name, 0, &vt, nullptr, nullptr);
    QString result;
    if (SUCCEEDED(hr) && vt.vt == VT_BSTR && vt.bstrVal)
        result = QString::fromWCharArray(vt.bstrVal);
    VariantClear(&vt);
    return result;
}

quint64 HardwareMonitor::getUint64Property(IWbemClassObject* obj, const wchar_t* name)
{
    VARIANT vt;
    VariantInit(&vt);
    HRESULT hr = obj->Get(name, 0, &vt, nullptr, nullptr);
    quint64 result = 0;
    if (SUCCEEDED(hr) && vt.vt == VT_BSTR) {
        result = QString::fromWCharArray(vt.bstrVal).toULongLong();
    } else if (SUCCEEDED(hr) && (vt.vt == VT_I8 || vt.vt == VT_UI8)) {
        result = vt.ullVal;
    }
    VariantClear(&vt);
    return result;
}

quint32 HardwareMonitor::getUint32Property(IWbemClassObject* obj, const wchar_t* name)
{
    VARIANT vt;
    VariantInit(&vt);
    HRESULT hr = obj->Get(name, 0, &vt, nullptr, nullptr);
    quint32 result = 0;
    if (SUCCEEDED(hr) && (vt.vt == VT_I4 || vt.vt == VT_UI4))
        result = vt.ulVal;
    VariantClear(&vt);
    return result;
}

double HardwareMonitor::getDoubleProperty(IWbemClassObject* obj, const wchar_t* name)
{
    VARIANT vt;
    VariantInit(&vt);
    HRESULT hr = obj->Get(name, 0, &vt, nullptr, nullptr);
    double result = 0.0;
    if (SUCCEEDED(hr) && vt.vt == VT_R8)
        result = vt.dblVal;
    else if (SUCCEEDED(hr) && vt.vt == VT_R4)
        result = (double)vt.fltVal;
    VariantClear(&vt);
    return result;
}

CpuInfo HardwareMonitor::getCpuInfo()
{
    CpuInfo info;

    auto* enumerator = executeQuery(L"SELECT * FROM Win32_Processor");
    if (!enumerator) return info;

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    if (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned > 0) {
        info.name = getStringProperty(obj, L"Name");
        info.cores = getUint32Property(obj, L"NumberOfCores");
        info.logicalProcessors = getUint32Property(obj, L"NumberOfLogicalProcessors");
        info.loadPercent = getDoubleProperty(obj, L"LoadPercentage");
        info.maxClockSpeed = QString::number(getUint32Property(obj, L"MaxClockSpeed")) + " MHz";
        info.currentClockSpeed = QString::number(getUint32Property(obj, L"CurrentClockSpeed")) + " MHz";
        obj->Release();
    }
    enumerator->Release();

    // Try to get CPU temperature (requires admin privileges)
    auto* tempEnum = executeQuery(
        L"SELECT * FROM MSAcpi_ThermalZoneTemperature");
    if (tempEnum) {
        obj = nullptr;
        returned = 0;
        if (tempEnum->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned > 0) {
            VARIANT vt;
            VariantInit(&vt);
            if (SUCCEEDED(obj->Get(L"CurrentTemperature", 0, &vt, nullptr, nullptr))) {
                // Temperature in tenths of Kelvin, convert to Celsius
                double kelvin = vt.dblVal / 10.0;
                info.temperature = kelvin - 273.15;
            }
            VariantClear(&vt);
            obj->Release();
        }
        tempEnum->Release();
    }

    return info;
}

GpuInfo HardwareMonitor::getGpuInfo()
{
    GpuInfo info;

    auto* enumerator = executeQuery(L"SELECT * FROM Win32_VideoController");
    if (!enumerator) return info;

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    if (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned > 0) {
        info.name = getStringProperty(obj, L"Name");
        info.adapterRam = getUint64Property(obj, L"AdapterRAM");
        info.driverVersion = getStringProperty(obj, L"DriverVersion");
        info.driverDate = getStringProperty(obj, L"DriverDate");
        info.videoProcessor = getStringProperty(obj, L"VideoProcessor");
        obj->Release();
    }
    enumerator->Release();

    return info;
}

MemoryInfo HardwareMonitor::getMemoryInfo()
{
    MemoryInfo info;

    auto* enumerator = executeQuery(L"SELECT * FROM Win32_OperatingSystem");
    if (!enumerator) return info;

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    if (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned > 0) {
        info.totalPhys = getUint64Property(obj, L"TotalVisibleMemorySize") * 1024;
        info.availPhys = getUint64Property(obj, L"FreePhysicalMemory") * 1024;
        info.totalVirtual = getUint64Property(obj, L"TotalVirtualMemorySize") * 1024;
        info.availVirtual = getUint64Property(obj, L"FreeVirtualMemory") * 1024;
        info.totalPageFile = info.totalVirtual - info.totalPhys;
        info.availPageFile = info.availVirtual - info.availPhys;
        obj->Release();
    }
    enumerator->Release();

    return info;
}

QVector<DiskInfo> HardwareMonitor::getDiskInfo()
{
    QVector<DiskInfo> disks;

    auto* enumerator = executeQuery(
        L"SELECT * FROM Win32_LogicalDisk WHERE DriveType=3");
    if (!enumerator) return disks;

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    while (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned > 0) {
        DiskInfo disk;
        disk.deviceID = getStringProperty(obj, L"DeviceID");
        disk.volumeName = getStringProperty(obj, L"VolumeName");
        disk.fileSystem = getStringProperty(obj, L"FileSystem");
        disk.totalSize = getUint64Property(obj, L"Size");
        disk.freeSpace = getUint64Property(obj, L"FreeSpace");
        disk.usedSpace = disk.totalSize - disk.freeSpace;
        disks.append(disk);
        obj->Release();
    }
    enumerator->Release();

    return disks;
}

QVector<NetworkInfo> HardwareMonitor::getNetworkInfo()
{
    QVector<NetworkInfo> networks;

    auto* enumerator = executeQuery(
        L"SELECT * FROM Win32_NetworkAdapter WHERE NetConnectionStatus=2");
    if (!enumerator) return networks;

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    while (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned > 0) {
        NetworkInfo net;
        net.name = getStringProperty(obj, L"Name");
        net.macAddress = getStringProperty(obj, L"MACAddress");
        net.isConnected = true;
        net.bytesReceived = getUint64Property(obj, L"BytesReceivedPersec");
        net.bytesSent = getUint64Property(obj, L"BytesSentPersec");
        networks.append(net);
        obj->Release();
    }
    enumerator->Release();

    // Get IP addresses
    auto* ipEnum = executeQuery(
        L"SELECT * FROM Win32_NetworkAdapterConfiguration WHERE IPEnabled=TRUE");
    if (ipEnum) {
        obj = nullptr;
        returned = 0;
        int idx = 0;
        while (ipEnum->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned > 0) {
            if (idx < networks.size()) {
                VARIANT vt;
                VariantInit(&vt);
                if (SUCCEEDED(obj->Get(L"IPAddress", 0, &vt, nullptr, nullptr))
                    && vt.vt == (VT_BSTR | VT_ARRAY) && vt.parray) {
                    long lb, ub;
                    SafeArrayGetLBound(vt.parray, 1, &lb);
                    SafeArrayGetUBound(vt.parray, 1, &ub);
                    BSTR* pData = nullptr;
                    SafeArrayAccessData(vt.parray, reinterpret_cast<void**>(&pData));
                    if (pData && lb <= ub)
                        networks[idx].ipAddress = QString::fromWCharArray(pData[0]);
                    SafeArrayUnaccessData(vt.parray);
                }
                VariantClear(&vt);
                idx++;
            }
            obj->Release();
        }
        ipEnum->Release();
    }

    return networks;
}

SystemInfo HardwareMonitor::getSystemInfo()
{
    SystemInfo info;

    auto* enumerator = executeQuery(L"SELECT * FROM Win32_OperatingSystem");
    if (!enumerator) return info;

    IWbemClassObject* obj = nullptr;
    ULONG returned = 0;
    if (enumerator->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned > 0) {
        info.osName = getStringProperty(obj, L"Caption");
        info.osVersion = getStringProperty(obj, L"Version");
        info.computerName = getStringProperty(obj, L"CSName");
        info.userName = getStringProperty(obj, L"RegisteredUser");
        obj->Release();
    }
    enumerator->Release();

    auto* csEnum = executeQuery(L"SELECT * FROM Win32_ComputerSystem");
    if (csEnum) {
        obj = nullptr;
        returned = 0;
        if (csEnum->Next(WBEM_INFINITE, 1, &obj, &returned) == S_OK && returned > 0) {
            info.systemModel = getStringProperty(obj, L"Model");
            info.systemManufacturer = getStringProperty(obj, L"Manufacturer");
            obj->Release();
        }
        csEnum->Release();
    }

    return info;
}
