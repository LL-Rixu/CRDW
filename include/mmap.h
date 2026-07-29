#pragma once

#include <log.h>
#include <string>
#include <Windows.h>

struct MMAPConfig
{
    DWORD desiredAccess = GENERIC_READ;
    DWORD shareMode = FILE_SHARE_READ;
    DWORD creationDisposition = OPEN_EXISTING;
    DWORD flagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN;
    DWORD flProtect = PAGE_READONLY;
    DWORD mapAccess = FILE_MAP_READ;

    consteval MMAPConfig(DWORD a_desiredAccess, DWORD a_shareMode, DWORD a_creationDisposition, DWORD a_flagsAndAttributes, DWORD a_flProtect, DWORD a_mapAccess): 
    desiredAccess(a_desiredAccess), 
    shareMode(a_shareMode),
    creationDisposition(a_creationDisposition),
    flagsAndAttributes(a_flagsAndAttributes),
    flProtect(a_flProtect),
    mapAccess(a_mapAccess)
    {}

    consteval MMAPConfig() {}
};

template<typename T, MMAPConfig Config = MMAPConfig()>
class MMAP
{
protected:
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = nullptr;
    T* map = nullptr;
    size_t size = 0;

public:
    T* Map() const { return map; }
    size_t Size() const { return size; }
    size_t Count() const { return size / sizeof(T); }

    bool Open() const { return hFile != INVALID_HANDLE_VALUE && hMap != nullptr && map != nullptr && size != 0; }

    T* begin() const { return &map[0]; }
    T* end() const { return &map[size / sizeof(T)]; }

    MMAP(const char* path): MMAP(std::string(path)) {}
    MMAP(const std::string& path)
    {
        hFile = CreateFileA(
            path.c_str(),
            Config.desiredAccess, 
            Config.shareMode, 
            NULL, 
            Config.creationDisposition, 
            Config.flagsAndAttributes, 
            NULL
        );

        if (hFile == INVALID_HANDLE_VALUE) { return; }
 
        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hFile, &fileSize)) return;
        size = static_cast<size_t>(fileSize.QuadPart); 

        hMap = CreateFileMappingA(
            hFile, 
            NULL, 
            Config.flProtect, 
            0, 
            0, 
            NULL
        );

        if (!hMap) { return; }

        map = reinterpret_cast<T*>(MapViewOfFile(
            hMap,
            Config.mapAccess,
            0,
            0,
            0
        ));

        Log(spdlog::level::info, "opened mmap {}", path);
    }

    ~MMAP()
    {
        if (map) { UnmapViewOfFile(map); }
        if (hMap) { CloseHandle(hMap); }
        if (hFile != INVALID_HANDLE_VALUE) { CloseHandle(hFile); }

        Log(spdlog::level::info, "closed mmap");
    }
};