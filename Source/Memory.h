#pragma once

enum class ProcStatus {
    NotRunning,
    Running
};

using byte = unsigned char;

// https://github.com/erayarslan/WriteProcessMemory-Example
// http://stackoverflow.com/q/32798185
// http://stackoverflow.com/q/36018838
// http://stackoverflow.com/q/1387064
// https://github.com/fkloiber/witness-trainer/blob/master/source/foreign_process_memory.cpp
class Memory final : public std::enable_shared_from_this<Memory> {
public:
    Memory(const std::wstring& processName);
    ~Memory();
    void StartHeartbeat(HWND window, WPARAM wParam, std::chrono::milliseconds beat = std::chrono::milliseconds(100));

    Memory(const Memory& memory) = delete;
    Memory& operator=(const Memory& other) = delete;

    using ScanFunc = std::function<void(int offset, int index, const std::vector<byte>& data)>;
    void AddSigScan(const std::vector<byte>& scanBytes, const ScanFunc& scanFunc);
    int ExecuteSigScans(int blockSize = 0x1000);

    template<class T>
    std::vector<T> ReadData(const std::vector<__int64>& offsets, size_t numItems) {
        assert(numItems);
        std::vector<T> data;
        data.resize(numItems);
        if (!ReadProcessMemory(_handle, ComputeOffset(offsets), &data[0], sizeof(T) * numItems, nullptr)) {
            MEMORY_THROW("Failed to read data.", offsets, numItems);
        }
        return data;
    }

    template <class T>
    void WriteData(const std::vector<__int64>& offsets, const std::vector<T>& data) {
        assert(data.size());
        if (!WriteProcessMemory(_handle, ComputeOffset(offsets), &data[0], sizeof(T) * data.size(), nullptr)) {
            MEMORY_THROW("Failed to write data.", offsets, data.size());
        }
    }

private:
    void Heartbeat(HWND window, WPARAM wParam);
    bool Initialize();
    void* ComputeOffset(std::vector<__int64> offsets);

    bool _threadActive = false;
    std::thread _thread;
    std::wstring _processName;
    std::map<uintptr_t, uintptr_t> _computedAddresses;
    uintptr_t _baseAddress = 0;
    HANDLE _handle = nullptr;
    struct SigScan {
        ScanFunc scanFunc;
        bool found;
    };
    std::map<std::vector<byte>, SigScan> _sigScans;
};
